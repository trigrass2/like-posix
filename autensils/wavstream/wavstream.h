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
 * This file is part of the Appleseed project, <https://github.com/drmetal/appleseed>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

#ifndef WAVSTREAM_H_
#define WAVSTREAM_H_

#include "wav.h"
#include "stream_common.h"

#ifndef WAVSTREAM_USE_MUTEX
#ifdef USE_FREERTOS
#define WAVSTREAM_USE_MUTEX USE_FREERTOS
#else
#define WAVSTREAM_USE_MUTEX 0
#endif
#endif

#if USE_FREERTOS && WAVSTREAM_USE_MUTEX
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#pragma message "building wavstream with thread safety 'on', WAVSTREAM_USE_MUTEX=1"
#else
#pragma message "building wavstream with thread safety 'off', WAVSTREAM_USE_MUTEX=0"
#endif


typedef struct {
    logger_t log;
    wav_file_t file;
    signed_stream_type_t level;
    signed_stream_type_t fsa;
    uint32_t restore_samplerate;
    uint32_t(*getsamplerate)(void);
    void(*setsamplerate)(uint32_t);
    wav_file_processing_t wavproc;
#if WAVSTREAM_USE_MUTEX
    SemaphoreHandle_t busy;
#endif
} wavstream_t;

#if WAVSTREAM_USE_MUTEX
#define init_wavstream_mutex() 						\
	do												\
	{												\
		wavstream->busy = xSemaphoreCreateMutex();		\
		assert_true(wavstream->busy);					\
	}												\
	while(0)
#define wavstream_take_mutex() (xSemaphoreTake(wavstream->busy, 1000/portTICK_RATE_MS) == pdTRUE)
#define wavstream_give_mutex() xSemaphoreGive(wavstream->busy)
#else
#define init_wavstream_mutex()
#define wavstream_take_mutex()	1
#define wavstream_give_mutex()
#endif

void wavstream_init(wavstream_t* wavstream, stream_connection_t* conn, stream_t* stream, const char* name, int8_t stream_channel, uint32_t(*getsamplerate)(void), void(*setsamplerate)(uint32_t));
void wavstream_enable(stream_connection_t* conn, const char* file);
bool wavstream_enabled(stream_connection_t* conn);
void wavstream_set_level(stream_connection_t* conn, signed_stream_type_t level);
signed_stream_type_t wavstream_get_level(stream_connection_t* conn);

#endif /* WAVSTREAM_H_ */




