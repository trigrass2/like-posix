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

#ifndef STREAM_DEFS_H
#define STREAM_DEFS_H

#include <stdint.h>
#include "board_config.h"
#include "cutensils.h"

#define STREAM_SR_PERIOD_RELOAD_2K           36000
#define STREAM_SR_PERIOD_RELOAD_4K           18000
#define STREAM_SR_PERIOD_RELOAD_8K           9000
#define STREAM_SR_PERIOD_RELOAD_11K025       6531
#define STREAM_SR_PERIOD_RELOAD_16K          4500
#define STREAM_SR_PERIOD_RELOAD_22K05        3265
#define STREAM_SR_PERIOD_RELOAD_32K          2250
#define STREAM_SR_PERIOD_RELOAD_44K1         1632
#define STREAM_SR_PERIOD_RELOAD_48K          1500
#define STREAM_SR_PERIOD_RELOAD_96K          1250
#define STREAM_SR_MIN                        2000
#define STREAM_SR_MAX                        96000

#define DEFAULT_STREAM_RESOLUTION            65536

typedef struct _stream_connection_t stream_connection_t;

typedef void(*stream_callback_t)(uint16_t* buffer, uint16_t length, uint8_t channels, stream_connection_t* conn);

typedef struct {
    uint16_t* buffer;                       ///< private to stream driver, points to the current buffer phase or NULL if no data is ready.
    uint16_t* _buffer;                      ///< private to stream driver, points to the start of the full stream buffer.
    stream_connection_t** connections;      ///< private to stream driver, register of service interfaces.
    uint32_t samplerate;                    ///< private to stream driver, holds a cached value of the last set samplerate.
    logger_t log;                           ///< private to stream driver, logger.
    SemaphoreHandle_t ready;                ///< private to stream driver, semaphore used to unblock connection(s) on data ready.
    uint16_t maxconns;                      ///< the number of connections possible on this stream
    uint8_t channels;                       ///< the number of channels supported by this stream
    uint16_t length;                        ///< stream buffer length, in samples
    uint16_t full_scale_amplitude;          ///< a number that may be used to scale the input/output of a stream.
} stream_t;

typedef struct _stream_connection_t{
    stream_callback_t process;              ///< function called to do signal processing on the stream buffer.
    const char* name;                       ///< name string for this channel.
    bool enabled;                           ///< enables processing on this connection.
    void* ctx;                              ///< application context data, set by the application.
    stream_t* stream;                       ///< stream this connection is associated with.
    uint8_t stream_channel;                ///< stream channel this connection is associated with.
}stream_connection_t;

#endif // STREAM_DEFS_H
