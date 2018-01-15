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
 * @defgroup adc
 *
 * multi channel on-chip ADC streaming driver.
 *
 * this driver supports a minimum of channels, and may use any number of ADC channels
 * in multiples of 2.
 * simultaneous sampling occurs on ADC1 and ADC2, on the selected channels in succession.
 *
 * example setup for 2 channel usage:
 *
\code

#define ADC_STREAM_MASTER_ADC_CHANNELS     {ADC_Channel_0}
#define ADC_STREAM_SLAVE_ADC_CHANNELS      {ADC_Channel_1}
#define ADC_STREAM_CHANNEL_PORTS           {GPIOA, GPIOA}
#define ADC_STREAM_CHANNEL_PINS            {GPIO_Pin_0,  GPIO_Pin_1}

\endcode
 *
 *
 * example setup for 8 channel usage:
 *
\code

#define ADC_STREAM_MASTER_ADC_CHANNELS     {ADC_Channel_0, ADC_Channel_1, ADC_Channel_2, ADC_Channel_3}
#define ADC_STREAM_SLAVE_ADC_CHANNELS      {ADC_Channel_10, ADC_Channel_11, ADC_Channel_11, ADC_Channel_11}
#define ADC_STREAM_CHANNEL_PORTS           {GPIOA, GPIOA, GPIOA, GPIOA, GPIOC, GPIOC, GPIOC, GPIOC}
#define ADC_STREAM_CHANNEL_PINS            {GPIO_Pin_0,  GPIO_Pin_1, GPIO_Pin_2,  GPIO_Pin_3, \
                                            GPIO_Pin_0, GPIO_Pin_1, GPIO_Pin_2,  GPIO_Pin_3}

\endcode
 *
 *
 * example usage:
 *
\code

#include "adc_stream.h"

void adc_stream_callback(uint16_t* buffer, uint16_t length, uint8_t channels, stream_connection_t* conn)
{
    printf("%s: %x %d %d %d\n", conn->name, buffer, length, channels, conn->stream_channel);
}

stream_connection_t adc_stream_conn;

void start()
{
    adc_stream_init();
    adc_stream_set_samplerate(2000);
    adc_stream_start();

    stream_connection_init(&adc_stream_conn, adc_stream_callback, "adc process", NULL);
    adc_stream_connect_service(&adc_stream_conn, 0);
    stream_connection_enable(&adc_stream_conn, true);
}

