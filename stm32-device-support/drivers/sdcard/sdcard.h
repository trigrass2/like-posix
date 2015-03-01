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
#include "sdcard_type.h"

#ifndef USE_THREAD_AWARE_SDCARD_DRIVER
#define USE_THREAD_AWARE_SDCARD_DRIVER   0
#endif

#if USE_THREAD_AWARE_SDCARD_DRIVER
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#endif

extern const char* sderrstr[];

SD_Error SD_PowerON(void);
SD_Error SD_PowerOFF(void);
SD_Error SD_Init(SD_CardInfo* sdcardinfo);
void SD_DeInit(void);
SD_Error SD_GetCardInfo(SD_CardInfo *cardinfo);
SD_Error SD_WaitIOOperation(sdio_wait_on_io_t io_flag);
SD_Error SD_QueryStatus(SDCardState* cardstatus);
SD_Error SD_ReadBlock(uint8_t *readbuff, uint32_t sector);
SD_Error SD_ReadMultiBlocks(uint8_t *readbuff, uint32_t sector, uint32_t NumberOfBlocks);
SD_Error SD_WriteBlock(const uint8_t *writebuff, uint32_t sector);
SD_Error SD_WriteMultiBlocks(const uint8_t *writebuff, uint32_t sector, uint32_t NumberOfBlocks);
SD_Error SD_Erase(uint32_t startaddr, uint32_t endaddr);
SDTransferState SD_GetTransferState(void);

uint8_t SD_Detect(void);
uint8_t SD_WPDetect(void);

void set_diskstatus(uint8_t state);
uint8_t get_diskstatus();

#endif // SDCARD_H_

/**
 * @}
 */
