/*
 * Copyright (c) 2015 Michael Stuart.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the like-posix project, <https://github.com/drmetal/like-posix>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

#include <string.h>
#include <stdbool.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include "onewire.h"


typedef struct {
    unsigned char ROM_NO[8];
    int last_discrepancy;
    int last_family_discrepancy;
    unsigned char last_device_flag;
    unsigned char crc8;
} onewire_search_t;

static int onewire_verify(int fd, onewire_search_t* search);
static int _onewire_search(int fd, onewire_search_t* search);
static unsigned char onewire_docrc8(onewire_search_t* search, unsigned char value);
static int onewire_first(int fd, onewire_search_t* search);
static int onewire_next(int fd, onewire_search_t* search);
static void onewire_target_setup(onewire_search_t* search, unsigned char family_code);
static void onewire_family_skip_setup(onewire_search_t* search);


/**
 * initializes a USART peripheral for use as a 1 wire master.
 *
 * @param usart - the usart device to use.
 * @param filename - the filename to assign to the usart device, typically "/dev/onewire0".
 */
void onewire_init(USART_TypeDef* usart, char* filename)
{
    usart_init(usart, filename, false, USART_ONEWIRE);
}

/**
 * sends the 1 wire bus reset, reads the response.
 *
 * @param fd is the 1 wire device file descriptor, returned by onewire_init().
 * @retval returns 0 on failure, non 0 on success.
 */
unsigned char onewire_reset(int fd)
{
    char c = 0xf0;
    struct termios t;

    tcgetattr(fd, &t);
    cfsetispeed(&t, 9600);
    tcsetattr(fd, TCSADRAIN, &t);

    write(fd, &c, 1);
    read(fd, &c, 1);

    printf("reset: %x\n", c);

    c = c > 0x10 && c < 0x90;

    tcgetattr(fd, &t);
    cfsetispeed(&t, 115200);
    tcsetattr(fd, TCSADRAIN, &t);

    return c;
}

/**
 * sends the 1 byte to the 1 wire bus, reads the response.
 *
 * to do a single byte write, specify the byte in the byte parameter.
 * to do a single byte read, specify 0xff in the byte parameter.
 *
 * @param fd is the 1 wire device file descriptor, returned by onewire_init().
 * @param byte is the byte to send.
 * @retval returns the received byte.
 *          returned byte is only valid if input byte was specified as 0xff.
 */
unsigned char onewire_xfer_byte(int fd, unsigned char byte)
{
    unsigned char c = 0;
    int i;
    char bits[8];
    for(i = 0; i < 8; i++)
        bits[i] = byte & (1<<i) ? 0xff : 0;

    write(fd, bits, 8);
    read(fd, bits, 8);

    for(i = 0; i < 8; i++)
        c |= bits[i] == 0xff ? (1<<i) : 0;

    return c;
}

/**
 * sends the 1 bit to the 1 wire bus, reads the response.
 *
 * to do a single bit write, specify the bit value in the bit parameter.
 * to do a single bit read, specify 0xff in the bit parameter.
 *
 * @param fd is the 1 wire device file descriptor, returned by onewire_init().
 * @param bit is the bit to send, set to 0 for logic low bit, or non 0 for logic high bit.
 * @retval returns the received bit as 0 for logic low bit, 1 for logic 1 bit.
 *          returned bit is only valid if input bit was specified as 0xff.
 */
unsigned char onewire_xfer_bit(int fd, unsigned char bit)
{
    bit = !bit ? 0 : 0xff;

    write(fd, &bit, 1);
    read(fd, &bit, 1);

    return bit == 0xff ? 1 : 0;
}

/**
 * issues the onewire rom command, either skipping the device serial number or matching it.
 *
 * @param fd is the uart file descriptor to use.
 * @param devcode specifies match the specified devcode, or skip device serial match when set to 0.
 */
void onewire_address_command(int fd, uint64_t devcode)
{
    if(devcode)
    {
        int i;
        unsigned char* byte = (unsigned char*)&devcode;
        onewire_write_byte(fd, ONEWIRE_MATCH_ROM);
        for(i = 0; i < 8; i++, byte++)
            onewire_write_byte(fd, *byte);
    }
    else
        onewire_write_byte(fd, ONEWIRE_SKIP_ROM);
}

