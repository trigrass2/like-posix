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
 * @defgroup i2s
 *
 * I2S streaming driver. supports 2 channel full duplex operation only.
 *
 * @file i2s_stream.c
 * @{
 */

#include "i2s_stream.h"

#include <string.h>
#include "asserts.h"
#include "stream_common.h"

#if USE_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#endif

#if(I2S_STREAM_BUFFER_LENGTH%2)
#error "buffer I2S_STREAM_BUFFER_LENGTH must be a multiple of 2"
#endif

void init_local_i2s();
void init_local_i2s_dma();
void init_local_i2s_io();


static uint16_t i2s_tx_stream_buffer[I2S_STREAM_CHANNEL_COUNT * I2S_STREAM_BUFFER_LENGTH];
static stream_connection_t* i2s_tx_stream_connections[I2S_STREAM_MAX_CONNECTIONS];
static stream_t i2s_tx_stream;

static uint16_t i2s_rx_stream_buffer[I2S_STREAM_CHANNEL_COUNT * I2S_STREAM_BUFFER_LENGTH];
static stream_connection_t* i2s_rx_stream_connections[I2S_STREAM_MAX_CONNECTIONS];
static stream_t i2s_rx_stream;

void i2s_stream_init()
{
    uint32_t resolution;
    if(I2S_USE_FORMAT == I2S_DataFormat_16b)
        resolution = 65536;
    else if(I2S_USE_FORMAT == I2S_DataFormat_16bextended)
        resolution = 65536;
    else if(I2S_USE_FORMAT == I2S_DataFormat_24b)
        resolution = 16777216;
    else if(I2S_USE_FORMAT == I2S_DataFormat_32b)
        resolution = 4294967295; // hackkk
    else
        assert_true(0);

    init_stream(&i2s_tx_stream, "i2s_tx_stream", I2S_STREAM_DEFAULT_SAMPLERATE,
            I2S_STREAM_MAX_CONNECTIONS, i2s_tx_stream_buffer, i2s_tx_stream_connections,
            I2S_STREAM_BUFFER_LENGTH, I2S_STREAM_CHANNEL_COUNT, 3, 128, I2S_DAC_FULL_SCALE_AMPLITUDE_MV, resolution);
    init_stream(&i2s_rx_stream, "i2s_rx_stream", I2S_STREAM_DEFAULT_SAMPLERATE,
            I2S_STREAM_MAX_CONNECTIONS, i2s_rx_stream_buffer, i2s_rx_stream_connections,
            I2S_STREAM_BUFFER_LENGTH, I2S_STREAM_CHANNEL_COUNT, 3, 128, I2S_ADC_FULL_SCALE_AMPLITUDE_MV, resolution);

    init_local_i2s_io();
    init_local_i2s();

    // enable dma interrupt
    NVIC_InitTypeDef dma_nvic;
    dma_nvic.NVIC_IRQChannel = I2S_STREAM_TX_DMA_IRQ_CHANNEL;
    dma_nvic.NVIC_IRQChannelPreemptionPriority = I2S_STREAM_DMA_IRQ_PRIORITY;
    dma_nvic.NVIC_IRQChannelSubPriority = 0;
    dma_nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&dma_nvic);
    dma_nvic.NVIC_IRQChannel = I2S_STREAM_RX_DMA_IRQ_CHANNEL;
    NVIC_Init(&dma_nvic);
}

stream_t* get_i2s_tx_stream()
{
    return &i2s_tx_stream;
}

stream_t* get_i2s_rx_stream()
{
    return &i2s_rx_stream;
}

