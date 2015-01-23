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
 * @file adc_stream.c
 * @{
 */
#include <string.h>
#include "asserts.h"
#include "adc_stream.h"

#if USE_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#endif


#if(ADC_STREAM_BUFFER_LENGTH%2)
#error "buffer ADC_STREAM_BUFFER_LENGTH must be a multiple of 2"
#endif

void adc_stream_processing_task(void* p);
void init_samplerate_timer();
void init_local_adc();
void init_local_adc_dma();
void init_local_adc_io();


static uint16_t adc_stream_buffer[ADC_STREAM_CHANNEL_COUNT * ADC_STREAM_BUFFER_LENGTH];
static stream_connection_t* adc_stream_connections[ADC_STREAM_MAX_CONNECTIONS];

static stream_t adc_stream;

void adc_stream_init()
{
    log_init(&adc_stream.log, "adc_stream");

    // set default samplerate
    adc_stream.samplerate = ADC_STREAM_DEFAULT_SAMPLERATE;
    // clear out service register
    memset(adc_stream.connections, 0, ADC_STREAM_MAX_CONNECTIONS * sizeof(void*));
    adc_stream._buffer = adc_stream_buffer;
    adc_stream.connections = adc_stream_connections;

#if USE_FREERTOS

    adc_stream.ready = xSemaphoreCreateBinary();
    assert_true(adc_stream.ready);

    assert_true(xTaskCreate(adc_stream_processing_task,
            "adc_stream",
            configMINIMAL_STACK_SIZE + 128,
            NULL,
            tskIDLE_PRIORITY + 3,
            NULL) == pdPASS);
#endif

    init_local_adc_io();
    init_local_adc();
    init_samplerate_timer();

    // enable dma interrupt
    NVIC_InitTypeDef dma_nvic =
    {
        .NVIC_IRQChannel = ADC_STREAM_DMA_IRQ_CHANNEL,
        .NVIC_IRQChannelPreemptionPriority = ADC_STREAM_DMA_IRQ_PRIORITY,
        .NVIC_IRQChannelSubPriority = 0,
        .NVIC_IRQChannelCmd = ENABLE
    };
    NVIC_Init(&dma_nvic);
}

#if FAMILY == STM32F1

void ADC_STREAM_INTERRUPT_HANDLER()
{
    static BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;

    if(DMA_GetITStatus(ADC_STREAM_DMA_TC) == SET)
    {
        // transfer complete
        adc_stream.buffer = adc_stream._buffer + ((ADC_STREAM_BUFFER_LENGTH / 2) * ADC_STREAM_CHANNEL_COUNT);
        xSemaphoreGiveFromISR(adc_stream.ready, &xHigherPriorityTaskWoken);
        DMA_ClearITPendingBit(ADC_STREAM_DMA_TC);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    if(DMA_GetITStatus(ADC_STREAM_DMA_HT) == SET)
    {
        // half transfer complete
        adc_stream.buffer = adc_stream._buffer;
        xSemaphoreGiveFromISR(adc_stream.ready, &xHigherPriorityTaskWoken);
        DMA_ClearITPendingBit(ADC_STREAM_DMA_HT);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

#elif FAMILY == STM32F4

void ADC_STREAM_INTERRUPT_HANDLER()
{
    static BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;

    if(DMA_GetITStatus(ADC_STREAM_DMA_STREAM, ADC_STREAM_DMA_TC) == SET)
    {
        // transfer complete
        adc_stream.buffer = adc_stream._buffer + ((ADC_STREAM_BUFFER_LENGTH / 2) * ADC_STREAM_CHANNEL_COUNT);
        xSemaphoreGiveFromISR(adc_stream.ready, &xHigherPriorityTaskWoken);
        DMA_ClearITPendingBit(ADC_STREAM_DMA_STREAM, ADC_STREAM_DMA_TC);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    if(DMA_GetITStatus(ADC_STREAM_DMA_STREAM, ADC_STREAM_DMA_HT) == SET)
    {
        // half transfer complete
        adc_stream.buffer = adc_stream._buffer;
        xSemaphoreGiveFromISR(adc_stream.ready, &xHigherPriorityTaskWoken);
        DMA_ClearITPendingBit(ADC_STREAM_DMA_STREAM, ADC_STREAM_DMA_HT);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}
#endif

void init_samplerate_timer()
{
    // Audio sample rate, select trigger
    RCC_APB1PeriphClockCmd(ADC_STREAM_SR_TIMER_CLOCK, ENABLE);

#if FAMILY == STM32F1
    TIM_TimeBaseInitTypeDef input_timer_init =
    {
            .TIM_Period = (ADC_SR_TIMER_CLOCK_RATE / ADC_STREAM_DEFAULT_SAMPLERATE) - 1,
            .TIM_Prescaler = ADC_SR_TIMER_PRESCALER-1,
            .TIM_CounterMode = TIM_CounterMode_Up,             // counter mode
            .TIM_ClockDivision = TIM_CKD_DIV1,                   // clock divider value (1, 2 or 4) (has no effect on OC/PWM)
    };
    TIM_TimeBaseInit(ADC_STREAM_SR_TIMER, &input_timer_init);

    TIM_OCInitTypeDef input_timer =
    {
            TIM_OCMode_PWM1,            //
            TIM_OutputState_Enable,     // output state
            TIM_OutputState_Enable,     // complementary output state
            0,                          // capture compare pulse value
            TIM_OCPolarity_High,        // output polarity
            TIM_OCPolarity_High,        // complementary output polarity
            TIM_OCIdleState_Set,        // idle state
            TIM_OCIdleState_Set,        // complementary idle state
    };

    if(ADC_STREAM_SR_TIMER == TIM4)
    {
        TIM_OC4Init(ADC_STREAM_SR_TIMER, &input_timer);
        TIM_OC4PreloadConfig(ADC_STREAM_SR_TIMER, TIM_OCPreload_Enable);
        TIM_SetCompare4(ADC_STREAM_SR_TIMER, 1);
    }
    else if(ADC_STREAM_SR_TIMER == ADC_STREAM_SR_TIMER)
    {
        TIM_OC2Init(ADC_STREAM_SR_TIMER, &input_timer);
        TIM_OC2PreloadConfig(ADC_STREAM_SR_TIMER, TIM_OCPreload_Enable);
        TIM_SetCompare2(ADC_STREAM_SR_TIMER, 1);
    }

    TIM_ARRPreloadConfig(ADC_STREAM_SR_TIMER, ENABLE);

#elif FAMILY == STM32F4

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    /* Time base configuration */
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_Period = (ADC_SR_TIMER_CLOCK_RATE / ADC_STREAM_DEFAULT_SAMPLERATE) - 1;
    TIM_TimeBaseStructure.TIM_Prescaler = ADC_SR_TIMER_PRESCALER-1;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(ADC_STREAM_SR_TIMER, &TIM_TimeBaseStructure);
#endif


    TIM_SelectOutputTrigger(ADC_STREAM_SR_TIMER, ADC_STREAM_SR_TIMER_TRIGGER_OUT); // ADC_ExternalTrigConv_T2_TRGO

    adc_stream_set_samplerate(ADC_STREAM_DEFAULT_SAMPLERATE);
}

void init_local_adc_io()
{
    uint8_t i;
    GPIO_TypeDef* stream_input_ports[ADC_STREAM_CHANNEL_COUNT] = ADC_STREAM_CHANNEL_PORTS;
    uint16_t stream_input_pins[ADC_STREAM_CHANNEL_COUNT] = ADC_STREAM_CHANNEL_PINS;
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
    // ADC IO pins
    for(i = 0; i < ADC_STREAM_CHANNEL_COUNT; i++)
    {
        gpioi.GPIO_Pin = stream_input_pins[i];
        GPIO_Init(stream_input_ports[i], &gpioi);
    }
}

#if FAMILY == STM32F1

/**
 * @brief   initialises the on ADC IO pins, and the ADC's themselves.
 *          enables the ADC DMA request.
 */
void init_local_adc()
{
    uint8_t i;

    uint32_t stream_adc_clocks[ADC_STREAM_UNIQUE_ADCS] = ADC_STREAM_UNIQUE_ADC_CLOCKS;
    uint8_t adc_master_channels[ADC_STREAM_CHANNEL_COUNT/2] = ADC_STREAM_MASTER_ADC_CHANNELS;
    uint8_t adc_slave_channels[ADC_STREAM_CHANNEL_COUNT/2] = ADC_STREAM_SLAVE_ADC_CHANNELS;

    // Audio ADC init
    // clock init
    for(i = 0; i < ADC_STREAM_UNIQUE_ADCS; i++)
        RCC_APB2PeriphClockCmd(stream_adc_clocks[i], ENABLE);

    RCC_ADCCLKConfig(ADC_STREAM_ADC_CLOCK_DIV);

    // we dont support single ADC operation at this time
    assert_true(ADC_STREAM_UNIQUE_ADCS == 2);

    ADC_DeInit(ADC_STREAM_MASTER_ADC);
    ADC_DeInit(ADC_STREAM_SLAVE_ADC);

    ADC_InitTypeDef adc_init =
    {
            .ADC_Mode = ADC_Mode_RegSimult,
            .ADC_ScanConvMode = ENABLE,
            .ADC_ContinuousConvMode = DISABLE,
            .ADC_ExternalTrigConv = ADC_STREAM_TRIGGER_SOURCE,
            .ADC_DataAlign = ADC_STREAM_ALIGNMENT,
            .ADC_NbrOfChannel = ADC_STREAM_CHANNEL_COUNT/ADC_STREAM_UNIQUE_ADCS
    };
    // init master
    ADC_Init(ADC_STREAM_MASTER_ADC, &adc_init);
    // init slave, is triggered by master ADC start conversion event
    adc_init.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_Init(ADC_STREAM_SLAVE_ADC, &adc_init);
    ADC_ExternalTrigConvCmd(ADC_STREAM_SLAVE_ADC, ENABLE);

    // init ADC ADC_STREAM_CHANNEL_COUNT
    for(i=0; i < (ADC_STREAM_CHANNEL_COUNT/ADC_STREAM_UNIQUE_ADCS); i++)
    {
        ADC_RegularChannelConfig(ADC_STREAM_MASTER_ADC, adc_master_channels[i], i+1, ADC_STREAM_ADC_CONVERSION_CYCLES);
        ADC_RegularChannelConfig(ADC_STREAM_SLAVE_ADC, adc_slave_channels[i], i+1, ADC_STREAM_ADC_CONVERSION_CYCLES);
    }

    // Enable Audio Channel 1
    ADC_Cmd(ADC_STREAM_MASTER_ADC, ENABLE);
    ADC_ResetCalibration(ADC_STREAM_MASTER_ADC);
    while(ADC_GetResetCalibrationStatus(ADC_STREAM_MASTER_ADC) == SET);
    ADC_StartCalibration(ADC_STREAM_MASTER_ADC);
    while(ADC_GetCalibrationStatus(ADC_STREAM_MASTER_ADC) == SET);

    // Enable Audio Channel 2
    ADC_Cmd(ADC_STREAM_SLAVE_ADC, ENABLE);
    ADC_ResetCalibration(ADC_STREAM_SLAVE_ADC);
    while(ADC_GetResetCalibrationStatus(ADC_STREAM_SLAVE_ADC) == SET);
    ADC_StartCalibration(ADC_STREAM_SLAVE_ADC);
    while(ADC_GetCalibrationStatus(ADC_STREAM_SLAVE_ADC) == SET);

    log_debug(&adc_stream.log, "ADC initialised");
}

#elif FAMILY == STM32F4

void init_local_adc()
{
    uint8_t i;
    uint32_t stream_adc_clocks[ADC_STREAM_UNIQUE_ADCS] = ADC_STREAM_UNIQUE_ADC_CLOCKS;
    uint8_t adc_master_channels[ADC_STREAM_CHANNEL_COUNT/2] = ADC_STREAM_MASTER_ADC_CHANNELS;
    uint8_t adc_slave_channels[ADC_STREAM_CHANNEL_COUNT/2] = ADC_STREAM_SLAVE_ADC_CHANNELS;

    // Audio ADC init
    // clock init
    for(i = 0; i < ADC_STREAM_UNIQUE_ADCS; i++)
        RCC_APB2PeriphClockCmd(stream_adc_clocks[i], ENABLE);

    // we dont support single ADC operation at this time
    assert_true(ADC_STREAM_UNIQUE_ADCS == 2);

    ADC_DeInit();

    ADC_CommonInitTypeDef adc_common_init =
    {
            .ADC_Mode = ADC_DualMode_RegSimult,
            .ADC_Prescaler = ADC_Prescaler_Div2,
            .ADC_DMAAccessMode = ADC_DMAAccessMode_1,
            .ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles
    };

    ADC_CommonInit(&adc_common_init);

    ADC_InitTypeDef adc_init =
    {
            .ADC_Resolution = ADC_Resolution_12b,
            .ADC_ScanConvMode = ENABLE,
            .ADC_ContinuousConvMode = DISABLE,
            .ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising,
            .ADC_ExternalTrigConv = ADC_STREAM_TRIGGER_SOURCE,
            .ADC_DataAlign = ADC_STREAM_ALIGNMENT,
            .ADC_NbrOfConversion = ADC_STREAM_CHANNEL_COUNT/ADC_STREAM_UNIQUE_ADCS,
    };
    ADC_Init(ADC_STREAM_MASTER_ADC, &adc_init);
    ADC_Init(ADC_STREAM_SLAVE_ADC, &adc_init);


    // init ADC ADC_STREAM_CHANNEL_COUNT
    for(i=0; i < (ADC_STREAM_CHANNEL_COUNT/ADC_STREAM_UNIQUE_ADCS); i++)
    {
        ADC_RegularChannelConfig(ADC_STREAM_MASTER_ADC, adc_master_channels[i], i+1, ADC_STREAM_ADC_CONVERSION_CYCLES);
        ADC_RegularChannelConfig(ADC_STREAM_SLAVE_ADC, adc_slave_channels[i], i+1, ADC_STREAM_ADC_CONVERSION_CYCLES);
    }

    /* Enable DMA request after last transfer (Multi-ADC mode)  */
    ADC_MultiModeDMARequestAfterLastTransferCmd(ENABLE);

    /* Enable ADC_STREAM_MASTER_ADC */
    ADC_Cmd(ADC_STREAM_MASTER_ADC, ENABLE);

    /* Enable ADC_STREAM_SLAVE_ADC */
    ADC_Cmd(ADC_STREAM_SLAVE_ADC, ENABLE);

    log_debug(&adc_stream.log, "ADC initialised");
}

#endif

/**
 * @brief   sets up the ADC DMA channel.
 *          DMA is running in word transfer width. 2x 16bit words per transfer.
 * @param   buffer is a pointer to an @ref AudioBuffer object.
 */
#if FAMILY == STM32F1
void init_local_adc_dma()
{
    DMA_InitTypeDef dma_init;

    // Audio adc_stream
    RCC_AHBPeriphClockCmd(ADC_STREAM_DMA_CLOCK, ENABLE);
    DMA_DeInit(ADC_STREAM_DMA_CHANNEL);

    dma_init.DMA_PeripheralBaseAddr = (uint32_t)&(ADC_STREAM_MASTER_ADC->DR);
    dma_init.DMA_MemoryBaseAddr = (uint32_t)adc_stream._buffer;
    dma_init.DMA_DIR = DMA_DIR_PeripheralSRC;
    dma_init.DMA_BufferSize = sizeof(adc_stream._buffer)/sizeof(uint32_t);
    dma_init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dma_init.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
    dma_init.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
    dma_init.DMA_Mode = DMA_Mode_Circular;
    dma_init.DMA_Priority = DMA_Priority_Medium;
    dma_init.DMA_M2M = DMA_M2M_Disable;

    DMA_Init(ADC_STREAM_DMA_CHANNEL, &dma_init);

    DMA_ITConfig(ADC_STREAM_DMA_CHANNEL, DMA_IT_TC|DMA_IT_HT, ENABLE);

    DMA_Cmd(ADC_STREAM_DMA_CHANNEL, ENABLE);

    log_debug(&adc_stream.log, "DMA initialised");
}

#elif FAMILY == STM32F4

void init_local_adc_dma()
{
    RCC_AHB1PeriphClockCmd(ADC_STREAM_DMA_CLOCK, ENABLE);

    DMA_InitTypeDef dma_init;

    DMA_DeInit(ADC_STREAM_DMA_STREAM);

    dma_init.DMA_Channel = ADC_STREAM_DMA_CHANNEL;
    dma_init.DMA_PeripheralBaseAddr = (uint32_t)&ADC_STREAM_MASTER_ADC->DR; //Source address // (uint32_t)0x40012308; // CDR_ADDRESS; Packed ADC_STREAM_MASTER_ADC, ADC_STREAM_SLAVE_ADC
    dma_init.DMA_Memory0BaseAddr = (uint32_t)adc_stream._buffer; //Destination address
    dma_init.DMA_DIR = DMA_DIR_PeripheralToMemory;
    dma_init.DMA_BufferSize = sizeof(adc_stream._buffer)/sizeof(uint16_t); //Buffer size
    dma_init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dma_init.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    dma_init.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    dma_init.DMA_Mode = DMA_Mode_Circular;
    dma_init.DMA_Priority = DMA_Priority_High;
    dma_init.DMA_FIFOMode = DMA_FIFOMode_Enable;
    dma_init.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
    dma_init.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    dma_init.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

    DMA_Init(ADC_STREAM_DMA_STREAM, &dma_init);

    /* Enable DMA Stream Half / Transfer Complete interrupt */
    DMA_ITConfig(ADC_STREAM_DMA_STREAM, DMA_IT_TC | DMA_IT_HT, ENABLE);

    /* ADC_STREAM_DMA_STREAM enable */
    DMA_Cmd(ADC_STREAM_DMA_STREAM, ENABLE);

    log_debug(&adc_stream.log, "DMA initialised");
}

#endif

/**
 * @brief   start the stream adc_stream.
 */
void adc_stream_start()
{
    adc_stream.buffer = NULL;
    //clear the buffer
    memset(adc_stream._buffer, ADC_STREAM_BUFFER_CLEAR_VALUE, sizeof(adc_stream._buffer));

    // setup adc_stream
    // enable ADC DMA channel
    init_local_adc_dma();
    ADC_DMACmd(ADC_STREAM_MASTER_ADC, ENABLE);

    // ADC trigger enable
#if FAMILY == STM32F1
    ADC_ExternalTrigConvCmd(ADC_STREAM_MASTER_ADC, ENABLE);
#elif FAMILY == STM32F4
    ADC_SoftwareStartConv(ADC_STREAM_MASTER_ADC);
#endif
    // enable trigger timer
    TIM_Cmd(ADC_STREAM_SR_TIMER, ENABLE);

    log_debug(&adc_stream.log, "started");
}

/**
 * @brief   stop the stream adc_stream.
 */
void adc_stream_stop()
{
    adc_stream.buffer = NULL;

    // enable trigger timer
    TIM_Cmd(ADC_STREAM_SR_TIMER, DISABLE);
    ADC_DMACmd(ADC_STREAM_MASTER_ADC, DISABLE);
    // ADC trigger disable
#if FAMILY == STM32F1
    ADC_ExternalTrigConvCmd(ADC_STREAM_MASTER_ADC, DISABLE);
#endif

    // disable ADC DMA channel
#if FAMILY == STM32F1
    DMA_Cmd(ADC_STREAM_DMA_CHANNEL, DISABLE);
#elif FAMILY == STM32F4
    DMA_Cmd(ADC_STREAM_DMA_STREAM, DISABLE);
#endif

    log_debug(&adc_stream.log, "stream adc_stream stopped");
}

/**
 * @brief   sets the stream sample rate.
 * @param   samplerate is the sample rate in Hz, in the range of STREAM_SR_PERIOD_RELOAD_MIN and STREAM_SR_PERIOD_RELOAD_MAX.
 *          if a sample rate out of range is specified, the default of @ref ADC_STREAM_DEFAULT_SAMPLERATE is set.
 * @retval  returns true if the operation was successful, false otherwise.
 */
void adc_stream_set_samplerate(uint32_t samplerate)
{
    adc_stream.samplerate = samplerate;

    if((adc_stream.samplerate < STREAM_SR_MIN) || (adc_stream.samplerate > STREAM_SR_MAX))
    {
        adc_stream.samplerate = ADC_STREAM_DEFAULT_SAMPLERATE;
        log_debug(&adc_stream.log, "invalid sample rate: %d", adc_stream.samplerate);
    }

    log_debug(&adc_stream.log, "sample rate set to %dHz", adc_stream.samplerate);

    ADC_STREAM_SR_TIMER->ARR = (ADC_SR_TIMER_CLOCK_RATE/adc_stream.samplerate)-1;
}

/**
 * @brief   gets the stream sample rate.
 * @retval  returns the sample rate in Hz.
 */
uint32_t adc_stream_get_samplerate()
{
    return adc_stream.samplerate;
}

/**
 * @brief   enters an stream service interface into the service register,
 *          if it is not there already and there is a space for it.
 *          up to ADC_STREAM_MAX_CONNECTIONS services may be registered.
 *          The size of the interface buffer (interface->bufferSize) is set to ADC_STREAM_BUFFER_LENGTH/2.
 *          The number of bytes in the buffer available to the service is calculated by:
 *          size-in-bytes = interface->bufferSize * sizeof(uint16_t) * number-of-ADC_STREAM_CHANNEL_COUNT
 * @param   interface is a pointer to the interface to register.
 */
void adc_stream_connect_service(stream_connection_t* interface)
{
    uint8_t i;
    bool registered = false;

    // check that it is not already registered
    for(i = 0; i < ADC_STREAM_MAX_CONNECTIONS; i++)
    {
        // check already registerd
        if(interface == adc_stream.connections[i])
        {
            log_debug(&adc_stream.log, "service %s already registered", interface->name);
            registered = true;
            break;
        }
    }

    if(!registered)
    {
        for(i = 0; i < ADC_STREAM_MAX_CONNECTIONS; i++)
        {
            // check for an empty space
            if(!adc_stream.connections[i])
            {
                adc_stream.connections[i] = interface;
                log_debug(&adc_stream.log, "service %s registered in slot %d", interface->name, i);
                registered = true;
                break;
            }
        }
    }

    if(!registered)
        log_debug(&adc_stream.log, "no space in service register for %s", interface->name);
}


/**
 * handler for stream processing in main loop.
 */
void adc_stream_processing_task(void* p)
{
    (void)p;
    uint8_t connection;
    uint8_t channel;
    stream_connection_t* interface;

    while(1)
    {
        adc_stream.buffer = NULL;
        xSemaphoreTake(adc_stream.ready, 1000/portTICK_RATE_MS);

        if(!adc_stream.buffer)
            continue;

        // call all registered + enabled service channel buffer read functions
        for(channel = 0; channel < ADC_STREAM_CHANNEL_COUNT; channel++)
        {
            for(connection = 0; connection < ADC_STREAM_MAX_CONNECTIONS; connection++)
            {
                interface = adc_stream.connections[connection];
                if(interface && interface->enabled)
                    interface->process(adc_stream.buffer, ADC_STREAM_BUFFER_LENGTH/2, ADC_STREAM_CHANNEL_COUNT, channel);
            }
        }
    }
}


/**
 * @}
 */