/**
 * search for all attached one wire device serial codes.
 * stores the codes in a 64b wide array.
 *
 * @param fd is the uart file descriptor to use.
 * @param buffer is an array to store discovered device codes in.
 * @param length is the maximum number of devices to find.
 *        if length > 1 then the device search algorithm is used.
 *        if length = 1 then no search is performed, the first device code is simply read.
 */
void onewire_search(int fd, uint64_t* buffer, int length)
{
    int i;

    // find ALL devices
    printf("\nFIND ALL\n");

    if(length == 1)
    {
        if(onewire_reset(fd))
        {
            unsigned char* byte = (unsigned char*)buffer;
            onewire_write_byte(fd, ONEWIRE_READ_ROM);
            for(i = 0; i < 8; i++, byte++)
                *byte = onewire_read_byte(fd);
        }
    }
    else if(length > 1)
    {
        onewire_search_t search;
        int cnt = 0;
        int rslt = onewire_first(fd, &search);
        while(length && rslt)
        {
           // print device found
           for (i = 7; i >= 0; i--)
              printf("%02X", search.ROM_NO[i]);
           printf("  %d\n",++cnt);

           memcpy(buffer, search.ROM_NO, 8);
           buffer++;
           rslt = onewire_next(fd, &search);
           length--;
        }
    }
}


//--------------------------------------------------------------------------
// Find the 'first' devices on the 1-Wire bus
// Return true  : device found, ROM number in ROM_NO buffer
//        false : no device present
//
int onewire_first(int fd, onewire_search_t* search)
{
   // reset the search state
   search->last_discrepancy = 0;
   search->last_device_flag = false;
   search->last_family_discrepancy = 0;

   return _onewire_search(fd, search);
}

//--------------------------------------------------------------------------
// Find the 'next' devices on the 1-Wire bus
// Return true  : device found, ROM number in ROM_NO buffer
//        false : device not found, end of search
//
int onewire_next(int fd, onewire_search_t* search)
{
   // leave the search state alone
   return _onewire_search(fd, search);
}

