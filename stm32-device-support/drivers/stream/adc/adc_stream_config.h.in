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
 * example ADC stream configuration file (place h file at project level)
 *
 * @file adc_stream_config.h
 * @{
 */

#ifndef ADC_STREAM_CONFIG_H
#define ADC_STREAM_CONFIG_H

/**
 * number of channels, minimum of 2, and must be a power of 2
 */
#define ADC_STREAM_CHANNEL_COUNT             2
/**
 * the number of samples in the sample buffer.
 * only half the lengtth of this buffer is accessible in the stream callback function at a time.
 * dont use this in the stream callback, use the length, channels and channel arguments to traverse the buffer.
 */
#define ADC_STREAM_BUFFER_LENGTH             512
/**
 * the maximum number of connections allowed on the stream.
 */
#define ADC_STREAM_MAX_CONNECTIONS           4
/**
 * this is the value used when clearing the buffer.
 */
#define ADC_STREAM_BUFFER_CLEAR_VALUE        0
/**
 * default samplerate. the stream is initialised with this value.
 */
#define ADC_STREAM_DEFAULT_SAMPLERATE        48000

/**
 * channels to use on the master ADC (ADC1).
 */
#define ADC_STREAM_MASTER_ADC_CHANNELS     {ADC_Channel_0}
/**
 * channels to use on the slave ADC (ADC2).
 */
#define ADC_STREAM_SLAVE_ADC_CHANNELS      {ADC_Channel_1}
/**
 * the GPIO ports where all the selected ADC channels appear on the chip.
 */
#define ADC_STREAM_CHANNEL_PORTS           {GPIOA, GPIOA}
/**
 * the GPIO pins (corresponding to the ports above) where all the selected ADC channels appear on the chip.
 */
#define ADC_STREAM_CHANNEL_PINS            {GPIO_Pin_0,  GPIO_Pin_1}
/**
 * the DMA interrupt priority.
 */
#define ADC_STREAM_DMA_IRQ_PRIORITY         4
 /**
  * sets the input sample rate timer - can be 2, 3 or 4 to select
  * TIM2, TIM3, TIM4 respectively.
  */
#define ADC_STREAM_SR_TIMER_UNIT            4
/**
 * sets ADC data alignment, may be ADC_DataAlign_Left or ADC_DataAlign_Right
 */
#define ADC_STREAM_ALIGNMENT                ADC_DataAlign_Left


#endif // ADC_STREAM_CONFIG_H
/**
 *
 */
