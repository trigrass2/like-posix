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

#include "diskdrive.h"
#include "ff.h"

static disk_interface_t* disks[DISKDRIVE_NUM_DRIVES];


void diskdrive_add_drive(disk_interface_t* disk, int drive)
{
	if(drive < DISKDRIVE_NUM_DRIVES && !disks[drive])
	{
		disks[drive] = disk;
	}
}

disk_interface_t* diskdrive_get_disk(int drive)
{
	if(drive < DISKDRIVE_NUM_DRIVES)
		return disks[drive];
	return NULL;
}

void diskdrive_chdrive(int drive)
{
	disk_interface_t* disk = diskdrive_get_disk(drive);
	f_chdrive(disk->mapping.drivemapping);
}

uint32_t diskdrive_card_capacity(int drive)
{
    uint32_t capacity = 0;
    uint32_t sectorsize = 0;
    uint32_t sectorcount = 0;
    disk_ioctl(drive, GET_SECTOR_SIZE, &sectorsize);
    disk_ioctl(drive, GET_SECTOR_COUNT, &sectorcount);
    capacity = sectorsize * (sectorcount / 1024);
    return capacity;
}

char* diskdrive_mountpoint(int drive)
{
	disk_interface_t* disk = diskdrive_get_disk(drive);
	return disk->mapping.mountpoint;
}

uint32_t diskdrive_sector_count(int drive)
{
    uint32_t sectorcount = 0;
    disk_ioctl(drive, GET_SECTOR_COUNT, &sectorcount);
    return sectorcount;
}

uint32_t diskdrive_sector_size(int drive)
{
    uint32_t sectorsize = 0;
    disk_ioctl(drive, GET_SECTOR_SIZE, &sectorsize);
    return sectorsize;
}

uint8_t diskdrive_card_type(int drive)
{
    uint8_t cardtype = 255;
    disk_ioctl(drive, MMC_GET_TYPE, &cardtype);
    return cardtype;
}

char* diskdrive_volume_label(int drive)
{
	disk_interface_t* disk = diskdrive_get_disk(drive);
	DWORD vsn;
	f_getlabel(disk->mapping.drivemapping, disk->mapping.drivename, &vsn);
	return disk->mapping.drivename;
}

char* diskdrive_logical_drive_number(int drive)
{
	disk_interface_t* disk = diskdrive_get_disk(drive);
	return disk->mapping.drivemapping;
}

uint32_t diskdrive_clusters_free(int drive)
{
	disk_interface_t* disk = diskdrive_get_disk(drive);
	FATFS *fs;
	DWORD nclst;
    f_getfree(disk->mapping.drivemapping, &nclst, &fs);
    return nclst;
}

uint32_t diskdrive_cluster_size(int drive)
{
	disk_interface_t* disk = diskdrive_get_disk(drive);
    return disk->mapping.fs.csize;
}
