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

#include "asserts.h"
#include "stream_common.h"

#if USE_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#endif

static void stream_processing_task(stream_t* stream);

/**
 * this file contains common stream control code, the funtions are
 * intended to be used to implement specific stream drivers.
 *
 * see the ADC, DAC and I2S stream drivers for examples.
 */

void init_stream(stream_t* stream, const char* name, uint32_t samplerate,
                uint16_t maxconns, uint16_t* stream_buffer, stream_connection_t** connections,
                uint16_t buffer_length, uint8_t channel_count, uint8_t task_prio, uint16_t task_stack,
                uint16_t full_scale_amplitude, uint32_t resolution)
{
    log_init(&stream->log, name);

    stream->buffer = NULL;
    stream->samplerate = samplerate;
    stream->_buffer = stream_buffer;
    stream->connections = connections;
    stream->channels = channel_count;
    stream->maxconns = maxconns;
    stream->length = buffer_length;
    stream->ready = NULL;
    stream->full_scale_amplitude = full_scale_amplitude;
    stream->resolution = resolution;

    memset(stream->connections, 0, maxconns * sizeof(stream_connection_t*));

#if USE_FREERTOS
    stream->ready = xSemaphoreCreateBinary();
    assert_true(stream->ready);

    assert_true(xTaskCreate((TaskFunction_t)stream_processing_task,
            name,
            configMINIMAL_STACK_SIZE + task_stack,
            stream,
            tskIDLE_PRIORITY + task_prio,
            NULL) == pdPASS);
#endif
}

/**
 * @brief   sets the stream sample rate.
 * @param	stream is the steam to operate on.
 * @param   samplerate_timer refers to the hardware timer used by the stream driver.
 * @param   timer_clockrate is the rate at which samplerate_timer is clocked (after prescaling).
 * @param   samplerate is the sample rate in Hz, in the range of STREAM_SR_PERIOD_RELOAD_MIN and STREAM_SR_PERIOD_RELOAD_MAX.
 *          if a sample rate out of range is specified, the default of @ref DAC_STREAM_DEFAULT_SAMPLERATE is set.
 * @retval  returns true if the operation was successful, false otherwise.
 */
void stream_set_samplerate(stream_t* stream, TIM_TypeDef* samplerate_timer, uint32_t timer_clockrate, uint32_t samplerate)
{
    if((samplerate < STREAM_SR_MIN) || (samplerate > STREAM_SR_MAX))
    {
        log_error(&stream->log, "invalid sample rate: %d", samplerate);
    }
    else
    {
        stream->samplerate = samplerate;
        log_debug(&stream->log, "sample rate set to %dHz", stream->samplerate);
        samplerate_timer->ARR = (timer_clockrate/stream->samplerate)-1;
    }
}

/**
 * @brief   gets the stream sample rate.
 * @retval  returns the sample rate in Hz.
 */
uint32_t stream_get_samplerate(stream_t* stream)
{
    return stream->samplerate;
}

/**
 * @brief   enters an stream service interface into the service register,
 *          if it is not there already and there is a space for it.
 *          up to DAC_STREAM_MAX_CONNECTIONS services may be registered.
 *          The size of the interface buffer (interface->bufferSize) is set to DAC_STREAM_BUFFER_LENGTH/2.
 *          The number of bytes in the buffer available to the service is calculated by:
 *          size-in-bytes = interface->bufferSize * sizeof(uint16_t) * number-of-DAC_STREAM_CHANNEL_COUNT
 * @param   interface is a pointer to the interface to register.
 * @param   ctx is a pointer to some application context data.
 * @param   stream is the stream to register against.
 * @param   stream_channel is the stream channel to connect the service on.
 */
void stream_connect_service(stream_connection_t* interface, void* ctx, stream_t* stream, uint8_t stream_channel)
{
    uint8_t i;
    bool registered = false;

    interface->ctx = ctx;
    interface->stream = stream;
    interface->stream_channel = stream_channel;

    // check that it is not already registered
    for(i = 0; i < stream->maxconns; i++)
    {
        // check already registerd
        if(interface == stream->connections[i])
        {
            log_debug(&stream->log, "service %s already registered", interface->name);
            registered = true;
            break;
        }
    }

    if(!registered)
    {
        for(i = 0; i < stream->maxconns; i++)
        {
            // check for an empty space
            if(!stream->connections[i])
            {
                stream->connections[i] = interface;
                log_debug(&stream->log, "service %s registered in slot %d", interface->name, i);
                registered = true;
                break;
            }
        }
    }

    if(!registered)
        log_warning(&stream->log, "no space in service register for %s", interface->name);
}

/**
 * handler for stream processing in main loop.
 */
void stream_processing_task(stream_t* stream)
{
    uint8_t connection;
    uint8_t channel;
    stream_connection_t* interface;

    while(1)
    {
        stream->buffer = NULL;
        xSemaphoreTake(stream->ready, 10000/portTICK_RATE_MS);

        if(!stream->buffer)
            continue;

        // call all registered + enabled service channel buffer read functions
        for(channel = 0; channel < stream->channels; channel++)
        {
            for(connection = 0; connection < stream->maxconns; connection++)
            {
                interface = stream->connections[connection];
                if(interface && interface->enabled && (interface->stream_channel == channel))
                    interface->process(stream->buffer + channel, stream->length/2, stream->channels, interface);
            }
        }
    }
}

void stream_connection_enable(stream_connection_t* interface, bool enable)
{
    uint16_t* buffer = interface->stream->_buffer + interface->stream_channel;
    uint8_t channels = interface->stream->channels;
    int length = interface->stream->length;
    int i;
    interface->enabled = enable;

    if(!enable)
    {
        for(i = 0; i < length; i++)
        {
            *buffer = 0;
            buffer += channels;
        }
    }
}

bool stream_connection_enabled(stream_connection_t* interface)
{
    return interface->enabled;
}

uint32_t stream_get_resolution(stream_connection_t* interface)
{
    return interface->stream->resolution;
}

uint32_t stream_get_full_scale_amplitude_mv(stream_connection_t* interface)
{
    return interface->stream->full_scale_amplitude;
}
