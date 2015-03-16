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
 * @addtogroup dac
 *
 * @file dac_stream.h
 * @{
 */

#ifndef DAC_STREAM_H
#define DAC_STREAM_H

#include <stdint.h>
#include "board_config.h"
#include "cutensils.h"
#include "dac_config.h"
#include "dac_stream_config.h"
#include "stream_common.h"

#define DAC_STREAM_CHANNEL_COUNT             2

#define DAC_STREAM_SR_TIMER_TRIGGER_OUT    TIM_TRGOSource_Update

#if FAMILY == STM32F1
#define DAC_SR_TIMER_PRESCALER             36
#elif FAMILY == STM32F4
#define DAC_SR_TIMER_PRESCALER             84
#endif

#define DAC_SR_TIMER_CLOCK_RATE            ((SystemCoreClock/2)/DAC_SR_TIMER_PRESCALER)

/**
 * the GPIO ports where all the selected DAC channels appear on the chip.
 */
#define DAC_STREAM_CHANNEL_PORTS           {GPIOA, GPIOA}
/**
 * the GPIO pins (corresponding to the ports above) where all the selected DAC channels appear on the chip.
 */
#define DAC_STREAM_CHANNEL_PINS            {GPIO_Pin_4,  GPIO_Pin_5}

#define DAC_TRIGGER_SRC_MASK                 0xFFC7FFC7

#if DAC_STREAM_SR_TIMER_UNIT == 2
#define DAC_STREAM_SR_TIMER                TIM2
#define DAC_STREAM_SR_TIMER_CLOCK          RCC_APB1Periph_TIM2
#define DAC1_TRIGGER_SRC_MASK              0x00000020
#define DAC2_TRIGGER_SRC_MASK              0x00200000
#if FAMILY == STM32F1
#define DAC_STREAM_DMA_CLOCK               RCC_AHBPeriph_DMA1
#define DAC_STREAM_DMA_CHANNEL             DMA1_Channel2
#define DAC_STREAM_DMA_IRQ_CHANNEL         DMA1_Channel2_IRQn
#define DAC_STREAM_DMA_TC                  DMA1_IT_TC2
#define DAC_STREAM_DMA_HT                  DMA1_IT_HT2
#define DAC_STREAM_DMA_TE                  DMA1_IT_TE2
#define DAC_STREAM_INTERRUPT_HANDLER       DMA1_Channel2_IRQHandler
#endif
#elif DAC_STREAM_SR_TIMER_UNIT == 3
#define DAC_STREAM_SR_TIMER                TIM3
#define DAC_STREAM_SR_TIMER_CLOCK          RCC_APB1Periph_TIM3
#define DAC1_TRIGGER_SRC_MASK              0x00000008
#define DAC2_TRIGGER_SRC_MASK              0x00080000
#if FAMILY == STM32F1
#define DAC_STREAM_DMA_CLOCK               RCC_AHBPeriph_DMA1
#define DAC_STREAM_DMA_CHANNEL             DMA1_Channel3
#define DAC_STREAM_DMA_IRQ_CHANNEL         DMA1_Channel3_IRQn
#define DAC_STREAM_DMA_TC                  DMA1_IT_TC3
#define DAC_STREAM_DMA_HT                  DMA1_IT_HT3
#define DAC_STREAM_DMA_TE                  DMA1_IT_TE3
#define DAC_STREAM_INTERRUPT_HANDLER       DMA1_Channel3_IRQHandler
#endif
#elif DAC_STREAM_SR_TIMER_UNIT == 4
#define DAC_STREAM_SR_TIMER                TIM4
#define DAC_STREAM_SR_TIMER_CLOCK          RCC_APB1Periph_TIM4
#define DAC1_TRIGGER_SRC_MASK              0x00000028
#define DAC2_TRIGGER_SRC_MASK              0x00280000
#if FAMILY == STM32F1
#define DAC_STREAM_DMA_CLOCK               RCC_AHBPeriph_DMA1
#define DAC_STREAM_DMA_CHANNEL             DMA1_Channel7
#define DAC_STREAM_DMA_IRQ_CHANNEL         DMA1_Channel7_IRQn
#define DAC_STREAM_DMA_TC                  DMA1_IT_TC7
#define DAC_STREAM_DMA_HT                  DMA21_IT_HT7
#define DAC_STREAM_DMA_TE                  DMA1_IT_TE7
#define DAC_STREAM_INTERRUPT_HANDLER       DMA1_Channel7_IRQHandler
#endif
#elif DAC_STREAM_SR_TIMER_UNIT == 5
#define DAC_STREAM_SR_TIMER                TIM5
#define DAC_STREAM_SR_TIMER_CLOCK          RCC_APB1Periph_TIM5
#define DAC1_TRIGGER_SRC_MASK              0x00000018
#define DAC2_TRIGGER_SRC_MASK              0x00180000
#if FAMILY == STM32F1
#define DAC_STREAM_DMA_CLOCK               RCC_AHBPeriph_DMA2
#define DAC_STREAM_DMA_CHANNEL             DMA2_Channel2
#define DAC_STREAM_DMA_IRQ_CHANNEL         DMA2_Channel2_IRQn
#define DAC_STREAM_DMA_TC                  DMA2_IT_TC2
#define DAC_STREAM_DMA_HT                  DMA2_IT_HT2
#define DAC_STREAM_DMA_TE                  DMA2_IT_TE2
#define DAC_STREAM_INTERRUPT_HANDLER       DMA2_Channel2_IRQHandler
#endif
#elif DAC_STREAM_SR_TIMER_UNIT == 6
#define DAC_STREAM_SR_TIMER                TIM6
#define DAC_STREAM_SR_TIMER_CLOCK          RCC_APB1Periph_TIM6
#define DAC1_TRIGGER_SRC_MASK              0x00000000
#define DAC2_TRIGGER_SRC_MASK              0x00000000
#if FAMILY == STM32F1
#define DAC_STREAM_DMA_CLOCK               RCC_AHBPeriph_DMA2
#define DAC_STREAM_DMA_CHANNEL             DMA2_Channel3
#define DAC_STREAM_DMA_IRQ_CHANNEL         DMA2_Channel3_IRQn
#define DAC_STREAM_DMA_TC                  DMA2_IT_TC3
#define DAC_STREAM_DMA_HT                  DMA2_IT_HT3
#define DAC_STREAM_DMA_TE                  DMA2_IT_TE3
#define DAC_STREAM_INTERRUPT_HANDLER       DMA2_Channel3_IRQHandler
#endif
#elif DAC_STREAM_SR_TIMER_UNIT == 7
#define DAC_STREAM_SR_TIMER                TIM7
#define DAC_STREAM_SR_TIMER_CLOCK          RCC_APB1Periph_TIM7
#define DAC1_TRIGGER_SRC_MASK              0x00000010
#define DAC2_TRIGGER_SRC_MASK              0x00100000
#if FAMILY == STM32F1
#define DAC_STREAM_DMA_CLOCK               RCC_AHBPeriph_DMA2
#define DAC_STREAM_DMA_CHANNEL             DMA2_Channel4
#define DAC_STREAM_DMA_IRQ_CHANNEL         DMA2_Channel4_IRQn
#define DAC_STREAM_DMA_TC                  DMA2_IT_TC4
#define DAC_STREAM_DMA_HT                  DMA2_IT_HT4
#define DAC_STREAM_DMA_TE                  DMA2_IT_TE4
#define DAC_STREAM_INTERRUPT_HANDLER       DMA2_Channel4_IRQHandler
#endif
#endif

#if FAMILY == STM32F4
#define DAC_STREAM_DMA_CLOCK               RCC_AHB1Periph_DMA1
#define DAC_STREAM_DMA_STREAM              DMA1_Stream5
#define DAC_STREAM_DMA_CHANNEL             DMA_Channel_7
#define DAC_STREAM_DMA_IRQ_CHANNEL         DMA1_Stream5_IRQn
#define DAC_STREAM_DMA_TC                  DMA_IT_TCIF5
#define DAC_STREAM_DMA_HT                  DMA_IT_HTIF5
#define DAC_STREAM_DMA_TE                  DMA_IT_TEIF5
#define DAC_STREAM_INTERRUPT_HANDLER       DMA1_Stream5_IRQHandler
#endif

/**
 * DAC_STREAM_TRIGGER_SOURCE must be set up to match @ref DAC_STREAM_SR_TIMER selection.
 */
#if DAC_STREAM_SR_TIMER_UNIT == 2
#define DAC_STREAM_TRIGGER_SOURCE          DAC_Trigger_T2_TRGO
#elif DAC_STREAM_SR_TIMER_UNIT == 3
#define DAC_STREAM_TRIGGER_SOURCE          DAC_Trigger_T3_TRGO
#elif DAC_STREAM_SR_TIMER_UNIT == 4
#define DAC_STREAM_TRIGGER_SOURCE          DAC_Trigger_T4_TRGO
#elif DAC_STREAM_SR_TIMER_UNIT == 5
#define DAC_STREAM_TRIGGER_SOURCE          DAC_Trigger_T5_TRGO
#elif DAC_STREAM_SR_TIMER_UNIT == 6
#define DAC_STREAM_TRIGGER_SOURCE          DAC_Trigger_T6_TRGO
#elif DAC_STREAM_SR_TIMER_UNIT == 7
#define DAC_STREAM_TRIGGER_SOURCE          DAC_Trigger_T7_TRGO
#endif

#define DAC_DR                             ((uint32_t)(DAC_BASE + 0x20 + DAC_STREAM_ALIGNMENT))

#define DAC_STREAM_MASTER_DAC              DAC_Channel_1
#define DAC_STREAM_SLAVE_DAC               DAC_Channel_2
#define DAC_STREAM_UNIQUE_DACS             2
#define DAC_STREAM_DAC_CLOCK               RCC_APB1Periph_DAC

#include "stream_defs.h"

void dac_stream_init();
void dac_stream_start();
void dac_stream_stop();
void dac_stream_set_samplerate(uint32_t samplerate);
uint32_t dac_stream_get_samplerate();
void dac_stream_connect_service(stream_connection_t* interface, uint8_t stream_channel);

void init_dac_samplerate_timer();
void init_local_dac();
void init_local_dac_dma();
void init_local_dac_io();

#endif // DAC_STREAM_H

/**
 * @}
 */
