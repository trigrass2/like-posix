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

#include "diskdrive.h"
#include "ff.h"

static disk_interface_t* disks[DISKDRIVE_NUM_DRIVES];
static char current_drive; // current drive is set to 0. this has to be the case, to match the default in ff.c

static char get_drive_number(char* lvn)
{
	return lvn[0] - '0';
}

void diskdrive_add_drive(disk_interface_t* disk)
{
	char drive = get_drive_number(disk->volume.lvn);

	if(drive < DISKDRIVE_NUM_DRIVES && !disks[drive])
	{
		disks[drive] = disk;
	}
}

disk_interface_t* diskdrive_get_disk(char drive)
{
	if(drive < DISKDRIVE_NUM_DRIVES)
		return disks[drive];
	return NULL;
}

int diskdrive_chdrive(char* lvn)
{
	char drive = get_drive_number(lvn);
	disk_interface_t* disk = diskdrive_get_disk(drive);

	if(disk)
	{
		if(f_chdrive(disk->volume.lvn) == FR_OK)
		{
			current_drive = drive;
			return 0;
		}
	}
	return -1;
}

uint32_t diskdrive_card_capacity()
{
    uint32_t capacity = 0;
    uint32_t sectorsize = 0;
    uint32_t sectorcount = 0;
    disk_ioctl(current_drive, GET_SECTOR_SIZE, &sectorsize);
    disk_ioctl(current_drive, GET_SECTOR_COUNT, &sectorcount);
    capacity = sectorsize * (sectorcount / 1024);
    return capacity;
}

char* diskdrive_mountpoint()
{
	disk_interface_t* disk = diskdrive_get_disk(current_drive);
	if(disk)
		return disk->volume.mountpoint;
	return NULL;
}

uint32_t diskdrive_sector_count()
{
	disk_interface_t* disk = diskdrive_get_disk(current_drive);
    FATFS *fs;
    DWORD fre_clust, fre_sect, tot_sect;
    f_getfree(disk->volume.lvn, &fre_clust, &fs);
    tot_sect = (fs->n_fatent - 2) * fs->csize;
    return (uint32_t)tot_sect;
}

uint32_t diskdrive_sector_size()
{
    uint32_t sectorsize = 0;
    disk_ioctl(current_drive, GET_SECTOR_SIZE, &sectorsize);
    return sectorsize;
}

uint8_t diskdrive_card_type()
{
    uint8_t cardtype = 255;
    disk_ioctl(current_drive, MMC_GET_TYPE, &cardtype);
    return cardtype;
}

char* diskdrive_volume_label()
{
	disk_interface_t* disk = diskdrive_get_disk(current_drive);
	if(disk)
	{
		DWORD vsn;
		f_getlabel(disk->volume.lvn, disk->volume.label, &vsn);
		return disk->volume.label;
	}
	return NULL;
}

char* diskdrive_logical_drive_number()
{
	disk_interface_t* disk = diskdrive_get_disk(current_drive);
	if(disk)
		return disk->volume.lvn;
	return NULL;
}

uint32_t diskdrive_clusters_free()
{
	disk_interface_t* disk = diskdrive_get_disk(current_drive);
	if(disk)
	{
		FATFS *fs;
		DWORD nclst;
		f_getfree(disk->volume.lvn, &nclst, &fs);
		return nclst;
	}
	return 0;
}

uint32_t diskdrive_cluster_size()
{
	disk_interface_t* disk = diskdrive_get_disk(current_drive);
	if(disk)
		return disk->volume.fs.csize;
	return 0;
}