void I2S_STREAM_TX_INTERRUPT_HANDLER()
{
#if USE_FREERTOS
    static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xHigherPriorityTaskWoken = pdFALSE;
#endif

    if(DMA_GetITStatus(I2S_STREAM_TX_DMA_STREAM, I2S_STREAM_TX_DMA_TC) == SET)
    {
        // transfer complete
        i2s_tx_stream.buffer = i2s_tx_stream._buffer + ((i2s_tx_stream.length / 2) * i2s_tx_stream.channels);
        DMA_ClearITPendingBit(I2S_STREAM_TX_DMA_STREAM, I2S_STREAM_TX_DMA_TC);
#if USE_FREERTOS
        xSemaphoreGiveFromISR(i2s_tx_stream.ready, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#endif
    }
    if(DMA_GetITStatus(I2S_STREAM_TX_DMA_STREAM, I2S_STREAM_TX_DMA_HT) == SET)
    {
        // half transfer complete
        i2s_tx_stream.buffer = i2s_tx_stream._buffer;
        DMA_ClearITPendingBit(I2S_STREAM_TX_DMA_STREAM, I2S_STREAM_TX_DMA_HT);
#if USE_FREERTOS
        xSemaphoreGiveFromISR(i2s_tx_stream.ready, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#endif
    }
}

void I2S_STREAM_RX_INTERRUPT_HANDLER()
{
#if USE_FREERTOS
    static BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;
#endif

    if(DMA_GetITStatus(I2S_STREAM_RX_DMA_STREAM, I2S_STREAM_RX_DMA_TC) == SET)
    {
        // transfer complete
        i2s_rx_stream.buffer = i2s_rx_stream._buffer + ((i2s_rx_stream.length / 2) * i2s_rx_stream.channels);
        DMA_ClearITPendingBit(I2S_STREAM_RX_DMA_STREAM, I2S_STREAM_RX_DMA_TC);
#if USE_FREERTOS
        xSemaphoreGiveFromISR(i2s_rx_stream.ready, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#endif
    }
    if(DMA_GetITStatus(I2S_STREAM_RX_DMA_STREAM, I2S_STREAM_RX_DMA_HT) == SET)
    {
        // half transfer complete
        i2s_rx_stream.buffer = i2s_rx_stream._buffer;
        DMA_ClearITPendingBit(I2S_STREAM_RX_DMA_STREAM, I2S_STREAM_RX_DMA_HT);
#if USE_FREERTOS
        xSemaphoreGiveFromISR(i2s_rx_stream.ready, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#endif
    }
}

void init_local_i2s_io()
{
    GPIO_InitTypeDef gpioi;
    gpioi.GPIO_Speed = GPIO_Speed_50MHz;
    gpioi.GPIO_Mode = GPIO_Mode_AF;
    gpioi.GPIO_OType = GPIO_OType_PP;
    gpioi.GPIO_PuPd = GPIO_PuPd_NOPULL;

    gpioi.GPIO_Pin = I2S_STREAM_SD_PIN;
    GPIO_Init(I2S_STREAM_SD_PORT, &gpioi);
    GPIO_PinAFConfig(I2S_STREAM_SD_PORT, I2S_STREAM_SD_PINSOURCE, I2S_STREAM_I2S_ALT_FUNCTION);
    gpioi.GPIO_Pin = I2S_STREAM_CK_PIN;
    GPIO_Init(I2S_STREAM_CK_PORT, &gpioi);
    GPIO_PinAFConfig(I2S_STREAM_CK_PORT, I2S_STREAM_CK_PINSOURCE, I2S_STREAM_I2S_ALT_FUNCTION);
    gpioi.GPIO_Pin = I2S_STREAM_MCK_PIN;
    GPIO_Init(I2S_STREAM_MCK_PORT, &gpioi);
    GPIO_PinAFConfig(I2S_STREAM_MCK_PORT, I2S_STREAM_MCK_PINSOURCE, I2S_STREAM_I2S_ALT_FUNCTION);
    gpioi.GPIO_Pin = I2S_STREAM_WS_PIN;
    GPIO_Init(I2S_STREAM_WS_PORT, &gpioi);
    GPIO_PinAFConfig(I2S_STREAM_WS_PORT, I2S_STREAM_WS_PINSOURCE, I2S_STREAM_I2S_ALT_FUNCTION);

    gpioi.GPIO_OType = GPIO_OType_OD;
    gpioi.GPIO_Pin = I2S_STREAM_EXT_SD_PIN;
    GPIO_Init(I2S_STREAM_EXT_SD_PORT, &gpioi);
    GPIO_PinAFConfig(I2S_STREAM_EXT_SD_PORT, I2S_STREAM_EXT_SD_PINSOURCE, I2S_STREAM_I2S_EXT_ALT_FUNCTION);
}

void init_local_i2s()
{
    // clock init
    RCC_APB1PeriphClockCmd(I2S_STREAM_I2S_CLOCK, ENABLE);

    RCC_PLLI2SCmd(ENABLE);

    SPI_I2S_DeInit(I2S_STREAM_I2S_PERIPHERAL);

    I2S_InitTypeDef i2s_init;
    i2s_init.I2S_AudioFreq = i2s_tx_stream.samplerate;
    i2s_init.I2S_MCLKOutput = I2S_USE_MCLKOUT;
    i2s_init.I2S_Mode = I2S_USE_MODE;
    i2s_init.I2S_DataFormat = I2S_USE_FORMAT;
    i2s_init.I2S_Standard = I2S_USE_DATA_STD;
    i2s_init.I2S_CPOL = I2S_USE_CPOL;
    I2S_Init(I2S_STREAM_I2S_PERIPHERAL, &i2s_init);
    log_debug(&i2s_tx_stream.log, "I2S initialised");
    I2S_FullDuplexConfig(I2S_STREAM_I2S_EXT_PERIPHERAL, &i2s_init);
    log_debug(&i2s_rx_stream.log, "I2S initialised");
}

/**
 * @brief   sets up the I2S DMA channel.
 *          DMA is running in word transfer width. 2x 16bit words per transfer.
 * @param   buffer is a pointer to an @ref AudioBuffer object.
 */
void init_local_i2s_dma()
{
    RCC_AHB1PeriphClockCmd(I2S_STREAM_DMA_CLOCK, ENABLE);

    DMA_InitTypeDef dma_init;

    DMA_DeInit(I2S_STREAM_TX_DMA_STREAM);

    dma_init.DMA_Channel = I2S_STREAM_TX_DMA_CHANNEL;
    dma_init.DMA_PeripheralBaseAddr = (uint32_t)&(I2S_STREAM_I2S_PERIPHERAL->DR);
    dma_init.DMA_Memory0BaseAddr = (uint32_t)i2s_tx_stream._buffer;
    dma_init.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    dma_init.DMA_BufferSize = sizeof(i2s_tx_stream_buffer)/sizeof(uint16_t); //Buffer size
    dma_init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dma_init.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    dma_init.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    dma_init.DMA_Mode = DMA_Mode_Circular;
    dma_init.DMA_Priority = DMA_Priority_High;
    dma_init.DMA_FIFOMode = DMA_FIFOMode_Disable;
    dma_init.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
    dma_init.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    dma_init.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

    DMA_Init(I2S_STREAM_TX_DMA_STREAM, &dma_init);
    SPI_I2S_DMACmd(I2S_STREAM_I2S_PERIPHERAL, SPI_I2S_DMAReq_Tx, ENABLE);
    DMA_ITConfig(I2S_STREAM_TX_DMA_STREAM, DMA_IT_TC | DMA_IT_HT, ENABLE);

    log_debug(&i2s_tx_stream.log, "DMA initialised");

    DMA_DeInit(I2S_STREAM_RX_DMA_STREAM);

    dma_init.DMA_Channel = I2S_STREAM_RX_DMA_CHANNEL;
    dma_init.DMA_PeripheralBaseAddr = (uint32_t)&(I2S_STREAM_I2S_EXT_PERIPHERAL->DR);
    dma_init.DMA_Memory0BaseAddr = (uint32_t)i2s_rx_stream._buffer;
    dma_init.DMA_DIR = DMA_DIR_PeripheralToMemory;
    dma_init.DMA_BufferSize = sizeof(i2s_rx_stream_buffer)/sizeof(uint16_t); //Buffer size
    dma_init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dma_init.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    dma_init.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    dma_init.DMA_Mode = DMA_Mode_Circular;
    dma_init.DMA_Priority = DMA_Priority_High;
    dma_init.DMA_FIFOMode = DMA_FIFOMode_Disable;
    dma_init.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
    dma_init.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    dma_init.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

    DMA_Init(I2S_STREAM_RX_DMA_STREAM, &dma_init);
    SPI_I2S_DMACmd(I2S_STREAM_I2S_EXT_PERIPHERAL, SPI_I2S_DMAReq_Rx, ENABLE);
    DMA_ITConfig(I2S_STREAM_RX_DMA_STREAM, DMA_IT_TC | DMA_IT_HT, ENABLE);

    log_debug(&i2s_rx_stream.log, "DMA initialised");
}

/**
 * @brief   start the stream i2s_tx_stream.
 */
void i2s_stream_start()
{
    i2s_tx_stream.buffer = NULL;
    i2s_rx_stream.buffer = NULL;
    //clear the buffer
    memset(i2s_tx_stream._buffer, I2S_STREAM_BUFFER_CLEAR_VALUE, sizeof(i2s_tx_stream_buffer));
    memset(i2s_rx_stream._buffer, I2S_STREAM_BUFFER_CLEAR_VALUE, sizeof(i2s_rx_stream_buffer));

    // setup i2s_tx_stream
    // enable I2S DMA channel
    init_local_i2s_dma();

    DMA_Cmd(I2S_STREAM_TX_DMA_STREAM, ENABLE);
    DMA_Cmd(I2S_STREAM_RX_DMA_STREAM, ENABLE);
    I2S_Cmd(I2S_STREAM_I2S_PERIPHERAL, ENABLE);
    log_debug(&i2s_tx_stream.log, "stream started");
    I2S_Cmd(I2S_STREAM_I2S_EXT_PERIPHERAL, ENABLE);
    log_debug(&i2s_rx_stream.log, "stream started");
}

/**
 * @brief   stop the stream i2s_tx_stream.
 */
void i2s_stream_stop()
{
    i2s_tx_stream.buffer = NULL;
    i2s_rx_stream.buffer = NULL;

    I2S_Cmd(I2S_STREAM_I2S_PERIPHERAL, DISABLE);
    I2S_Cmd(I2S_STREAM_I2S_EXT_PERIPHERAL, DISABLE);

    SPI_I2S_DMACmd(I2S_STREAM_I2S_PERIPHERAL, SPI_I2S_DMAReq_Tx, DISABLE);
    SPI_I2S_DMACmd(I2S_STREAM_I2S_EXT_PERIPHERAL, SPI_I2S_DMAReq_Rx, DISABLE);

    // disable I2S DMA channel
    DMA_Cmd(I2S_STREAM_TX_DMA_STREAM, DISABLE);
    log_debug(&i2s_tx_stream.log, "stream stopped");
    DMA_Cmd(I2S_STREAM_RX_DMA_STREAM, DISABLE);
    log_debug(&i2s_rx_stream.log, "stream stopped");
}

/**
 * @brief   wraps stream_set_samplerate, see stream_common.c for info.
 */
void i2s_stream_set_samplerate(uint32_t samplerate)
{
    i2s_tx_stream.samplerate = i2s_rx_stream.samplerate = samplerate;
	i2s_stream_stop();
    RCC_PLLI2SCmd(DISABLE);
	RCC_APB1PeriphClockCmd(I2S_STREAM_I2S_CLOCK, DISABLE);
	init_local_i2s();
    i2s_stream_start();
}

/**
 * @brief   wraps stream_get_samplerate, see stream_common.c for info.
 */
uint32_t i2s_stream_get_samplerate()
{
    return stream_get_samplerate(&i2s_tx_stream);
}

/**
 * @brief   wraps stream_connect_service, see stream_common.c for info.
 */
void i2s_tx_stream_connect_service(stream_connection_t* interface, uint8_t stream_channel)
{
    stream_connect_service(interface, &i2s_tx_stream, stream_channel);
}

/**
 * @brief   wraps stream_connect_service, see stream_common.c for info.
 */
void i2s_rx_stream_connect_service(stream_connection_t* interface, uint8_t stream_channel)
{
    stream_connect_service(interface, &i2s_rx_stream, stream_channel);
}


/**
 * @}
 */
