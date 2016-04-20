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

#ifndef FATFS_DISK_INTERFACE_H_
#define FATFS_DISK_INTERFACE_H_

#include "ff.h"
#include "diskio.h"

#define DISKDRIVE_NUM_DRIVES  _VOLUMES // set in ffconf.h


typedef struct _disk_interface_t disk_interface_t;

typedef struct {
    unsigned long capacity;
    uint8_t type;
    unsigned long erase_block_size;
    unsigned long block_size;
}disk_info_t;

typedef struct {
    FATFS fs;
    char lvn[3]; ///< logical drive number
    char* mountpoint;
    char label[13];
}disk_volume_t;

struct _disk_interface_t {
	void(*disk_initialize)(disk_interface_t*);
	void(*disk_status)(disk_interface_t*);
	DRESULT(*disk_read)(disk_interface_t*, BYTE*, DWORD, UINT);
	DRESULT(*disk_write)(disk_interface_t*, const BYTE*, DWORD, UINT);
	DRESULT(*disk_ioctl)(disk_interface_t*, BYTE, void*);
	DSTATUS status;
	disk_info_t info;
	disk_volume_t volume;
	void* ctx;
};

void diskdrive_add_drive(disk_interface_t* disk);
disk_interface_t* diskdrive_get_disk(unsigned char drive);
int diskdrive_chdrive(const char* lvn);

uint32_t diskdrive_card_capacity();
char* diskdrive_mountpoint();
uint32_t diskdrive_sector_count();
uint32_t diskdrive_sector_size();
uint8_t diskdrive_card_type();
char* diskdrive_volume_label();
char* diskdrive_logical_drive_number();
uint32_t diskdrive_clusters_free();
uint32_t diskdrive_cluster_size();

#endif /* FATFS_DISK_INTERFACE_H_ */
