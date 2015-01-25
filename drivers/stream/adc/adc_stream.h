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
 * @addtogroup adc
 *
 * @file adc_stream.h
 * @{
 */

#ifndef ADC_STREAM_H
#define ADC_STREAM_H

#include <stdint.h>

#include "adc_stream_config.h.in"
#include "board_config.h"
#include "cutensils.h"


#if ADC_STREAM_SR_TIMER_UNIT == 2
#define ADC_STREAM_SR_TIMER                TIM2
#define ADC_STREAM_SR_TIMER_CLOCK          RCC_APB1Periph_TIM2
#if FAMILY == STM32F1
#define ADC_SR_TIMER_PRESCALER             36
#define ADC_STREAM_SR_TIMER_TRIGGER_OUT    TIM_TRGOSource_OC2Ref
#elif FAMILY == STM32F4
#define ADC_SR_TIMER_PRESCALER             84
#define ADC_STREAM_SR_TIMER_TRIGGER_OUT    TIM_TRGOSource_Update
#endif

#elif ADC_STREAM_SR_TIMER_UNIT == 3
#define ADC_STREAM_SR_TIMER                TIM3
#define ADC_STREAM_SR_TIMER_CLOCK          RCC_APB1Periph_TIM3
#define ADC_STREAM_SR_TIMER_TRIGGER_OUT    TIM_TRGOSource_Update
#if FAMILY == STM32F1
#define ADC_SR_TIMER_PRESCALER             36
#elif FAMILY == STM32F4
#define ADC_SR_TIMER_PRESCALER             84
#endif

#elif ADC_STREAM_SR_TIMER_UNIT == 4

#define ADC_STREAM_SR_TIMER                TIM4
#define ADC_STREAM_SR_TIMER_CLOCK          RCC_APB1Periph_TIM4
#define ADC_STREAM_SR_TIMER_TRIGGER_OUT    TIM_TRGOSource_OC4Ref
#if FAMILY == STM32F1
#define ADC_SR_TIMER_PRESCALER             36
#elif FAMILY == STM32F4
#define ADC_SR_TIMER_PRESCALER             84
#endif
#endif

#define ADC_SR_TIMER_CLOCK_RATE            ((SystemCoreClock/2)/ADC_SR_TIMER_PRESCALER)

#if FAMILY == STM32F1
#define ADC_STREAM_DMA_CLOCK               RCC_AHBPeriph_DMA1
#define ADC_STREAM_DMA_CHANNEL             DMA1_Channel1
#define ADC_STREAM_DMA_IRQ_CHANNEL         DMA1_Channel1_IRQn
#define ADC_STREAM_DMA_TC                  DMA1_IT_TC1
#define ADC_STREAM_DMA_HT                  DMA1_IT_HT1
#define ADC_STREAM_DMA_TE                  DMA1_IT_TE1
#define ADC_STREAM_INTERRUPT_HANDLER       DMA1_Channel1_IRQHandler
#elif FAMILY == STM32F4
#define ADC_STREAM_DMA_CLOCK               RCC_AHB1Periph_DMA2
#define ADC_STREAM_DMA_STREAM              DMA2_Stream0
#define ADC_STREAM_DMA_CHANNEL             DMA_Channel_0
#define ADC_STREAM_DMA_IRQ_CHANNEL         DMA2_Stream0_IRQn
#define ADC_STREAM_DMA_TC                  DMA_IT_TCIF0
#define ADC_STREAM_DMA_HT                  DMA_IT_HTIF0
#define ADC_STREAM_DMA_TE                  DMA_IT_TEIF0
#define ADC_STREAM_INTERRUPT_HANDLER       DMA2_Stream0_IRQHandler
#endif

/**
 * ADC clock is 12MHz @ 72MHzAPB2 clock / 6
 */
#define ADC_STREAM_ADC_CLOCK_DIV           RCC_PCLK2_Div6

/**
 * ADC sample rate setting: max would be 12M/28.5=421.052kHz
 */
#if FAMILY == STM32F1
#define ADC_STREAM_ADC_CONVERSION_CYCLES   ADC_SampleTime_28Cycles5
#elif FAMILY == STM32F4
#define ADC_STREAM_ADC_CONVERSION_CYCLES   ADC_SampleTime_28Cycles
#endif
/**
 * ADC_STREAM_TRIGGER_SOURCE must be set up to match @ref ADC_STREAM_SR_TIMER selection.
 * can be ADC_ExternalTrigConv_T4_CC4, ADC_ExternalTrigConv_T2_CC2 or ADC_ExternalTrigConv_T3_TRGO
 */
#if ADC_STREAM_SR_TIMER_UNIT == 2
#if FAMILY == STM32F1
#define ADC_STREAM_TRIGGER_SOURCE          ADC_ExternalTrigConv_T2_CC2
#elif FAMILY == STM32F4
#define ADC_STREAM_TRIGGER_SOURCE          ADC_ExternalTrigConv_T2_TRGO
#endif
#elif ADC_STREAM_SR_TIMER_UNIT == 3
#define ADC_STREAM_TRIGGER_SOURCE          ADC_ExternalTrigConv_T3_TRGO
#elif ADC_STREAM_SR_TIMER_UNIT == 4
#define ADC_STREAM_TRIGGER_SOURCE          ADC_ExternalTrigConv_T4_CC4
#endif

#define ADC_STREAM_MASTER_ADC              ADC1
#define ADC_STREAM_SLAVE_ADC               ADC2
#define ADC_STREAM_UNIQUE_ADCS             2
#define ADC_STREAM_UNIQUE_ADC_CLOCKS       {RCC_APB2Periph_ADC1, RCC_APB2Periph_ADC2}

#include "stream_defs.h"

void adc_stream_init();
void adc_stream_start();
void adc_stream_stop();
void adc_stream_set_samplerate(uint32_t samplerate);
uint32_t adc_stream_get_samplerate();
void adc_stream_connect_service(stream_connection_t* interface);


#endif // ADC_STREAM_H

/**
 * @}
 */
