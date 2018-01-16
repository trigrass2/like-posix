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

#include <string.h>
#include "adc_stream.h"

static uint16_t adc_stream_buffer[ADC_STREAM_CHANNEL_COUNT * ADC_STREAM_BUFFER_LENGTH];
static stream_connection_t* adc_stream_connections[ADC_STREAM_MAX_CONNECTIONS];

static ADC_HandleTypeDef adc_stream_hadc =
{
	.Instance = ADC_STREAM_MASTER_ADC,
	.Init = {
#if FAMILY == STM32F4
		.ClockPrescaler = ADC_STREAM_ADC_CLOCK_DIV,
		.Resolution = ADC_RESOLUTION_12B,
		.EOCSelection = ADC_EOC_SEQ_CONV,
		.DMAContinuousRequests = ENABLE,
		.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING,
#endif
		.DataAlign = ADC_STREAM_ALIGNMENT,
		.ScanConvMode = ADC_STREAM_SCANCONVMODE,
		.ContinuousConvMode = ADC_STREAM_CONTINUOUSCONV,
		.NbrOfConversion = ADC_STREAM_CHANNEL_COUNT,
		.DiscontinuousConvMode = DISABLE,
		.NbrOfDiscConversion = 1,
		.ExternalTrigConv = ADC_STREAM_TRIGGER_SOURCE,
	},
#if FAMILY == STM32F4
	.NbrOfCurrentConversionRank = 0,
#endif
	.DMA_Handle = NULL,
	.Lock = HAL_UNLOCKED,
	.State = HAL_ADC_STATE_RESET,
	.ErrorCode = HAL_ADC_ERROR_NONE,
};

stream_t adc_stream;

static void adc_sampler_init_adc();



/**
 * this driver implements an interrupt driven adc data server (implements a stream server interface).
 *
 * usage:
 *
 *  // implement adc_stream_config.h (use template adc_stream_config.h.in) to configure the server
 *  // then call the following functions to set up...
 *
 *	// start stream server
 *	adc_stream_init();
 *	adc_stream_start();
 *	adc_stream_set_samplerate(2000);
 *
 *	// start stream client (up to ADC_STREAM_MAX_CONNECTIONS clients may be added)
 *	stream_connection_t adc_stream_conn;
 *	stream_connection_init(&adc_stream_conn, adc_stream_callback, "adc process", "some app context data");
 *	adc_stream_connect_service(&adc_stream_conn, 0);
 *
 *  // start/stop clients
 *	stream_connection_enable(&adc_stream_conn, true);
 *
 *	// start/ stop the whole stream
 *	adc_stream_stop();
 *
 *	the function adc_stream_callback() receives buffered data as it is received.
 *  - the data in the buffer reads like this:
 *    {ADC1.ChannelN.Sample0, ADC1.ChannelM.Sample0, ... , ADC1.ChannelN.Sample1, ADC1.ChannelM.Sample1, ... }
 *    for all channels, and the number of samples specified in adc_stream_config.h.
 *  - employs a split circular buffer, half of it is presented to adc_stream_callback() at a time
 *  - the number of samples received at a time is (ADC_STREAM_BUFFER_LENGTH/ADC_STREAM_CHANNEL_COUNT)/2
 *  - the amount of memory used by the buffer in total is ADC_STREAM_BUFFER_LENGTH * ADC_STREAM_CHANNEL_COUNT * sizeof(uint16_t)
 *  - the stream server api in stream.h may be used
 *
 *  // example adc_stream_callback()
 *  // prints all samples
 *  // use unsigned_stream_type_t or signed_stream_type_t as needed.
 *  void adc_stream_callback(unsigned_stream_type_t* buffer, uint16_t length, uint8_t channels, stream_connection_t* conn)
 *  {
 *  	uint32_t sample;
 *  	uint32_t channel;
 *
 *     	for (sample = 0; sample < length; sample += channels) {
 *     		for (channel = 0; channel < channels; channel++) {
 *     			prinf("sample %d channel %d value %d\n", sample, channel, buffer[sample + channel]);
 *  		}
 *     	}
 *  }
 */
