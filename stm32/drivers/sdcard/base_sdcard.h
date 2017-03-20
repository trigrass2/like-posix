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
 * This file is part of the like-posix project, <https://github.com/drmetal/like-posix>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

#ifndef BASE_SDCARD_H_
#define BASE_SDCARD_H_

#define SD_DATATIMEOUT           ((uint32_t)100000000)

#define SD_CARD_PORT                GPIOC
#define SD_CARD_CK_PIN              GPIO_PIN_12
#define SD_CARD_D0_PIN              GPIO_PIN_8
#define SD_CARD_D1_PIN              GPIO_PIN_9
#define SD_CARD_D2_PIN              GPIO_PIN_10
#define SD_CARD_D3_PIN              GPIO_PIN_11
#define SD_CARD_CMD_PORT            GPIOD
#define SD_CARD_CMD_PIN             GPIO_PIN_2

#define SD_SECTOR_SIZE 512

#if SDCARD_DRIVER_MODE == SDCARD_DRIVER_MODE_SDIO_4BIT || SDCARD_DRIVER_MODE == SDCARD_DRIVER_MODE_SDIO_1BIT

#define __DMAx_TxRx_CLK_ENABLE            __HAL_RCC_DMA2_CLK_ENABLE

#if FAMILY == STM32F4

#define SD_DMAx_Tx_CHANNEL                DMA_CHANNEL_4
#define SD_DMAx_Rx_CHANNEL                DMA_CHANNEL_4
#define SD_DMAx_Tx_STREAM                 DMA2_Stream6
#define SD_DMAx_Rx_STREAM                 DMA2_Stream3
#define SD_DMAx_Tx_IRQn                   DMA2_Stream6_IRQn
#define SD_DMAx_Rx_IRQn                   DMA2_Stream3_IRQn

#define SD_IRQHandler                     SDIO_IRQHandler
#define SD_DMA_Tx_IRQHandler              DMA2_Stream6_IRQHandler
#define SD_DMA_Rx_IRQHandler              DMA2_Stream3_IRQHandler

#elif FAMILY == STM32F1

#define SD_IRQHandler                     SDIO_IRQHandler
#define SD_DMA_TxRx_IRQHandler            DMA2_Channel4_5_IRQHandler
#define SD_DMAx_TxRx_IRQn                 DMA2_Channel4_5_IRQn
#define SD_DMAx_TxRx_CHANNEL              DMA2_Channel4
#define SD_DMAx                           DMA2

#endif

#elif SDCARD_DRIVER_MODE == SDCARD_DRIVER_MODE_SPI


#endif

#endif // BASE_SDCARD_H_

