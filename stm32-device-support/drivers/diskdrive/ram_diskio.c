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


#include "ram_diskio.h"

#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include "cutensils.h"



static void ramdisk_initialize(disk_interface_t* disk);
static DRESULT ramdisk_read(disk_interface_t* disk, BYTE *buff, DWORD sector, UINT count);
static DRESULT ramdisk_write(disk_interface_t* disk, const BYTE *buff, DWORD sector, UINT count);
static DRESULT ramdisk_ioctl(disk_interface_t* disk, BYTE ctrl, void *buff);
static void ramdisk_status(disk_interface_t* disk);

FRESULT ramdisk_mount(disk_interface_t* disk, int drive, ramdisk_t* ramdisk, void* memory, unsigned long sizebytes)
{
	FRESULT res;
	disk->disk_initialize = ramdisk_initialize;
	disk->disk_read = ramdisk_read;
	disk->disk_write = ramdisk_write;
	disk->disk_ioctl = ramdisk_ioctl;
	disk->disk_status = ramdisk_status;
	disk->ctx = ramdisk;
	ramdisk->memory = memory;
	ramdisk->sizebytes = sizebytes;

	diskdrive_add_drive(disk, drive);

	sprintf(disk->mapping.drivemapping, "%d:", drive);
	disk->mapping.drivename[0] = '\0';
	disk->mapping.mountpoint = "/";

	res = f_mount(&disk->mapping.fs, disk->mapping.drivemapping, 1);

	if(res == FR_NO_FILESYSTEM)
		res = f_mkfs(disk->mapping.drivemapping, 1, RAMDISK_CLUSTER_SIZE);

	f_setlabel(RAMDISK_VOLUME_LABLE);

	return res;
}

void ramdisk_initialize(disk_interface_t* disk)
{
	ramdisk_t* ramdisk = (ramdisk_t*)disk->ctx;
	disk->status = 0;

    disk->info.block_size = RAMDISK_SS;
    disk->info.erase_block_size = 1;
    disk->info.type = 255;
    disk->info.capacity = ramdisk->sizebytes / RAMDISK_SS;
}

DRESULT ramdisk_read(disk_interface_t* disk, BYTE *buff, DWORD sector, UINT count)
{
	ramdisk_t* ramdisk = (ramdisk_t*)disk->ctx;
	BYTE *sptr = ramdisk->memory + (sector * RAMDISK_SS);

	while(count > 0 && (size_t)(sptr - ramdisk->memory) < ramdisk->sizebytes)
	{
		memcpy(buff, sptr, RAMDISK_SS);
		sector++;
		buff += RAMDISK_SS;
		sptr += RAMDISK_SS;
		count--;
	}
	return count == 0 ? RES_OK : RES_ERROR;
}

DRESULT ramdisk_write(disk_interface_t* disk, const BYTE *buff, DWORD sector, UINT count)
{
	ramdisk_t* ramdisk = (ramdisk_t*)disk->ctx;
	BYTE *sptr = ramdisk->memory + (sector * RAMDISK_SS);

	while(count > 0 && (size_t)(sptr - ramdisk->memory) < ramdisk->sizebytes)
	{
		memcpy(sptr, buff, RAMDISK_SS);
		sector++;
		buff += RAMDISK_SS;
		sptr += RAMDISK_SS;
		count--;
	}

	return count == 0 ? RES_OK : RES_ERROR;
}

DRESULT ramdisk_ioctl(disk_interface_t* disk, BYTE ctrl, void *buff)
{
	DRESULT res = RES_OK;

	if(disk->status & (STA_NODISK | STA_NOINIT))
		return RES_NOTRDY;

	switch (ctrl)
	{
	 	case CTRL_SYNC :

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
		default:
			res = RES_PARERR;
		break;
	}

	return res;
}

void ramdisk_status(disk_interface_t* disk)
{
	(void)disk;
}
