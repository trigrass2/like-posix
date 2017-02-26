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

#ifndef STREAM_COMMON_H_
#define STREAM_COMMON_H_

#include "stream_defs.h"
void init_stream(stream_t* stream, const char* name, uint32_t samplerate,
                uint16_t maxconns, unsigned_stream_type_t* stream_buffer, stream_connection_t** connections,
                uint16_t buffer_length, uint8_t channel_count, uint8_t task_prio, uint16_t task_stack, uint16_t full_scale_amplitude, uint32_t resolution);
void stream_set_samplerate(stream_t* stream, TIM_TypeDef* samplerate_timer, uint32_t timer_clockrate, uint32_t samplerate);
uint32_t stream_get_samplerate(stream_t* stream);

void stream_connection_init(stream_connection_t* interface, stream_callback_t process, const char* name, void* ctx);

void stream_connect_service(stream_connection_t* interface, stream_t* stream, uint8_t stream_channel);
void stream_disconnect_service(stream_connection_t* interface, stream_t* stream);

void stream_connection_enable(stream_connection_t* interface, bool enable);
bool stream_connection_enabled(stream_connection_t* interface);

uint32_t stream_get_resolution(stream_connection_t* interface);
uint32_t stream_get_full_scale_amplitude_mv(stream_connection_t* interface);
uint32_t stream_get_length(stream_connection_t* interface);
uint32_t stream_get_channel_count(stream_connection_t* interface);

#endif /* STREAM_COMMON_H_ */
