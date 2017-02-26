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

#ifndef DS1820_H_
#define DS1820_H_

// FUNCTION COMMANDS
#define DS1820_CONVERT_TEMP        0x44
#define DS1820_COPY_SCRATCHPAD     0x48
#define DS1820_WRITE_SCRATCHPAD    0x4E
#define DS1820_READ_SCRATCHPAD     0xBE
#define DS1820_RECALL_EEPROM       0xB8
#define DS1820_READ_PS             0xB4

int ds1820_open(const char* filename);
void ds1820_close(int fd);
void ds1820_convert(int fd, uint64_t devcode);
float ds1820_read_temperature(int fd, uint64_t devcode);

/////////////////////////////////////////////////////////////////////////////////
////Brad Goold 2012
//// Modified by Mike Stuart, Feb 2014
/////////////////////////////////////////////////////////////////////////////////
//
//#include <stdint.h>
//#include "board_config.h"
//#include "ds1820_config.h.in"
//
//#ifndef DS1820_H
//#define DS1820_H
//
//#define DS1820_MAX_VALUE              125
//#define DS1820_MIN_VALUE              -55



//
//// error codes
//#define BUS_ERROR           0xFE
//#define PRESENCE_ERROR      0xFD
//#define NO_ERROR            0x00
//
//void ds1820_init(void);
//uint64_t ds1820_search();
//void ds1820_convert(void);
//float ds1820_read_device(uint64_t devcode);
//
//#endif

#endif //DS1820_H_
