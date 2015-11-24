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

/**
 * @addtogroup sdfs
 *
 * @file sdcard.h
 * @{
 */
#ifndef SDCARD_H_
#define SDCARD_H_

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "board_config.h"
#include "sdcard_config.h"
#include "base_sdcard.h"

#define SDCARD_DRIVER_MODE_SPI          1
#define SDCARD_DRIVER_MODE_SDIO_1BIT    2
#define SDCARD_DRIVER_MODE_SDIO_4BIT    3

#ifndef SDCARD_DRIVER_MODE
#error "SDCARD_DRIVER_MODE must be specified in boardname.bsp/sdcard_config.h"
#endif

#define SDIO_STD_CAPACITY_SD_CARD_V1_1             ((uint32_t)0x00000000)
#define SDIO_STD_CAPACITY_SD_CARD_V2_0             ((uint32_t)0x00000001)
#define SDIO_HIGH_CAPACITY_SD_CARD                 ((uint32_t)0x00000002)
#define SDIO_MULTIMEDIA_CARD                       ((uint32_t)0x00000003)
#define SDIO_SECURE_DIGITAL_IO_CARD                ((uint32_t)0x00000004)
#define SDIO_HIGH_SPEED_MULTIMEDIA_CARD            ((uint32_t)0x00000005)
#define SDIO_SECURE_DIGITAL_IO_COMBO_CARD          ((uint32_t)0x00000006)
#define SDIO_HIGH_CAPACITY_MMC_CARD                ((uint32_t)0x00000007)
#define SDIO_UNKNOWN_CARD_TYPE                     ((uint32_t)0x00000008)

enum {
	SD_NOT_PRESENT=0,
	SD_PRESENT=1
};

enum {
	SD_NOT_WRITE_PROTECTED=0,
	SD_WRITE_PROTECTED=1
};

HAL_SD_ErrorTypedef sd_init(HAL_SD_CardInfoTypedef* uSdCardInfo);
HAL_SD_ErrorTypedef sd_deinit(void);
uint8_t sd_detect(void);
uint8_t sd_write_protected(void);
uint8_t sd_read(uint8_t *pData, uint32_t sector, uint32_t sectors);
uint8_t sd_write(uint8_t *pData, uint32_t sector, uint32_t sectors);
uint8_t sd_erase(uint32_t startsector, uint32_t endsector);
void sd_get_card_info(HAL_SD_CardInfoTypedef *CardInfo);
HAL_SD_TransferStateTypedef sd_get_transfer_state(void);


#endif // SDCARD_H_

/**
 * @}
 */
