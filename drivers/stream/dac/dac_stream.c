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
 * @defgroup dac
 *
 * 2 channel on-chip DAC streaming driver.
 *
 * example usage:
 *
\code

#include "dac_stream.h"

void dac_stream_callback(uint16_t* buffer, uint16_t length, uint8_t channels, uint8_t channel)
{
    // prints every time the dac buffer is ready to be "filled"
    // normally would put signal processing code here
    printf("%x %d %d %d\n", buffer, length, channels, channel);
}

stream_connection_t dac_stream_conn = {
        .process = dac_stream_callback,
        .name = "dac stream tester",
        .enabled = true
};

void start()
{
    dac_stream_init();
    dac_stream_set_samplerate(22050);
    dac_stream_connect_service(&dac_stream_conn);
    dac_stream_start();
}

\endcode
 *
 * @file dac_stream.c
 * @{
 */
#include "dac_stream.h"

#include <string.h>
#include "asserts.h"

#if USE_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#endif


#if(DAC_STREAM_BUFFER_LENGTH%2)
#error "buffer DAC_STREAM_BUFFER_LENGTH must be a multiple of 2"
#endif

void dac_stream_processing_task(void* p);
void init_samplerate_timer();
void init_local_dac();
void init_local_dac_dma();
void init_local_dac_io();


static uint16_t dac_stream_buffer[DAC_STREAM_CHANNEL_COUNT * DAC_STREAM_BUFFER_LENGTH];
static stream_connection_t* dac_stream_connections[DAC_STREAM_MAX_CONNECTIONS];

static stream_t dac_stream;

void dac_stream_init()
{
    log_init(&dac_stream.log, "dac_stream");

    // set default samplerate
    dac_stream.samplerate = DAC_STREAM_DEFAULT_SAMPLERATE;
    // clear out service register
    memset(dac_stream.connections, 0, DAC_STREAM_MAX_CONNECTIONS * sizeof(void*));
    dac_stream._buffer = dac_stream_buffer;
    dac_stream.connections = dac_stream_connections;

#if USE_FREERTOS
    dac_stream.ready = xSemaphoreCreateBinary();
    assert_true(dac_stream.ready);

    assert_true(xTaskCreate(dac_stream_processing_task,
            "dac_stream",
            configMINIMAL_STACK_SIZE + 128,
            NULL,
            tskIDLE_PRIORITY + 3,
            NULL) == pdPASS);
#endif

    init_local_dac_io();
    init_local_dac();
    init_samplerate_timer();

    // enable dma interrupt
    NVIC_InitTypeDef dma_nvic =
    {
        .NVIC_IRQChannel = DAC_STREAM_DMA_IRQ_CHANNEL,
        .NVIC_IRQChannelPreemptionPriority = DAC_STREAM_DMA_IRQ_PRIORITY,
        .NVIC_IRQChannelSubPriority = 0,
        .NVIC_IRQChannelCmd = ENABLE
    };
    NVIC_Init(&dma_nvic);
}

#if FAMILY == STM32F1

void DAC_STREAM_INTERRUPT_HANDLER()
{
    static BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;

    if(DMA_GetITStatus(DAC_STREAM_DMA_TC) == SET)
    {
        // transfer complete
        dac_stream.buffer = dac_stream._buffer + ((DAC_STREAM_BUFFER_LENGTH / 2) * DAC_STREAM_CHANNEL_COUNT);
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
        dac_stream.buffer = dac_stream._buffer + ((DAC_STREAM_BUFFER_LENGTH / 2) * DAC_STREAM_CHANNEL_COUNT);
        xSemaphoreGiveFromISR(dac_stream.ready, &xHigherPriorityTaskWoken);
        DMA_ClearITPendingBit(DAC_STREAM_DMA_STREAM, DAC_STREAM_DMA_TC);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    if(DMA_GetITStatus(DAC_STREAM_DMA_STREAM, DAC_STREAM_DMA_HT) == SET)
    {
        // half transfer complete
        dac_stream.buffer = dac_stream._buffer;
        xSemaphoreGiveFromISR(dac_stream.ready, &xHigherPriorityTaskWoken);
        DMA_ClearITPendingBit(DAC_STREAM_DMA_STREAM, DAC_STREAM_DMA_HT);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}
#endif

void init_samplerate_timer()
{
    // Audio sample rate, select trigger
    RCC_APB1PeriphClockCmd(DAC_STREAM_SR_TIMER_CLOCK, ENABLE);
    TIM_TimeBaseInitTypeDef input_timer_init =
    {
            .TIM_Period = (DAC_SR_TIMER_CLOCK_RATE / DAC_STREAM_DEFAULT_SAMPLERATE) - 1,
            .TIM_Prescaler = DAC_SR_TIMER_PRESCALER-1,
            .TIM_CounterMode = TIM_CounterMode_Up,             // counter mode
            .TIM_ClockDivision = TIM_CKD_DIV1,                   // clock divider value (1, 2 or 4) (has no effect on OC/PWM)
    };
    TIM_TimeBaseInit(DAC_STREAM_SR_TIMER, &input_timer_init);
    TIM_SelectOutputTrigger(DAC_STREAM_SR_TIMER, DAC_STREAM_SR_TIMER_TRIGGER_OUT); // DAC_ExternalTrigConv_T2_TRGO
    dac_stream_set_samplerate(DAC_STREAM_DEFAULT_SAMPLERATE);
}

void init_local_dac_io()
{
    uint8_t i;
    GPIO_TypeDef* stream_input_ports[DAC_STREAM_CHANNEL_COUNT] = DAC_STREAM_CHANNEL_PORTS;
    uint16_t stream_input_pins[DAC_STREAM_CHANNEL_COUNT] = DAC_STREAM_CHANNEL_PINS;
#if FAMILY == STM32F1
    GPIO_InitTypeDef gpioi = {
            .GPIO_Speed = GPIO_Speed_2MHz,
            .GPIO_Mode = GPIO_Mode_AIN
    };
#elif FAMILY == STM32F4
    GPIO_InitTypeDef gpioi = {
            .GPIO_Speed = GPIO_Speed_2MHz,
            .GPIO_Mode = GPIO_Mode_AN,
            .GPIO_OType = GPIO_OType_OD,
            .GPIO_PuPd = GPIO_PuPd_NOPULL
    };
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
    // Audio DAC init
    // clock init
    RCC_APB1PeriphClockCmd(DAC_STREAM_DAC_CLOCK, ENABLE);

    // we dont support single DAC operation at this time
    assert_true(DAC_STREAM_UNIQUE_DACS == 2);

    DAC_DeInit();

    DAC_InitTypeDef dac_init =
    {
            .DAC_Trigger = DAC_STREAM_TRIGGER_SOURCE,
            .DAC_WaveGeneration = DAC_WaveGeneration_None,
            .DAC_LFSRUnmask_TriangleAmplitude = 0,
            .DAC_OutputBuffer = DAC_OutputBuffer_Enable
    };
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
    dma_init.DMA_PeripheralBaseAddr = (uint32_t)DAC_DHR12LD_Address;
    dma_init.DMA_MemoryBaseAddr = (uint32_t)dac_stream._buffer;
    dma_init.DMA_DIR = DMA_DIR_PeripheralDST;
    dma_init.DMA_BufferSize = sizeof(dac_stream._buffer)/sizeof(uint32_t);
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
    dma_init.DMA_PeripheralBaseAddr = (uint32_t)DAC_DHR12LD_Address;
    dma_init.DMA_Memory0BaseAddr = (uint32_t)dac_stream._buffer; //Destination address
    dma_init.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    dma_init.DMA_BufferSize = sizeof(dac_stream._buffer)/sizeof(uint32_t); //Buffer size
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
    memset(dac_stream._buffer, DAC_STREAM_BUFFER_CLEAR_VALUE, sizeof(dac_stream._buffer));

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

    DMA_Cmd(DAC_STREAM_DMA_CHANNEL, DISABLE);

    log_debug(&dac_stream.log, "dac_stream stopped");
}

/**
 * @brief   sets the stream sample rate.
 * @param   samplerate is the sample rate in Hz, in the range of STREAM_SR_PERIOD_RELOAD_MIN and STREAM_SR_PERIOD_RELOAD_MAX.
 *          if a sample rate out of range is specified, the default of @ref DAC_STREAM_DEFAULT_SAMPLERATE is set.
 * @retval  returns true if the operation was successful, false otherwise.
 */
void dac_stream_set_samplerate(uint32_t samplerate)
{
    dac_stream.samplerate = samplerate;

    if((dac_stream.samplerate < STREAM_SR_MIN) || (dac_stream.samplerate > STREAM_SR_MAX))
    {
        dac_stream.samplerate = DAC_STREAM_DEFAULT_SAMPLERATE;
        log_debug(&dac_stream.log, "invalid sample rate: %d", dac_stream.samplerate);
    }

    log_debug(&dac_stream.log, "sample rate set to %dHz", dac_stream.samplerate);

    DAC_STREAM_SR_TIMER->ARR = (DAC_SR_TIMER_CLOCK_RATE/dac_stream.samplerate)-1;
}

/**
 * @brief   gets the stream sample rate.
 * @retval  returns the sample rate in Hz.
 */
uint32_t dac_stream_get_samplerate()
{
    return dac_stream.samplerate;
}

/**
 * @brief   enters an stream service interface into the service register,
 *          if it is not there already and there is a space for it.
 *          up to DAC_STREAM_MAX_CONNECTIONS services may be registered.
 *          The size of the interface buffer (interface->bufferSize) is set to DAC_STREAM_BUFFER_LENGTH/2.
 *          The number of bytes in the buffer available to the service is calculated by:
 *          size-in-bytes = interface->bufferSize * sizeof(uint16_t) * number-of-DAC_STREAM_CHANNEL_COUNT
 * @param   interface is a pointer to the interface to register.
 */
void dac_stream_connect_service(stream_connection_t* interface)
{
    uint8_t i;
    bool registered = false;

    // check that it is not already registered
    for(i = 0; i < DAC_STREAM_MAX_CONNECTIONS; i++)
    {
        // check already registerd
        if(interface == dac_stream.connections[i])
        {
            log_debug(&dac_stream.log, "service %s already registered", interface->name);
            registered = true;
            break;
        }
    }

    if(!registered)
    {
        for(i = 0; i < DAC_STREAM_MAX_CONNECTIONS; i++)
        {
            // check for an empty space
            if(!dac_stream.connections[i])
            {
                dac_stream.connections[i] = interface;
                log_debug(&dac_stream.log, "service %s registered in slot %d", interface->name, i);
                registered = true;
                break;
            }
        }
    }

    if(!registered)
        log_debug(&dac_stream.log, "no space in service register for %s", interface->name);
}


/**
 * handler for stream processing in main loop.
 */
void dac_stream_processing_task(void* p)
{
    (void)p;
    uint8_t connection;
    uint8_t channel;
    stream_connection_t* interface;

    while(1)
    {
        dac_stream.buffer = NULL;
        xSemaphoreTake(dac_stream.ready, 1000/portTICK_RATE_MS);

        if(!dac_stream.buffer)
            continue;

        // call all registered + enabled service channel buffer read functions
        for(channel = 0; channel < DAC_STREAM_CHANNEL_COUNT; channel++)
        {
            for(connection = 0; connection < DAC_STREAM_MAX_CONNECTIONS; connection++)
            {
                interface = dac_stream.connections[connection];
                if(interface && interface->enabled)
                    interface->process(dac_stream.buffer, DAC_STREAM_BUFFER_LENGTH/2, DAC_STREAM_CHANNEL_COUNT, channel);
            }
        }
    }
}


/**
 * @}
 */
