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
 * @defgroup dac
 *
 * 2 channel on-chip DAC streaming driver.
 *
 * example usage:
 *
\code

#include "dac_stream.h"

void dac_stream_callback(uint16_t* buffer, uint16_t length, uint8_t channels, stream_connection_t* conn)
{
    printf("%s: %x %d %d %d\n", conn->name, buffer, length, channels, conn->stream_channel);
}

stream_connection_t dac_stream_conn;

void start()
{
    dac_stream_init();
    dac_stream_set_samplerate(2000);
    dac_stream_start();

    stream_connection_init(&dac_stream_conn, dac_stream_callback, "dac process", NULL);
    dac_stream_connect_service(&dac_stream_conn, 0);
    stream_connection_enable(&dac_stream_conn, true);
}

\endcode
 *
 * @file dac_stream.c
 * @{
 */

#include <string.h>
#include "asserts.h"
#include "dac_stream.h"
#include "stream_common.h"

#if USE_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#endif


#if(DAC_STREAM_BUFFER_LENGTH%2)
#error "buffer DAC_STREAM_BUFFER_LENGTH must be a multiple of 2"
#endif


static uint16_t dac_stream_buffer[DAC_STREAM_CHANNEL_COUNT * DAC_STREAM_BUFFER_LENGTH];
static stream_connection_t* dac_stream_connections[DAC_STREAM_MAX_CONNECTIONS];

stream_t dac_stream;

void dac_stream_init()
{
    uint32_t resolution = DAC_STREAM_ALIGNMENT == DAC_Align_12b_L ? 65536 : 4096;

    init_stream(&dac_stream, "dac_stream", DAC_STREAM_DEFAULT_SAMPLERATE,
            DAC_STREAM_MAX_CONNECTIONS, dac_stream_buffer, dac_stream_connections,
            DAC_STREAM_BUFFER_LENGTH, DAC_STREAM_CHANNEL_COUNT, DAC_STREAM_THREAD_PRIO,
			DAC_STREAM_THREAD_STACK_SIZE, DAC_FULL_SCALE_AMPLITUDE_MV, resolution);

    init_local_dac_io();
    init_local_dac();
    init_dac_samplerate_timer();

    // enable dma interrupt
    NVIC_InitTypeDef dma_nvic;
    dma_nvic.NVIC_IRQChannel = DAC_STREAM_DMA_IRQ_CHANNEL;
    dma_nvic.NVIC_IRQChannelPreemptionPriority = DAC_STREAM_DMA_IRQ_PRIORITY;
    dma_nvic.NVIC_IRQChannelSubPriority = 0;
    dma_nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&dma_nvic);
}

stream_t* get_dac_stream()
{
	return &dac_stream;
}

#if FAMILY == STM32F1

