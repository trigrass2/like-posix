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
#include "sdcard_type.h"
#include "sdcard_config.h"

#ifndef USE_THREAD_AWARE_SDCARD_DRIVER
#define USE_THREAD_AWARE_SDCARD_DRIVER   0
#endif

#if USE_THREAD_AWARE_SDCARD_DRIVER
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#endif

//extern const char* sderrstr[];
//
//HAL_SD_ErrorTypedef SD_PowerON(void);
//HAL_SD_ErrorTypedef SD_PowerOFF(void);
//HAL_SD_ErrorTypedef SD_Init(SD_CardInfo* sdcardinfo);
//void SD_DeInit(void);
//HAL_SD_ErrorTypedef SD_WaitIOOperation(sdio_wait_on_io_t io_flag);
//HAL_SD_ErrorTypedef SD_QueryStatus(HAL_SD_CardStateTypedef* cardstatus);
//HAL_SD_ErrorTypedef SD_ReadBlock(uint8_t *readbuff, uint32_t sector);
//HAL_SD_ErrorTypedef SD_ReadMultiBlocks(uint8_t *readbuff, uint32_t sector, uint32_t NumberOfBlocks);
//HAL_SD_ErrorTypedef SD_WriteBlock(const uint8_t *writebuff, uint32_t sector);
//HAL_SD_ErrorTypedef SD_WriteMultiBlocks(const uint8_t *writebuff, uint32_t sector, uint32_t NumberOfBlocks);
//HAL_SD_ErrorTypedef SD_Erase(uint32_t startaddr, uint32_t endaddr);
//HAL_SD_TransferStateTypedef SD_GetTransferState(void);
//
//uint8_t SD_Detect(void);
//uint8_t SD_WPDetect(void);

//#define SD_DATATIMEOUT           ((uint32_t)100000000)

#define SD_CARD_PORT                GPIOC
#define SD_CARD_CK_PIN              GPIO_PIN_12
#define SD_CARD_D0_PIN              GPIO_PIN_8
#define SD_CARD_D1_PIN              GPIO_PIN_9
#define SD_CARD_D2_PIN              GPIO_PIN_10
#define SD_CARD_D3_PIN              GPIO_PIN_11
#define SD_CARD_CMD_PORT            GPIOD
#define SD_CARD_CMD_PIN             GPIO_PIN_2

/**
  * @brief  SD detection on its memory slot
  */
#define SD_PRESENT                                 ((uint8_t)0x01)
#define SD_NOT_PRESENT                             ((uint8_t)0x00)

/**
  * @brief  SD write protect detection
  */
#define SD_WRITE_PROTECTED                         ((uint8_t)0x01)
#define SD_NOT_WRITE_PROTECTED                     ((uint8_t)0x00)

#define   MSD_OK                        ((uint8_t)0x00)
#define   MSD_ERROR                     ((uint8_t)0x01)
#define   MSD_ERROR_SD_NOT_PRESENT      ((uint8_t)0x02)

#define __DMAx_TxRx_CLK_ENABLE            __HAL_RCC_DMA2_CLK_ENABLE
#define SD_DMAx_Tx_CHANNEL                DMA_CHANNEL_4
#define SD_DMAx_Rx_CHANNEL                DMA_CHANNEL_4
#define SD_DMAx_Tx_STREAM                 DMA2_Stream6
#define SD_DMAx_Rx_STREAM                 DMA2_Stream3
#define SD_DMAx_Tx_IRQn                   DMA2_Stream6_IRQn
#define SD_DMAx_Rx_IRQn                   DMA2_Stream3_IRQn

#define SD_IRQHandler                     SDIO_IRQHandler
#define SD_DMA_Tx_IRQHandler              DMA2_Stream6_IRQHandler
#define SD_DMA_Rx_IRQHandler              DMA2_Stream3_IRQHandler

#if defined(SD_CARD_NPRES_PIN)
#define SD_DetectIRQHandler()             HAL_GPIO_EXTI_IRQHandler(SD_CARD_NPRES_PIN)
#elif defined(SD_CARD_PRES_PIN)
#define SD_DetectIRQHandler()             HAL_GPIO_EXTI_IRQHandler(SD_CARD_PRES_PIN)
#endif

HAL_SD_ErrorTypedef sd_init(HAL_SD_CardInfoTypedef* uSdCardInfo);
HAL_SD_ErrorTypedef sd_deinit(void);
void set_diskstatus(uint8_t state);
uint8_t get_diskstatus();
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
