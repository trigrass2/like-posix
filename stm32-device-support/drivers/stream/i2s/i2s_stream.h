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
 * @addtogroup i2s
 *
 * @file i2s_stream.h
 * @{
 */

#ifndef I2S_STREAM_H
#define I2S_STREAM_H

#include <stdint.h>
#include "codec_config.h"

#include "board_config.h"
#include "cutensils.h"
#include "i2s_stream_config.h"
#include "stream_common.h"

#define I2S_STREAM_DMA_CLOCK                RCC_AHB1Periph_DMA1

#if I2S_STREAM_UNIT == 2

#define I2S_STREAM_TX_DMA_STREAM            DMA1_Stream4
#define I2S_STREAM_TX_DMA_CHANNEL           DMA_Channel_0
#define I2S_STREAM_TX_DMA_IRQ_CHANNEL       DMA1_Stream4_IRQn
#define I2S_STREAM_TX_DMA_TC                DMA_IT_TCIF4
#define I2S_STREAM_TX_DMA_HT                DMA_IT_HTIF4
#define I2S_STREAM_TX_DMA_TE                DMA_IT_TEIF4
#define I2S_STREAM_TX_INTERRUPT_HANDLER     DMA1_Stream4_IRQHandler

#define I2S_STREAM_RX_DMA_STREAM            DMA1_Stream3
#define I2S_STREAM_RX_DMA_CHANNEL           DMA_Channel_3
#define I2S_STREAM_RX_DMA_IRQ_CHANNEL       DMA1_Stream3_IRQn
#define I2S_STREAM_RX_DMA_TC                DMA_IT_TCIF3
#define I2S_STREAM_RX_DMA_HT                DMA_IT_HTIF3
#define I2S_STREAM_RX_DMA_TE                DMA_IT_TEIF3
#define I2S_STREAM_RX_INTERRUPT_HANDLER     DMA1_Stream3_IRQHandler

#define I2S_STREAM_I2S_PERIPHERAL           SPI2
#define I2S_STREAM_I2S_EXT_PERIPHERAL       I2S2ext
#define I2S_STREAM_I2S_CLOCK                RCC_APB1Periph_SPI2
#define I2S_STREAM_I2S_ALT_FUNCTION         GPIO_AF_SPI2
#define I2S_STREAM_I2S_EXT_ALT_FUNCTION     GPIO_AF_I2S2ext

#define I2S_STREAM_SD_PORT                  GPIOB
#define I2S_STREAM_SD_PIN                   GPIO_Pin_15
#define I2S_STREAM_SD_PINSOURCE             GPIO_PinSource15
#define I2S_STREAM_EXT_SD_PORT              GPIOB
#define I2S_STREAM_EXT_SD_PIN               GPIO_Pin_14
#define I2S_STREAM_EXT_SD_PINSOURCE         GPIO_PinSource14
#define I2S_STREAM_CK_PORT                  GPIOB
#define I2S_STREAM_CK_PIN                   GPIO_Pin_13
#define I2S_STREAM_CK_PINSOURCE             GPIO_PinSource13
#define I2S_STREAM_WS_PORT                  GPIOB
#define I2S_STREAM_WS_PIN                   GPIO_Pin_12
#define I2S_STREAM_WS_PINSOURCE             GPIO_PinSource12
#define I2S_STREAM_MCK_PORT                 GPIOC
#define I2S_STREAM_MCK_PIN                  GPIO_Pin_6
#define I2S_STREAM_MCK_PINSOURCE            GPIO_PinSource6

#elif I2S_STREAM_UNIT == 3

#define I2S_STREAM_TX_DMA_STREAM            DMA1_Stream7
#define I2S_STREAM_TX_DMA_CHANNEL           DMA_Channel_0
#define I2S_STREAM_TX_DMA_IRQ_CHANNEL       DMA1_Stream7_IRQn
#define I2S_STREAM_TX_DMA_TC                DMA_IT_TCIF7
#define I2S_STREAM_TX_DMA_HT                DMA_IT_HTIF7
#define I2S_STREAM_TX_DMA_TE                DMA_IT_TEIF7
#define I2S_STREAM_TX_INTERRUPT_HANDLER     DMA1_Stream7_IRQHandler

#define I2S_STREAM_RX_DMA_STREAM            DMA1_Stream0
#define I2S_STREAM_RX_DMA_CHANNEL           DMA_Channel_3
#define I2S_STREAM_RX_DMA_IRQ_CHANNEL       DMA1_Stream0_IRQn
#define I2S_STREAM_RX_DMA_TC                DMA_IT_TCIF0
#define I2S_STREAM_RX_DMA_HT                DMA_IT_HTIF0
#define I2S_STREAM_RX_DMA_TE                DMA_IT_TEIF0
#define I2S_STREAM_RX_INTERRUPT_HANDLER     DMA1_Stream0_IRQHandler

#define I2S_STREAM_I2S_PERIPHERAL           SPI3
#define I2S_STREAM_I2S_EXT_PERIPHERAL       I2S3ext
#define I2S_STREAM_I2S_CLOCK                RCC_APB1Periph_SPI3
#define I2S_STREAM_I2S_ALT_FUNCTION         GPIO_AF_SPI3
#define I2S_STREAM_I2S_EXT_ALT_FUNCTION     GPIO_AF_I2S3ext

#define I2S_STREAM_SD_PORT                  GPIOB
#define I2S_STREAM_SD_PIN                   GPIO_Pin_5
#define I2S_STREAM_SD_PINSOURCE             GPIO_PinSource5
#define I2S_STREAM_EXT_SD_PORT              GPIOB
#define I2S_STREAM_EXT_SD_PIN               GPIO_Pin_4
#define I2S_STREAM_EXT_SD_PINSOURCE         GPIO_PinSource4
#define I2S_STREAM_CK_PORT                  GPIOB
#define I2S_STREAM_CK_PIN                   GPIO_Pin_3
#define I2S_STREAM_CK_PINSOURCE             GPIO_PinSource3
#define I2S_STREAM_WS_PORT                  GPIOA
#define I2S_STREAM_WS_PIN                   GPIO_Pin_15
#define I2S_STREAM_WS_PINSOURCE             GPIO_PinSource15
#define I2S_STREAM_MCK_PORT                 GPIOC
#define I2S_STREAM_MCK_PIN                  GPIO_Pin_7
#define I2S_STREAM_MCK_PINSOURCE            GPIO_PinSource7

#endif

#include "stream_defs.h"

void i2s_stream_init();
stream_t* get_i2s_tx_stream();
stream_t* get_i2s_rx_stream();
void i2s_stream_start();
void i2s_stream_stop();
void i2s_stream_set_samplerate(uint32_t samplerate);
uint32_t i2s_stream_get_samplerate();
void i2s_tx_stream_connect_service(stream_connection_t* interface, uint8_t stream_channel);
void i2s_rx_stream_connect_service(stream_connection_t* interface, uint8_t stream_channel);


#endif // I2S_STREAM_H

/**
 * @}
 */
