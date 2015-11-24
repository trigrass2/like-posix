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

#ifndef SIGGEN_H_
#define SIGGEN_H_

#include "stream_common.h"


extern const uint16_t sine_unsigned_65536[65536];
extern const uint16_t sine_unsigned_32768[32768];
extern const uint16_t sine_unsigned_16384[16384];
extern const uint16_t sine_unsigned_8192[8192];
extern const uint16_t sine_unsigned_4096[4096];
extern const uint16_t sine_unsigned_2048[2048];
extern const uint16_t sine_unsigned_1024[1024];
extern const uint16_t sine_unsigned_512[512];

extern const int16_t sine_signed_65536[65536];
extern const int16_t sine_signed_32768[32768];
extern const int16_t sine_signed_16384[16384];
extern const int16_t sine_signed_8192[8192];
extern const int16_t sine_signed_4096[4096];
extern const int16_t sine_signed_2048[2048];
extern const int16_t sine_signed_1024[1024];
extern const int16_t sine_signed_512[512];

#define SIGGEN_DEFAULT_FREQUENCY           1000
#define SIGGEN_DEFAULT_AMPLITUDE           100

typedef struct {
    uint16_t phase_accumulator;
    uint16_t tuning_word;
    const uint16_t* lut;
    uint32_t lut_length;
    uint32_t frequency;
    uint16_t amplitude;
    logger_t log;
    int32_t fsa;
} siggen_t;

void siggen_init(siggen_t* siggen, stream_connection_t* conn, stream_t* stream, const char* name, uint8_t stream_channel, uint16_t* lut, uint32_t length);
void siggen_set_frequency(stream_connection_t* conn, uint32_t frequency);
uint32_t siggen_get_frequency(stream_connection_t* conn);
void siggen_enable(stream_connection_t* conn, bool enable);
bool siggen_enabled(stream_connection_t* conn);
void siggen_set_amplitude(stream_connection_t* conn, uint16_t amplitude);
uint16_t siggen_get_amplitude(stream_connection_t* conn);

#endif /* SIGGEN_H_ */




