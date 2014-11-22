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
 * @{
 */

#include "board_config.h"

#ifndef SDIO_HW_DEFS_H_
#define SDIO_HW_DEFS_H_

#if FAMILY == STM32F1
#define SDIO_FIFO_ADDRESS                ((uint32_t)0x40018080)
/**
  * @brief  SDIO Intialization Frequency (400KHz max)
  */
#define SDIO_INIT_CLK_DIV                ((uint8_t)0xB2)
/**
  * @brief  SDIO Data Transfer Frequency (25MHz max)
  * F_SDIO = 72MHz/(SDIO_TRANSFER_CLK_DIV+2) = 18MHz
  */
#define SDIO_TRANSFER_CLK_DIV            ((uint8_t)0x02)

#define SD_SDIO_DMA_IRQHANDLER          DMA2_Channel4_5_IRQHandler
#define SD_SDIO_DMA_IRQn               DMA2_Channel4_5_IRQn

#elif FAMILY == STM32F4
#define SDIO_FIFO_ADDRESS                ((uint32_t)0x40012C80)
/**
  * @brief  SDIO Intialization Frequency (400KHz max)
  */
#define SDIO_INIT_CLK_DIV                ((uint8_t)0x76)
/**
  * @brief  SDIO Data Transfer Frequency (25MHz max)
  * F_SDIO = 48MHz/(SDIO_TRANSFER_CLK_DIV+2) = 16MHz
  */
#define SDIO_TRANSFER_CLK_DIV            ((uint8_t)0x01)


#define SD_SDIO_DMA_STREAM3           3
//#define SD_SDIO_DMA_STREAM6           6
#ifdef SD_SDIO_DMA_STREAM3
 #define SD_SDIO_DMA_STREAM            DMA2_Stream3
 #define SD_SDIO_DMA_CHANNEL           DMA_Channel_4
 #define SD_SDIO_DMA_FLAG_FEIF         DMA_FLAG_FEIF3
 #define SD_SDIO_DMA_FLAG_DMEIF        DMA_FLAG_DMEIF3
 #define SD_SDIO_DMA_FLAG_TEIF         DMA_FLAG_TEIF3
 #define SD_SDIO_DMA_FLAG_HTIF         DMA_FLAG_HTIF3
 #define SD_SDIO_DMA_FLAG_TCIF         DMA_FLAG_TCIF3
 #define SD_SDIO_DMA_IRQn              DMA2_Stream3_IRQn
 #define SD_SDIO_DMA_IRQHANDLER        DMA2_Stream3_IRQHandler
#elif defined SD_SDIO_DMA_STREAM6
 #define SD_SDIO_DMA_STREAM            DMA2_Stream6
 #define SD_SDIO_DMA_CHANNEL           DMA_Channel_4
 #define SD_SDIO_DMA_FLAG_FEIF         DMA_FLAG_FEIF6
 #define SD_SDIO_DMA_FLAG_DMEIF        DMA_FLAG_DMEIF6
 #define SD_SDIO_DMA_FLAG_TEIF         DMA_FLAG_TEIF6
 #define SD_SDIO_DMA_FLAG_HTIF         DMA_FLAG_HTIF6
 #define SD_SDIO_DMA_FLAG_TCIF         DMA_FLAG_TCIF6
 #define SD_SDIO_DMA_IRQn              DMA2_Stream6_IRQn
 #define SD_SDIO_DMA_IRQHANDLER        DMA2_Stream6_IRQHandler
#endif /* SD_SDIO_DMA_STREAM3 */

#endif


#define SD_CARD_PORT                GPIOC
#define SD_CARD_CK_PIN              GPIO_Pin_12
#define SD_CARD_D0_PIN              GPIO_Pin_8
#define SD_CARD_D1_PIN              GPIO_Pin_9
#define SD_CARD_D2_PIN              GPIO_Pin_10
#define SD_CARD_D3_PIN              GPIO_Pin_11
#define SD_CARD_CK_PINSOURCE        GPIO_PinSource12
#define SD_CARD_D0_PINSOURCE        GPIO_PinSource8
#define SD_CARD_D1_PINSOURCE        GPIO_PinSource9
#define SD_CARD_D2_PINSOURCE        GPIO_PinSource10
#define SD_CARD_D3_PINSOURCE        GPIO_PinSource11
#define SD_CARD_CMD_PORT            GPIOD
#define SD_CARD_CMD_PIN             GPIO_Pin_2
#define SD_CARD_CMD_PINSOURCE       GPIO_PinSource2


#endif // SDIO_HW_DEFS_H_

/**
  * @}
  */
