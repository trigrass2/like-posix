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
 * This file is part of the Appleseed project, <https://github.com/drmetal/appleseed>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */
#include <stdlib.h>
#include "sdcard_diskio.h"
#include "sdcard.h"


static void sdcard_initialize(disk_interface_t* disk);
static DRESULT sdcard_read(disk_interface_t* disk, BYTE *buff, DWORD sector, UINT count);
static DRESULT sdcard_write(disk_interface_t* disk, const BYTE *buff, DWORD sector, UINT count);
static DRESULT sdcard_ioctl(disk_interface_t* disk, BYTE ctrl, void *buff);
static void sdcard_status(disk_interface_t* disk);

FRESULT sdcard_mount(disk_interface_t* disk, char drive)
{
	FRESULT res;
	disk->disk_initialize = sdcard_initialize;
	disk->disk_read = sdcard_read;
	disk->disk_write = sdcard_write;
	disk->disk_ioctl = sdcard_ioctl;
	disk->disk_status = sdcard_status;

	sprintf(disk->volume.lvn, "%d:", drive);
	disk->volume.label[0] = '\0';
	disk->volume.mountpoint = "/";

	diskdrive_add_drive(disk);

	res = f_mount(&disk->volume.fs, disk->volume.lvn, 1);

	if(res == FR_NO_FILESYSTEM)
	{
//	    // r11
//		// res = f_mkfs(disk->volume.lvn, 0, 0);

	    // r12a
	    char* workarea = malloc(512);
	    if(workarea)
	    {
	        res = f_mkfs(disk->volume.lvn, FM_ANY, 0, workarea, 512);
	        free(workarea);
	    }
	}

	return res;
}

void sdcard_initialize(disk_interface_t* disk)
{
	HAL_SD_CardInfoTypedef cardinfo;

    disk->status = STA_NOINIT | STA_PROTECT | STA_NODISK;

    if((sd_detect() == SD_PRESENT))
        disk->status &= ~STA_NODISK;

    if(sd_write_protected() != SD_WRITE_PROTECTED)
        disk->status &= ~STA_PROTECT;

    if(disk->status == STA_NOINIT)
    {
        if(sd_init(&cardinfo) == SD_OK)
        {
            disk->status &= ~STA_NOINIT;

            disk->info.block_size = cardinfo.CardBlockSize;
            disk->info.erase_block_size = cardinfo.SD_csd.EraseGrSize;
            disk->info.type = cardinfo.CardType;
            disk->info.capacity = cardinfo.CardCapacity;
        }
    }
}

DRESULT sdcard_read(disk_interface_t* disk, BYTE *buff, DWORD sector, UINT count)
{
    if(disk->status & (STA_NODISK | STA_NOINIT))
        return RES_NOTRDY;

    if(sd_read((uint8_t*)buff, sector, count) != SD_OK)
        return RES_ERROR;

    return RES_OK;
}

DRESULT sdcard_write (disk_interface_t* disk, const BYTE *buff, DWORD sector, UINT count)
{
    if(disk->status & (STA_NODISK | STA_NOINIT))
        return RES_NOTRDY;

    if(disk->status & STA_PROTECT)
        return RES_WRPRT;

    if(sd_write((uint8_t*)buff, sector, count) != SD_OK)
        return RES_ERROR;

    return RES_OK;
}


DRESULT sdcard_ioctl(disk_interface_t* disk, BYTE ctrl, void *buff)
{
   DRESULT res = RES_OK;

    if(disk->status & (STA_NODISK | STA_NOINIT))
        return RES_NOTRDY;

    switch (ctrl)
    {
      case CTRL_SYNC :        // error if transfer not complete
          if(sd_get_transfer_state() == SD_TRANSFER_BUSY)
              res = RES_ERROR;
      break;
      case GET_SECTOR_COUNT : // Get number of sectors on the disk
          *(DWORD*)buff = disk->info.capacity;
      break;
      case GET_SECTOR_SIZE :  // Get R/W sector size
          *(DWORD*)buff = disk->info.block_size;
      break;
      case GET_BLOCK_SIZE :   // Get erase block size in unit of sector
          *(DWORD*)buff = disk->info.erase_block_size;
      break;
      case MMC_GET_TYPE :     // Get card type
          *(BYTE*)buff = disk->info.type;
      break;
      case MMC_GET_CSD:

      break;
      case MMC_GET_CID:

      break;
      case MMC_GET_OCR:

      break;
      case MMC_GET_SDSTAT:

      break;
//      case CTRL_POWER :
//        if(*(BYTE*)buff == 0)
//            SD_PowerOFF();
//        if(*(BYTE*)buff == 1)
//            SD_PowerON();
//      break;
//      case CTRL_ERASE_SECTOR:
//          sd_erase(*(((DWORD*)buff)), *(((DWORD*)buff)+1));
//      break;
      default:
         res = RES_PARERR;
      break;
    }

    return res;
}

/**
 * if disk is present, clears STA_NODISK flag.
 * else sets STA_NODISK flag.
 * if disk is not write protected, clears SD_WRITE_PROTECTED flag.
 * else sets SD_WRITE_PROTECTED flag.
 */
void sdcard_status(disk_interface_t* disk)
{
    // update status, based on card inserted state

    if((sd_detect() == SD_PRESENT))
       disk->status &= ~STA_NODISK;         // indicate disk present
   else
       disk->status |= STA_NODISK;         // indicate no-disk

   if(sd_write_protected() == SD_WRITE_PROTECTED)
       disk->status |= STA_PROTECT;     // indicate write protected
   else
       disk->status &= ~STA_PROTECT;     // indicate not write protected
}