\endcode
 *
 * @file adc_stream.c
 * @{
 */

#include <string.h>
#include "asserts.h"
#include "adc_stream.h"
#include "stream_common.h"

#if USE_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#endif


#if(ADC_STREAM_BUFFER_LENGTH%2)
#error "buffer ADC_STREAM_BUFFER_LENGTH must be a multiple of 2"
#endif


static uint16_t adc_stream_buffer[ADC_STREAM_CHANNEL_COUNT * ADC_STREAM_BUFFER_LENGTH];
static stream_connection_t* adc_stream_connections[ADC_STREAM_MAX_CONNECTIONS];

static void init_local_adc();
static void init_local_adc_dma();

stream_t adc_stream;

DMA_HandleTypeDef adc_stream_hdma = {
	.Instance = ADC_STREAM_DMA_INST,
	.Init = {
		.Channel = ADC_STREAM_DMA_STREAM_CHANNEL,
		.Direction = DMA_PERIPH_TO_MEMORY,
		.PeriphInc = DMA_PINC_DISABLE,
		.MemInc = DMA_MINC_ENABLE,
		.PeriphDataAlignment = DMA_PDATAALIGN_WORD,
		.MemDataAlignment = DMA_MDATAALIGN_WORD,
		.Mode = DMA_CIRCULAR,
		.Priority = DMA_PRIORITY_HIGH,
		.FIFOMode = DMA_FIFOMODE_DISABLE,
		.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL,
		.MemBurst = DMA_MBURST_SINGLE,
		.PeriphBurst = DMA_PBURST_SINGLE,
	},
	.Lock = HAL_UNLOCKED,
	.State = HAL_ADC_STATE_RESET,
	.Parent = NULL,
	.ErrorCode = HAL_DMA_ERROR_NONE,
	.StreamBaseAddress = 0,
	.StreamIndex = 0,
};

ADC_HandleTypeDef adc_stream_hadc[ADC_STREAM_UNIQUE_ADCS] = {
	{
		.Instance = ADC_STREAM_MASTER_ADC,
		.Init = {
			.ClockPrescaler = ADC_STREAM_ADC_CLOCK_DIV,
			.Resolution = ADC_RESOLUTION_12B,
			.DataAlign = ADC_STREAM_ALIGNMENT,
			.ScanConvMode = ENABLE,
			.EOCSelection = ADC_EOC_SINGLE_CONV,//ADC_EOC_SEQ_CONV,
			.ContinuousConvMode = ADC_STREAM_CONTINUOUSCONV,
			.DMAContinuousRequests = ENABLE,
			.NbrOfConversion = ADC_STREAM_CHANNEL_COUNT/ADC_STREAM_UNIQUE_ADCS,
			.DiscontinuousConvMode = DISABLE,
			.NbrOfDiscConversion = 1,
			.ExternalTrigConv = ADC_STREAM_TRIGGER_SOURCE,
			.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING,
		},
		.NbrOfCurrentConversionRank = 0,
		.DMA_Handle = &adc_stream_hdma,
		.Lock = HAL_UNLOCKED,
		.State = HAL_ADC_STATE_RESET,
		.ErrorCode = HAL_ADC_ERROR_NONE,
	},
	{
		.Instance = ADC_STREAM_SLAVE_ADC,
		.Init = {
			.ClockPrescaler = ADC_STREAM_ADC_CLOCK_DIV,
			.Resolution = ADC_RESOLUTION_12B,
			.DataAlign = ADC_STREAM_ALIGNMENT,
			.ScanConvMode = ENABLE,
			.EOCSelection = ADC_EOC_SINGLE_CONV,//ADC_EOC_SEQ_CONV,
			.ContinuousConvMode = ADC_STREAM_CONTINUOUSCONV,
			.DMAContinuousRequests = ENABLE,
			.NbrOfConversion = ADC_STREAM_CHANNEL_COUNT/ADC_STREAM_UNIQUE_ADCS,
			.DiscontinuousConvMode = DISABLE,
			.NbrOfDiscConversion = 1,
			.ExternalTrigConv = ADC_STREAM_TRIGGER_SOURCE,
			.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING,
		},
		.NbrOfCurrentConversionRank = 0,
		.DMA_Handle = NULL,
		.Lock = HAL_UNLOCKED,
		.State = HAL_ADC_STATE_RESET,
		.ErrorCode = HAL_ADC_ERROR_NONE,
	}
};


void adc_stream_init()
{
    uint32_t resolution = ADC_STREAM_ALIGNMENT == ADC_DATAALIGN_LEFT ? 65536 : 4096;

    init_stream(&adc_stream, "adc_stream", ADC_STREAM_DEFAULT_SAMPLERATE,
            ADC_STREAM_MAX_CONNECTIONS, adc_stream_buffer, adc_stream_connections,
            ADC_STREAM_BUFFER_LENGTH, ADC_STREAM_CHANNEL_COUNT, ADC_STREAM_THREAD_PRIO, ADC_STREAM_THREAD_STACK_SIZE, ADC_FULL_SCALE_AMPLITUDE_MV, resolution);

    adc_stream_init_local_io();
    init_local_adc();
    adc_stream_init_samplerate_timer();
}

stream_t* get_adc_stream()
{
	return &adc_stream;
}

void adc_stream_start()
{
    init_local_adc_dma();
    HAL_StatusTypeDef ret = HAL_ADCEx_MultiModeStart_DMA(&adc_stream_hadc[0], (uint32_t*)adc_stream._buffer, sizeof(adc_stream_buffer)/sizeof(uint32_t));
    assert_true(ret == HAL_OK);
    _adc_stream_start();
}

void adc_stream_stop()
{
    _adc_stream_stop();
    HAL_StatusTypeDef ret = HAL_ADCEx_MultiModeStop_DMA(&adc_stream_hadc[0]);
    assert_true(ret == HAL_OK);
}



static void init_local_adc_dma()
{
	ADC_STREAM_DMA_CLOCK_ENABLE();
	HAL_StatusTypeDef ret = HAL_DMA_DeInit(&adc_stream_hdma);
	assert_true(ret == HAL_OK);
	ret = HAL_DMA_Init(&adc_stream_hdma);
    assert_true(ret == HAL_OK);
    __HAL_LINKDMA(&adc_stream_hadc[0], DMA_Handle, adc_stream_hdma);
    log_debug(&adc_stream.log, "DMA initialised");
}

static void init_local_adc()
{
	uint32_t adc_master_channels[] = ADC_STREAM_MASTER_ADC_CHANNELS;
	uint32_t adc_slave_channels[] = ADC_STREAM_SLAVE_ADC_CHANNELS;
	uint32_t i;
	__HAL_RCC_ADC1_CLK_ENABLE();
	__HAL_RCC_ADC2_CLK_ENABLE();

	// dont support single ADC operation at this time
	assert_true(ADC_STREAM_UNIQUE_ADCS == 2);

	HAL_ADC_DeInit(&adc_stream_hadc[0]);
	HAL_ADC_DeInit(&adc_stream_hadc[1]);

	ADC_MultiModeTypeDef mm_config;
	mm_config.DMAAccessMode = ADC_DMAACCESSMODE_2;
	mm_config.Mode = ADC_DUALMODE_REGSIMULT;
	mm_config.TwoSamplingDelay = ADC_TWOSAMPLINGDELAY_5CYCLES;
	HAL_StatusTypeDef ret = HAL_ADCEx_MultiModeConfigChannel(&adc_stream_hadc[0], &mm_config);
	assert_true(ret == HAL_OK);

	HAL_ADC_Init(&adc_stream_hadc[0]);
	HAL_ADC_Init(&adc_stream_hadc[1]);
	//ADC_ExternalTrigConvCmd(ADC_STREAM_SLAVE_ADC, en)

	ADC_ChannelConfTypeDef channel_config;
    channel_config.SamplingTime = ADC_STREAM_ADC_CONVERSION_CYCLES;

    // init ADC adc_stream.channels
    for(i=0; i < (ADC_STREAM_CHANNEL_COUNT/ADC_STREAM_UNIQUE_ADCS); i++)
    {
    	channel_config.Rank = i+1;
    	channel_config.Channel = adc_master_channels[i];
    	ret = HAL_ADC_ConfigChannel(&adc_stream_hadc[0], &channel_config);
    	channel_config.Channel = adc_slave_channels[i];
    	assert_true(ret == HAL_OK);
    	ret = HAL_ADC_ConfigChannel(&adc_stream_hadc[1], &channel_config);
    	assert_true(ret == HAL_OK);
    }

    HAL_NVIC_EnableIRQ(ADC_STREAM_DMA_IRQ_CHANNEL);
    HAL_NVIC_SetPriority(ADC_STREAM_DMA_IRQ_CHANNEL, ADC_STREAM_DMA_IRQ_PRIORITY, 0);

    log_debug(&adc_stream.log, "ADC initialised");
}

void ADC_STREAM_DMA_IRQ_HANDLER()
{
	HAL_DMA_IRQHandler(&adc_stream_hdma);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	static BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;
	// transfer complete
	adc_stream.buffer = adc_stream._buffer + ((adc_stream.length / 2) * adc_stream.channels);
	xSemaphoreGiveFromISR(adc_stream.ready, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
    static BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;
	// half transfer complete
	adc_stream.buffer = adc_stream._buffer;
	xSemaphoreGiveFromISR(adc_stream.ready, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


/**
 * @}
 */
