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

/**
 * @defgroup sdfs SD Card Filesystem
 *
 * provides a little manager for card detection and filesystem mount/dismount.
 *
 * relies upon the configuration file sdcard_config.h:
 *
 * Eg:
 *
\code

#ifndef SDCARD_CONFIG_H_
#define SDCARD_CONFIG_H_

// define active high sd present pin, if used
// #define SD_CARD_PRES_PIN            GPIO_Pin_
// #define SD_CARD_PRES_PORT           GPIO

// define active low sd present pin, if used
#define SD_CARD_NPRES_PIN            GPIO_Pin_3
#define SD_CARD_NPRES_PORT           GPIOD

// define active high write protect pin, if used
// #define SD_CARD_WP_PIN              GPIO_Pin_
// #define SD_CARD_WP_PORT             GPIO

// define active low write protect pin, if used
// #define SD_CARD_NWP_PIN              GPIO_Pin_
// #define SD_CARD_NWP_PORT             GPIO

#define SDCARD_IT_PRIORITY          5
#define SDCARD_TASK_PRIORITY        2
#define SDCARD_TASK_STACK           192
#define SDCARD_DRIVER_MODE			SDCARD_DRIVER_MODE_XXXXXXXX

#endif // SDCARD_CONFIG_H_

\endcode
 *
 * @file sdfs.c
 * @{
 */

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "sdfs.h"
#include "cutensils.h"

typedef struct {
    FATFS fs;
#if USE_FREERTOS
    TaskHandle_t sdcard_task_handle;
#endif
    char* drivemapping;
    char drivenumber;
    char* mountpoint;
    char drivename[13];
    bool mounted;
    logger_t log;
}sdfs_t;

#if USE_FREERTOS
static void sdcard_task(void* pvParameters);
#endif

static sdfs_t sdfs;

/**
 * initializes the sdfs task...
 */
bool sdfs_init(void)
{
    sdfs.drivenumber = 0;
    sdfs.drivemapping = "0:";
    sdfs.drivename[0] = '\0';
    sdfs.mountpoint = "/";
    sdfs.mounted = false;
    log_init(&sdfs.log, "sdfs");
    log_syslog(&sdfs.log, "sdfs init");

#if USE_FREERTOS
    return xTaskCreate(sdcard_task,
               "sdfs",
               configMINIMAL_STACK_SIZE + SDCARD_TASK_STACK,
               NULL,
               tskIDLE_PRIORITY + SDCARD_TASK_PRIORITY,
               &sdfs.sdcard_task_handle) == pdPASS;

#else
    if(f_mount(&sdfs.fs, sdfs.drivemapping, 1) == FR_OK)
		sdfs.mounted = true;

    return sdfs.mounted;
#endif
}

#if USE_FREERTOS
void sdcard_task(void* pvParameters)
{
    (void)pvParameters;
    FRESULT fr;

    for(;;)
    {
    	if(sd_detect() != SD_PRESENT)
    		log_syslog(&sdfs.log, "wait for disk");
		// wait for disk
		while(sd_detect() != SD_PRESENT)
			usleep(100000);

        // after power on or any card not present event, wait a while with the IO in an idle state
        f_mount(NULL, sdfs.drivemapping, 0);
        sdfs.mounted = false;
        sd_deinit();
        fr = f_mount(&sdfs.fs, sdfs.drivemapping, 1);

        if(fr == FR_OK)
           sdfs.mounted = true;
        else
        {
            log_debug(&sdfs.log, "mount failed: %d", fr);
        	sleep(2);
        	continue;
        }

        log_debug(&sdfs.log, "mounted volume '%s'", sdfs.drivemapping);

        while(sdfs.mounted && sd_detect() == SD_PRESENT)
        	usleep(100000);

        log_syslog(&sdfs.log, "disk out");
    }
}
#endif


bool sdfs_ready()
{
    return sdfs.mounted;
    usleep(1000);
}

uint32_t sdfs_card_capacity()
{
    uint32_t capacity = 0;
    uint32_t sectorsize = 0;
    uint32_t sectorcount = 0;
    disk_ioctl(sdfs.drivenumber, GET_SECTOR_SIZE, &sectorsize);
    disk_ioctl(sdfs.drivenumber, GET_SECTOR_COUNT, &sectorcount);
    capacity = sectorsize * (sectorcount / 1024);
    return capacity;
}

char* sdfs_mountpoint()
{
	return sdfs.mountpoint;
}

uint32_t sdfs_sector_count()
{
    uint32_t sectorcount = 0;
    disk_ioctl(sdfs.drivenumber, GET_SECTOR_COUNT, &sectorcount);
    return sectorcount;
}

uint32_t sdfs_sector_size()
{
    uint32_t sectorsize = 0;
    disk_ioctl(sdfs.drivenumber, GET_SECTOR_SIZE, &sectorsize);
    return sectorsize;
}

uint8_t sdfs_card_type()
{
    uint8_t cardtype = SDIO_UNKNOWN_CARD_TYPE;
    disk_ioctl(sdfs.drivenumber, MMC_GET_TYPE, &cardtype);
    return cardtype;
}


char* sdfs_drive_name()
{
	DWORD vsn;
	f_getlabel(sdfs.drivemapping, sdfs.drivename, &vsn);
	return sdfs.drivename;
}

char* sdfs_drive_mapping()
{
	return sdfs.drivemapping;
}

uint32_t sdfs_clusters_free()
{
	FATFS *fs;
	DWORD nclst;
    f_getfree(sdfs.drivemapping, &nclst, &fs);
    return nclst;
}

uint32_t sdfs_cluster_size()
{
    return sdfs.fs.csize;
}

/**
 * @}
 */