void adc_stream_init()
{
    uint32_t resolution = ADC_STREAM_ALIGNMENT == ADC_DATAALIGN_LEFT ? 65536 : 4096;

    init_stream(&adc_stream, "adc_stream", ADC_STREAM_DEFAULT_SAMPLERATE,
            ADC_STREAM_MAX_CONNECTIONS, adc_stream_buffer, adc_stream_connections,
            ADC_STREAM_BUFFER_LENGTH, ADC_STREAM_CHANNEL_COUNT, ADC_STREAM_THREAD_PRIO, ADC_STREAM_THREAD_STACK_SIZE, ADC_FULL_SCALE_AMPLITUDE_MV, resolution);


    adc_stream_init_local_io();
    adc_sampler_init_adc();
    adc_stream_init_samplerate_timer();
}

stream_t* get_adc_stream()
{
	return &adc_stream;
}

void adc_stream_start()
{
    HAL_StatusTypeDef ret = HAL_ADC_Start_IT(&adc_stream_hadc);
    assert_true(ret == HAL_OK);
    _adc_stream_start();
}

void adc_stream_stop()
{
    _adc_stream_stop();
	HAL_StatusTypeDef ret = HAL_ADC_Stop_IT(&adc_stream_hadc);
    assert_true(ret == HAL_OK);
}





static void adc_sampler_init_adc()
{
	uint32_t i;
	uint32_t adc_master_channels[] = ADC_STREAM_MASTER_ADC_CHANNELS;
    ADC_STREAM_MASTER_ADC_CLOCK_ENABLE();
    HAL_StatusTypeDef ret = HAL_ADC_Init(&adc_stream_hadc);
    assert_true(ret == HAL_OK);

    ADC_ChannelConfTypeDef channel_config;
	channel_config.SamplingTime = ADC_STREAM_ADC_CONVERSION_CYCLES;

	// init ADC adc_stream.channels
	for(i=0; i < ADC_STREAM_CHANNEL_COUNT; i++)
	{
		channel_config.Rank = i+1;
		channel_config.Channel = adc_master_channels[i];
		ret = HAL_ADC_ConfigChannel(&adc_stream_hadc, &channel_config);
		assert_true(ret == HAL_OK);
	}

    HAL_NVIC_EnableIRQ(ADC_STREAM_MASTER_ADC_IRQn);
    HAL_NVIC_SetPriority(ADC_STREAM_MASTER_ADC_IRQn, 4, 0);
}

void ADC_STREAM_MASTER_ADC_IRQ_HANDLER()
{
	HAL_ADC_IRQHandler(&adc_stream_hadc);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* adc_stream_hadc)
{
    static BaseType_t xHigherPriorityTaskWoken;
	static uint32_t i = 0;

	adc_stream_buffer[i] = adc_stream_hadc->Instance->DR;
	i++;

#if USE_FREERTOS
	if(i == (ADC_STREAM_CHANNEL_COUNT * ADC_STREAM_BUFFER_LENGTH) / 2) {
	    // half transfer complete
	    xHigherPriorityTaskWoken = pdFALSE;
		adc_stream.buffer = adc_stream._buffer;
		xSemaphoreGiveFromISR(adc_stream.ready, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
	else if(i == ADC_STREAM_CHANNEL_COUNT * ADC_STREAM_BUFFER_LENGTH) {
		i = 0;
	    // transfer complete
	    xHigherPriorityTaskWoken = pdFALSE;
		adc_stream.buffer = adc_stream._buffer + ((adc_stream.length / 2) * adc_stream.channels);
		xSemaphoreGiveFromISR(adc_stream.ready, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
#else
	if(i == (ADC_STREAM_CHANNEL_COUNT * ADC_STREAM_BUFFER_LENGTH) / 2) {
	    // half transfer complete
		assert_true(0);
	}
	else if(i == ADC_STREAM_CHANNEL_COUNT * ADC_STREAM_BUFFER_LENGTH) {
		i = 0;
	    // transfer complete
		assert_true(0);
	}
#endif
}

/**
 * @}
 */