void DAC_STREAM_INTERRUPT_HANDLER()
{
    static BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;

    if(DMA_GetITStatus(DAC_STREAM_DMA_TC) == SET)
    {
        // transfer complete
        dac_stream.buffer = dac_stream._buffer + ((DAC_STREAM_BUFFER_LENGTH / 2) * dac_stream.channels);
        xSemaphoreGiveFromISR(dac_stream.ready, &xHigherPriorityTaskWoken);
        DMA_ClearITPendingBit(DAC_STREAM_DMA_TC);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    if(DMA_GetITStatus(DAC_STREAM_DMA_HT) == SET)
    {
        // half transfer complete
        dac_stream.buffer = dac_stream._buffer;
        xSemaphoreGiveFromISR(dac_stream.ready, &xHigherPriorityTaskWoken);
        DMA_ClearITPendingBit(DAC_STREAM_DMA_HT);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

#elif FAMILY == STM32F4

void DAC_STREAM_INTERRUPT_HANDLER()
{
    static BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;

    if(DMA_GetITStatus(DAC_STREAM_DMA_STREAM, DAC_STREAM_DMA_TC) == SET)
    {
        // transfer complete
        dac_stream.buffer = dac_stream._buffer;
        xSemaphoreGiveFromISR(dac_stream.ready, &xHigherPriorityTaskWoken);
        DMA_ClearITPendingBit(DAC_STREAM_DMA_STREAM, DAC_STREAM_DMA_TC);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    if(DMA_GetITStatus(DAC_STREAM_DMA_STREAM, DAC_STREAM_DMA_HT) == SET)
    {
        // half transfer complete
        dac_stream.buffer = dac_stream._buffer + ((DAC_STREAM_BUFFER_LENGTH / 2) * dac_stream.channels);
        xSemaphoreGiveFromISR(dac_stream.ready, &xHigherPriorityTaskWoken);
        DMA_ClearITPendingBit(DAC_STREAM_DMA_STREAM, DAC_STREAM_DMA_HT);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}
#endif

void init_dac_samplerate_timer()
{
    // Audio sample rate, select trigger
    RCC_APB1PeriphClockCmd(DAC_STREAM_SR_TIMER_CLOCK, ENABLE);
    TIM_TimeBaseInitTypeDef input_timer_init;
    input_timer_init.TIM_Period = (DAC_SR_TIMER_CLOCK_RATE/dac_stream.samplerate)-1;
    input_timer_init.TIM_Prescaler = DAC_SR_TIMER_PRESCALER - 1;
    input_timer_init.TIM_CounterMode = TIM_CounterMode_Up;             // counter mode
    input_timer_init.TIM_ClockDivision = TIM_CKD_DIV1;                   // clock divider value (1, 2 or 4) (has no effect on OC/PWM)

    TIM_TimeBaseInit(DAC_STREAM_SR_TIMER, &input_timer_init);
    TIM_SelectOutputTrigger(DAC_STREAM_SR_TIMER, DAC_STREAM_SR_TIMER_TRIGGER_OUT);
    dac_stream_set_samplerate(dac_stream.samplerate);
}

void init_local_dac_io()
{
    uint8_t i;
    GPIO_InitTypeDef gpioi;
    GPIO_TypeDef* stream_input_ports[DAC_STREAM_CHANNEL_COUNT] = DAC_STREAM_CHANNEL_PORTS;
    uint16_t stream_input_pins[DAC_STREAM_CHANNEL_COUNT] = DAC_STREAM_CHANNEL_PINS;
#if FAMILY == STM32F1
    gpioi.GPIO_Speed = GPIO_Speed_2MHz;
    gpioi.GPIO_Mode = GPIO_Mode_AIN;
#elif FAMILY == STM32F4
    gpioi.GPIO_Speed = GPIO_Speed_2MHz;
    gpioi.GPIO_Mode = GPIO_Mode_AN;
    gpioi.GPIO_OType = GPIO_OType_OD;
    gpioi.GPIO_PuPd = GPIO_PuPd_NOPULL;
#endif
    // DAC IO pins
    for(i = 0; i < DAC_STREAM_CHANNEL_COUNT; i++)
    {
        gpioi.GPIO_Pin = stream_input_pins[i];
        GPIO_Init(stream_input_ports[i], &gpioi);
    }
}

/**
 * @brief   initialises the on DAC IO pins, and the DAC's themselves.
 *          enables the DAC DMA request.
 */
void init_local_dac()
{
    // we dont support single DAC operation at this time
    assert_true(DAC_STREAM_UNIQUE_DACS == 2);

    // clock init
    RCC_APB1PeriphClockCmd(DAC_STREAM_DAC_CLOCK, ENABLE);

    DAC_DeInit();

    DAC_InitTypeDef dac_init;
    dac_init.DAC_Trigger = DAC_STREAM_TRIGGER_SOURCE;
    dac_init.DAC_WaveGeneration = DAC_WaveGeneration_None;
    dac_init .DAC_LFSRUnmask_TriangleAmplitude = 0;
    dac_init.DAC_OutputBuffer = DAC_OutputBuffer_Enable;

    DAC_Init(DAC_STREAM_MASTER_DAC, &dac_init);
    DAC_Init(DAC_STREAM_SLAVE_DAC, &dac_init);

    // set trigger source
    DAC->CR &= DAC_TRIGGER_SRC_MASK;
    DAC->CR |= DAC1_TRIGGER_SRC_MASK | DAC2_TRIGGER_SRC_MASK;

    DAC_DMACmd(DAC_STREAM_MASTER_DAC, ENABLE);
    DAC_Cmd(DAC_STREAM_MASTER_DAC, ENABLE);
    DAC_Cmd(DAC_STREAM_SLAVE_DAC, ENABLE);

    log_debug(&dac_stream.log, "DAC initialised");
}

/**
 * @brief   sets up the DAC DMA channel.
 *          DMA is running in word transfer width. 2x 16bit words per transfer.
 * @param   buffer is a pointer to an @ref AudioBuffer object.
 */
#if FAMILY == STM32F1
void init_local_dac_dma()
{
    DMA_InitTypeDef dma_init;

    // Audio dac_stream
    RCC_AHBPeriphClockCmd(DAC_STREAM_DMA_CLOCK, ENABLE);
    DMA_DeInit(DAC_STREAM_DMA_CHANNEL);

    // todo how to set DAC data reg address?
    dma_init.DMA_PeripheralBaseAddr = (uint32_t)DAC_DR;
    dma_init.DMA_MemoryBaseAddr = (uint32_t)dac_stream._buffer;
    dma_init.DMA_DIR = DMA_DIR_PeripheralDST;
    dma_init.DMA_BufferSize = sizeof(dac_stream_buffer)/sizeof(uint32_t);
    dma_init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dma_init.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
    dma_init.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
    dma_init.DMA_Mode = DMA_Mode_Circular;
    dma_init.DMA_Priority = DMA_Priority_Medium;
    dma_init.DMA_M2M = DMA_M2M_Disable;

    DMA_Init(DAC_STREAM_DMA_CHANNEL, &dma_init);
    DMA_ITConfig(DAC_STREAM_DMA_CHANNEL, DMA_IT_TC|DMA_IT_HT, ENABLE);
    DMA_Cmd(DAC_STREAM_DMA_CHANNEL, ENABLE);

    log_debug(&dac_stream.log, "DMA initialised");
}

#elif FAMILY == STM32F4

void init_local_dac_dma()
{
    RCC_AHB1PeriphClockCmd(DAC_STREAM_DMA_CLOCK, ENABLE);

    DMA_InitTypeDef dma_init;

    DMA_DeInit(DAC_STREAM_DMA_STREAM);

    dma_init.DMA_Channel = DAC_STREAM_DMA_CHANNEL;
    dma_init.DMA_PeripheralBaseAddr = (uint32_t)DAC_DR;
    dma_init.DMA_Memory0BaseAddr = (uint32_t)dac_stream._buffer; //Destination address
    dma_init.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    dma_init.DMA_BufferSize = sizeof(dac_stream_buffer)/sizeof(uint32_t); //Buffer size
    dma_init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dma_init.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
    dma_init.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
    dma_init.DMA_Mode = DMA_Mode_Circular;
    dma_init.DMA_Priority = DMA_Priority_High;
    dma_init.DMA_FIFOMode = DMA_FIFOMode_Disable;
    dma_init.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
    dma_init.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    dma_init.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

    DMA_Init(DAC_STREAM_DMA_STREAM, &dma_init);

    /* Enable DMA Stream Half / Transfer Complete interrupt */
    DMA_ITConfig(DAC_STREAM_DMA_STREAM, DMA_IT_TC | DMA_IT_HT, ENABLE);

    /* DAC_STREAM_DMA_STREAM enable */
    DMA_Cmd(DAC_STREAM_DMA_STREAM, ENABLE);

    log_debug(&dac_stream.log, "DMA initialised");
}

#endif

/**
 * @brief   start the stream dac_stream.
 */
void dac_stream_start()
{
    dac_stream.buffer = NULL;
    //clear the buffer
    memset(dac_stream._buffer, DAC_STREAM_BUFFER_CLEAR_VALUE, sizeof(dac_stream_buffer));

    // setup dac_stream
    // enable DAC DMA channel
    init_local_dac_dma();

    // DAC trigger enable todo - better API for this?
    DAC->CR |= DAC_CR_TEN1 | DAC_CR_TEN2;

    // enable trigger timer
    TIM_Cmd(DAC_STREAM_SR_TIMER, ENABLE);

    log_debug(&dac_stream.log, "started");
}

/**
 * @brief   stop the stream dac_stream.
 */
void dac_stream_stop()
{
    dac_stream.buffer = NULL;

    // enable trigger timer
    TIM_Cmd(DAC_STREAM_SR_TIMER, DISABLE);
    DAC_DMACmd(DAC_STREAM_MASTER_DAC, DISABLE);

    // DAC trigger disable
    DAC->CR &= ~(DAC_CR_TEN1 | DAC_CR_TEN2);

#if FAMILY == STM32F1
    DMA_Cmd(DAC_STREAM_DMA_CHANNEL, DISABLE);
#elif FAMILY == STM32F4
    DMA_Cmd(DAC_STREAM_DMA_STREAM, DISABLE);
#endif

    log_debug(&dac_stream.log, "dac_stream stopped");
}

/**
 * @brief   wraps stream_set_samplerate, see stream_common.c for info.
 */
void dac_stream_set_samplerate(uint32_t samplerate)
{
    stream_set_samplerate(&dac_stream, DAC_STREAM_SR_TIMER, DAC_SR_TIMER_CLOCK_RATE, samplerate);
}

/**
 * @brief   wraps stream_get_samplerate, see stream_common.c for info.
 */
uint32_t dac_stream_get_samplerate()
{
    return stream_get_samplerate(&dac_stream);
}

/**
 * @brief   wraps stream_connect_service, see stream_common.c for info.
 */
void dac_stream_connect_service(stream_connection_t* interface, uint8_t stream_channel)
{
    stream_connect_service(interface, &dac_stream, stream_channel);
}


/**
 * @}
 */