//--------------------------------------------------------------------------
// Perform the 1-Wire Search Algorithm on the 1-Wire bus using the existing
// search state.
// Return true  : device found, ROM number in ROM_NO buffer
//        false : device not found, end of search
//
int _onewire_search(int fd, onewire_search_t* search)
{
   int id_bit_number;
   int last_zero, rom_byte_number, search_result;
   int id_bit, cmp_id_bit;
   unsigned char rom_byte_mask, search_direction;

   // initialize for search
   id_bit_number = 1;
   last_zero = 0;
   rom_byte_number = 0;
   rom_byte_mask = 1;
   search_result = 0;
   search->crc8 = 0;

   // if the last call was not the last one
   if (!search->last_device_flag)
   {
      // 1-Wire reset
      if (!onewire_reset(fd))
      {
         // reset the search
         search->last_discrepancy = 0;
         search->last_device_flag = false;
         search->last_family_discrepancy = 0;
         return false;
      }

      // issue the search command
      onewire_write_byte(fd, ONEWIRE_SEARCH_ROM);
      printf("search\n");

      // loop to do the search
      do
      {
         // read a bit and its complement
         id_bit = onewire_read_bit(fd);
         cmp_id_bit = onewire_read_bit(fd);

         // check for no devices on 1-wire
         if ((id_bit == 1) && (cmp_id_bit == 1))
         {
            break;
            printf("none\n");
         }
         else
         {
            // all devices coupled have 0 or 1
            if (id_bit != cmp_id_bit)
               search_direction = id_bit;  // bit write value for search
            else
            {
               // if this discrepancy if before the Last Discrepancy
               // on a previous next then pick the same as last time
               if (id_bit_number < search->last_discrepancy)
                  search_direction = ((search->ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
               else
                  // if equal to last pick 1, if not then pick 0
                  search_direction = (id_bit_number == search->last_discrepancy);

               // if 0 was picked then record its position in LastZero
               if (search_direction == 0)
               {
                  last_zero = id_bit_number;

                  // check for Last discrepancy in family
                  if (last_zero < 9)
                     search->last_family_discrepancy = last_zero;
               }
            }

            // set or clear the bit in the ROM byte rom_byte_number
            // with mask rom_byte_mask
            if (search_direction == 1)
              search->ROM_NO[rom_byte_number] |= rom_byte_mask;
            else
              search->ROM_NO[rom_byte_number] &= ~rom_byte_mask;

            printf("rom byte %x\n", search->ROM_NO[rom_byte_number]);

            // serial number search direction write bit
            onewire_write_bit(fd, search_direction);

            // increment the byte counter id_bit_number
            // and shift the mask rom_byte_mask
            id_bit_number++;
            rom_byte_mask <<= 1;

            // if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
            if (rom_byte_mask == 0)
            {
                onewire_docrc8(search, search->ROM_NO[rom_byte_number]);  // accumulate the CRC
                rom_byte_number++;
                rom_byte_mask = 1;
            }
         }
      }
      while(rom_byte_number < 8);  // loop until through all ROM bytes 0-7

      // if the search was successful then
      if (!((id_bit_number < 65) || (search->crc8 != 0)))
      {
         // search successful so set LastDiscrepancy,LastDeviceFlag,search_result
         search->last_discrepancy = last_zero;

         // check for last device
         if (search->last_discrepancy == 0)
            search->last_device_flag = true;

         search_result = true;
      }
   }

   // if no device found then reset counters so next 'search' will be like a first
   if (!search_result || !search->ROM_NO[0])
   {
      search->last_discrepancy = 0;
      search->last_device_flag = false;
      search->last_family_discrepancy = 0;
      search_result = false;
   }

   return search_result;
}

//--------------------------------------------------------------------------
// Verify the device with the ROM number in ROM_NO buffer is present.
// Return true  : device verified present
//        false : device not present
//
int onewire_verify(int fd, onewire_search_t* search)
{
   unsigned char rom_backup[8];
   int i;
   int rslt;
   int ld_backup;
   bool ldf_backup;
   int lfd_backup;


   // keep a backup copy of the current state
   for (i = 0; i < 8; i++)
      rom_backup[i] = search->ROM_NO[i];
   ld_backup = search->last_discrepancy;
   ldf_backup = search->last_device_flag;
   lfd_backup = search->last_family_discrepancy;

   // set search to find the same device
   search->last_discrepancy = 64;
   search->last_device_flag = false;

   if (_onewire_search(fd, search))
   {
      // check if same device found
      rslt = true;
      for (i = 0; i < 8; i++)
      {
         if (rom_backup[i] != search->ROM_NO[i])
         {
            rslt = false;
            break;
         }
      }
   }
   else
     rslt = false;

   // restore the search state
   for (i = 0; i < 8; i++)
      search->ROM_NO[i] = rom_backup[i];
   search->last_discrepancy = ld_backup;
   search->last_device_flag = ldf_backup;
   search->last_family_discrepancy = lfd_backup;

   // return the result of the verify
   return rslt;
}

//--------------------------------------------------------------------------
// Setup the search to find the device type 'family_code' on the next call
// to onewire_next() if it is present.
//
void onewire_target_setup(onewire_search_t* search, unsigned char family_code)
{
   int i;

   // set the search state to find SearchFamily type devices
   search->ROM_NO[0] = family_code;
   for (i = 1; i < 8; i++)
      search->ROM_NO[i] = 0;
   search->last_discrepancy = 64;
   search->last_family_discrepancy = 0;
   search->last_device_flag = false;
}

//--------------------------------------------------------------------------
// Setup the search to skip the current device type on the next call
// to onewire_next().
//
void onewire_family_skip_setup(onewire_search_t* search)
{
   // set the Last discrepancy to last family discrepancy
   search->last_discrepancy = search->last_family_discrepancy;
   search->last_family_discrepancy = 0;

   // check for end of list
   if (search->last_discrepancy == 0)
      search->last_device_flag = true;
}



static const unsigned char dscrc_table[] = {
        0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
      157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
       35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
      190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
       70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
      219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
      101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
      248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
      140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
       17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
      175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
       50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
      202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
       87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
      233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
      116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53};

//--------------------------------------------------------------------------
// Calculate the CRC8 of the byte value provided with the current
// global 'crc8' value.
// Returns current global crc8 value
//
unsigned char onewire_docrc8(onewire_search_t* search, unsigned char value)
{
   search->crc8 = dscrc_table[search->crc8 ^ value];
   return search->crc8;
}

