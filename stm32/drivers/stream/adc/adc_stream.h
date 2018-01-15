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

/**
 * @addtogroup adc
 *
 * @file adc_stream.h
 * @{
 */

#ifndef ADC_STREAM_H
#define ADC_STREAM_H

#include <stdint.h>
#include "adc_config.h"

#include "board_config.h"
#include "adc_stream_config.h"
#include "logger.h"
#include "stream_common.h"


#define ADC_SR_TIMER_PRESCALER             (SystemCoreClock/2000000)
#define ADC_SR_TIMER_CLOCK_RATE            ((SystemCoreClock/2)/ADC_SR_TIMER_PRESCALER)

#if ADC_STREAM_SR_TIMER_UNIT == 2
#define ADC_STREAM_SR_TIMER                TIM2
#define ADC_STREAM_SR_TIMER_CLOCK          __TIM2_CLK_ENABLE
#if FAMILY == STM32F1
#define ADC_STREAM_SR_TIMER_TRIGGER_OUT    TIM_TRGO_OC2REF
#define ADC_STREAM_SR_TIMER_OC_CHANNEL    	TIM_CHANNEL_2
#elif FAMILY == STM32F4
#define ADC_STREAM_SR_TIMER_TRIGGER_OUT    TIM_TRGO_UPDATE
#endif

#elif ADC_STREAM_SR_TIMER_UNIT == 3
#define ADC_STREAM_SR_TIMER                TIM3
#define ADC_STREAM_SR_TIMER_CLOCK          __TIM3_CLK_ENABLE
#define ADC_STREAM_SR_TIMER_TRIGGER_OUT    TIM_TRGO_UPDATE

#elif ADC_STREAM_SR_TIMER_UNIT == 4

#define ADC_STREAM_SR_TIMER                TIM4
#define ADC_STREAM_SR_TIMER_CLOCK          __TIM4_CLK_ENABLE
#define ADC_STREAM_SR_TIMER_TRIGGER_OUT    TIM_TRGO_OC4REF
#define ADC_STREAM_SR_TIMER_OC_CHANNEL    	TIM_CHANNEL_4

#endif

#if FAMILY == STM32F1
#define ADC_STREAM_DMA_CLOCK_ENABLE			__HAL_RCC_DMA1_CLK_ENABLE
#define ADC_STREAM_DMA_INST             	DMA1_Channel1
#define ADC_STREAM_DMA_IRQ_CHANNEL         DMA1_Channel1_IRQn
#define ADC_STREAM_DMA_IRQ_HANDLER         DMA1_IRQHandler
#elif FAMILY == STM32F4
#define ADC_STREAM_DMA_CLOCK_ENABLE			__HAL_RCC_DMA2_CLK_ENABLE
#define ADC_STREAM_DMA_INST              	DMA2_Stream0
#define ADC_STREAM_DMA_STREAM_CHANNEL      DMA_CHANNEL_0
#define ADC_STREAM_DMA_IRQ_CHANNEL         DMA2_Stream0_IRQn
#define ADC_STREAM_DMA_IRQ_HANDLER         DMA2_Stream0_IRQHandler
#endif

/**
 * ADC clock is 12MHz @ 72MHzAPB2 clock / 6
 */
#define ADC_STREAM_ADC_CLOCK_DIV           ADC_CLOCK_SYNC_PCLK_DIV6 // todo check ratio for f1 and f4, old f4 version was at /2

/**
 * ADC sample rate setting: max would be 12M/28.5=421.052kHz
 */
#ifndef ADC_STREAM_ADC_CONVERSION_CYCLES
#define ADC_STREAM_ADC_CONVERSION_CYCLES   ADC_SAMPLETIME_28CYCLES
#endif


/**
 * ADC_STREAM_TRIGGER_SOURCE must be set up to match @ref ADC_STREAM_SR_TIMER selection.
 * can be ADC_ExternalTrigConv_T4_CC4, ADC_ExternalTrigConv_T2_CC2 or ADC_ExternalTrigConv_T3_TRGO
 */
#if ADC_STREAM_SR_TIMER_UNIT == 0
#define ADC_STREAM_TRIGGER_SOURCE          ADC_SOFTWARE_START
#define ADC_STREAM_CONTINUOUSCONV          ENABLE
#elif ADC_STREAM_SR_TIMER_UNIT == 2
#if FAMILY == STM32F1
#define ADC_STREAM_TRIGGER_SOURCE          ADC_EXTERNALTRIGCONV_T2_CC2
#elif FAMILY == STM32F4
#define ADC_STREAM_TRIGGER_SOURCE          ADC_EXTERNALTRIGCONV_T2_TRGO
#endif
#define ADC_STREAM_CONTINUOUSCONV          DISABLE
#elif ADC_STREAM_SR_TIMER_UNIT == 3
#define ADC_STREAM_TRIGGER_SOURCE          ADC_EXTERNALTRIGCONV_T3_TRGO
#define ADC_STREAM_CONTINUOUSCONV          DISABLE
#elif ADC_STREAM_SR_TIMER_UNIT == 4
#define ADC_STREAM_TRIGGER_SOURCE          ADC_EXTERNALTRIGCONV_T4_CC4
#define ADC_STREAM_CONTINUOUSCONV          DISABLE
#endif

#define ADC_STREAM_MASTER_ADC              ADC1
#define ADC_STREAM_MASTER_ADC_CLOCK_ENABLE __HAL_RCC_ADC1_CLK_ENABLE
#define ADC_STREAM_SLAVE_ADC               ADC2
#define ADC_STREAM_SLAVE_ADC_CLOCK_ENABLE __HAL_RCC_ADC2_CLK_ENABLE
#define ADC_STREAM_UNIQUE_ADCS             2


#include "stream_defs.h"


stream_t* get_adc_stream();
void adc_stream_init();
void adc_stream_start();
void adc_stream_stop();
void adc_stream_connect_service(stream_connection_t* interface, uint8_t stream_channel);
void adc_stream_set_samplerate(uint32_t samplerate);
uint32_t adc_stream_get_samplerate();

void adc_stream_init_local_io();
void adc_stream_init_samplerate_timer();
void _adc_stream_start();
void _adc_stream_stop();

#endif // ADC_STREAM_H

/**
 * @}
 */
