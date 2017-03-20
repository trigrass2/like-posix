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
 * @defgroup adc_thru_dac
 *
 * multi ADC loop through to DAC driver.
 *
 * @file adc_thru_dac.c
 * @{
 */
#include <unistd.h>
#include "board_config.h"
#include "adc_stream_config.h"
#include "dac_stream_config.h"
#include "adc_stream.h"
#include "dac_stream.h"
#include "adc_thru_dac.h"
#include "logger.h"
#include "dac_stream.h"
#include "stream_defs.h"
#include "adc_thru_dac.h"
#include "adc_config.h"

//extern uint16_t adc_stream_buffer[ADC_STREAM_CHANNEL_COUNT * ADC_STREAM_BUFFER_LENGTH];
//extern stream_connection_t* adc_stream_connections[ADC_STREAM_MAX_CONNECTIONS];

extern stream_t adc_stream;
extern stream_t dac_stream;

void adc_thru_dac_stream_init()
{
    adc_stream_init();
    dac_stream_init();

    dac_stream._buffer = adc_stream._buffer;

    // enable dma interrupt
    NVIC_InitTypeDef dma_nvic;
    dma_nvic.NVIC_IRQChannel = DAC_STREAM_DMA_IRQ_CHANNEL;
    dma_nvic.NVIC_IRQChannelPreemptionPriority = DAC_STREAM_DMA_IRQ_PRIORITY;
    dma_nvic.NVIC_IRQChannelSubPriority = 0;
    dma_nvic.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&dma_nvic);
}

static void stagger_dac_dma_process(uint16_t* buffer, uint16_t length, uint8_t channels, stream_connection_t* conn)
{
    (void)buffer;
    (void)length;
    (void)channels;
    bool* done = (bool*)(conn->ctx);
    dac_stream_start();
    stream_disconnect_service(conn, &adc_stream);
    *done = true;
}

/**
 * @brief   start the stream adc_thru_dac_stream.
 */
void adc_thru_dac_stream_start()
{
    stream_connection_t conn;
    bool done = false;
    stream_connection_init(&conn, stagger_dac_dma_process, NULL, &done);
    stream_connect_service(&conn, &adc_stream, 0);
    adc_stream_start();

//    while(!done)
//        usleep(10000);
}

/**
 * @brief   stop the stream adc_thru_dac_stream.
 */
void adc_thru_dac_stream_stop()
{
    adc_stream_stop();
    dac_stream_stop();
}

/**
 * @brief   wraps stream_set_samplerate, see stream_common.c for info.
 */
void adc_thru_dac_stream_set_samplerate(uint32_t samplerate)
{
    stream_set_samplerate(&adc_stream, ADC_STREAM_SR_TIMER, ADC_SR_TIMER_CLOCK_RATE, samplerate);
    stream_set_samplerate(&adc_stream, DAC_STREAM_SR_TIMER, DAC_SR_TIMER_CLOCK_RATE, samplerate);
}

/**
 * @brief   wraps stream_get_samplerate, see stream_common.c for info.
 */
uint32_t adc_thru_dac_stream_get_samplerate()
{
    return stream_get_samplerate(&adc_stream);
}

/**
 * @brief   wraps stream_connect_service, see stream_common.c for info.
 */
void adc_thru_dac_stream_connect_service(stream_connection_t* interface, uint8_t stream_channel)
{
    stream_connect_service(interface, &adc_stream, stream_channel);
}


/**
 * @}
 */
