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
 * This file is part of the Appleseed project, <https://github.com/drmetal/app-l-seed>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */


#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include "diskdrive.h"
#include "cutensils.h"


DSTATUS disk_initialize(BYTE drive)      /* Physical drive number (0) */
{
	disk_interface_t* disk = diskdrive_get_disk(drive);

	if(disk)
	{
		disk->disk_initialize(disk);
		return disk->status;
	}

	return STA_NOINIT;
}

DRESULT disk_read(BYTE drive, BYTE *buff, DWORD sector, UINT count)
{
	disk_interface_t* disk = diskdrive_get_disk(drive);

	if(disk)
		return disk->disk_read(disk, buff, sector, count);

	return RES_NOTRDY;
}

DRESULT disk_write (BYTE drive, const BYTE *buff, DWORD sector, UINT count)
{
	disk_interface_t* disk = diskdrive_get_disk(drive);

	if(disk)
		return disk->disk_write(disk, buff, sector, count);

	return RES_NOTRDY;
}

DRESULT disk_ioctl(BYTE drive, BYTE ctrl, void *buff)
{
	disk_interface_t* disk = diskdrive_get_disk(drive);

	if(disk)
		return disk->disk_ioctl(disk, ctrl, buff);

	return RES_NOTRDY;
}

/**
 * @brief   returns fat fs compliant time stamp from scheduler time base.
 *
 * @retval  returns an integer of the following format:
 *  bits  0:4       5 bits  Second/2        (0..29)
 *  bits  5:10      6 bits  Minute          (0..59)
 *  bits  11:15     5 bits  Hour            (0..23)
 *  bits  16:20     5 bits  Day in month    (1..31)
 *  bits  21:24     4 bits  Month           (1..12)
 *  bits  25:31     7 bits  Year from 1980  (0..127)
 */
DWORD get_fattime(void)
{
    DWORD ftime = 0;

    time_t t;
    time(&t);
    struct tm* lt = localtime(&t);
    ftime = ((lt->tm_year - 80) << 25) |
    ((lt->tm_mon + 1) << 21) |
    ((lt->tm_mday) << 16) |
    ((lt->tm_hour) << 11) |
    ((lt->tm_min) << 5) |
    (lt->tm_sec/2);

    return ftime;
}

/**
 * if disk is present, clears STA_NODISK flag.
 * else sets STA_NODISK flag.
 * if disk is not write protected, clears SD_WRITE_PROTECTED flag.
 * else sets SD_WRITE_PROTECTED flag.
 */
DSTATUS disk_status(BYTE drive)
{
	disk_interface_t* disk = diskdrive_get_disk(drive);

	if(disk)
	{
		disk->disk_status(disk);
		return disk->status;
	}

	return STA_NOINIT;
}
