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
 * This file is part of the stm32-device-support project, <https://github.com/drmetal/stm32-device-support>
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
#define SDCARD_TASK_STACK        	1024

#endif // SDCARD_CONFIG_H_

\endcode
 *
 * @file sdfs.c
 * @{
 */

#include "sdfs.h"
#include "cutensils.h"

typedef struct {
    FATFS fs;
    TaskHandle_t sdcard_task_handle;
    uint8_t drive;
    bool mounted;
}sdfs_t;

static void sdcard_task(void* pvParameters);

static sdfs_t sdfs;

/**
 * initializes the sdfs task...
 */
void sdfs_init(void)
{
	sdfs.drive = 0;
	sdfs.mounted = false;
	log_syslog(NULL, "sdfs init");

    xTaskCreate(sdcard_task,
               "sdcard",
               configMINIMAL_STACK_SIZE + SDCARD_TASK_STACK,
               NULL,
               tskIDLE_PRIORITY + SDCARD_TASK_PRIORITY,
               &sdfs.sdcard_task_handle);
}

void sdcard_task(void* pvParameters)
{
    (void)pvParameters;

    for(;;)
    {
    	// after power on or any card not present event, wait a while with the IO in an idle state
    	f_mount(sdfs.drive, NULL);
        set_diskstatus(SD_NOT_PRESENT);
    	SD_DeInit();
    	vTaskDelay(250/portTICK_RATE_MS);

    	log_syslog(NULL, "wait for disk");
        // wait for disk
        while(SD_Detect() != SD_PRESENT)
        	 vTaskDelay(100/portTICK_RATE_MS);

        if(!(disk_initialize(sdfs.drive) & STA_NOINIT))
        {
        	sdfs.mounted = f_mount(sdfs.drive, &sdfs.fs) == FR_OK;
        	set_diskstatus(SD_PRESENT);
        }

        // wait for disk out
        while((SD_Detect() == SD_PRESENT) && (get_diskstatus() == SD_PRESENT))
            vTaskDelay(250/portTICK_RATE_MS);

    	log_syslog(NULL, "disk out");
    }
}


bool sdfs_ready()
{
    return sdfs.mounted;
}

uint32_t sdfs_card_capacity()
{
    uint32_t capacity = 0;
    uint32_t sectorsize = 0;
    uint32_t sectorcount = 0;
	disk_ioctl(sdfs.drive, GET_SECTOR_SIZE, &sectorsize);
	disk_ioctl(sdfs.drive, GET_SECTOR_COUNT, &sectorcount);
	capacity = sectorsize * (sectorcount / 1024);
    return capacity;
}

uint32_t sdfs_sector_size()
{
    uint32_t sectorsize = 0;
    disk_ioctl(sdfs.drive, GET_SECTOR_SIZE, &sectorsize);
    return sectorsize;
}

uint8_t sdfs_card_type()
{
    uint8_t cardtype = SDIO_UNKNOWN_CARD_TYPE;
    disk_ioctl(sdfs.drive, MMC_GET_TYPE, &cardtype);
    return cardtype;
}

/**
 * @}
 */
