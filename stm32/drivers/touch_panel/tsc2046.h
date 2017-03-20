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
 * based heavily on the code by Zizzle at: https://github.com/Zizzle/stm32_freertos_example/tree/master/drivers
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */
#include <stdbool.h>
#include "tsc2046_config.h"
#include "spi.h"
#include "board_config.h"

#ifndef TSC2046_H_
#define TSC2046_H_

#define TSC2046_POLLED_SPI 0 // set to 1 for polled SPI, 0 for interrupt driven SPI
typedef SPI_HANDLE_t TSC2046_t;

/**
 * http://e2e.ti.com/support/other_analog/touch/f/750/t/177249.aspx
 * ignore busy signal, is redundant and not documented in:
 * http://www.ti.com/lit/ds/symlink/tsc2046.pdf
 */

TSC2046_t tsc2046_init();
void tsc2046_read(TSC2046_t tsc2046, int16_t* x, int16_t* y);
bool tsc2046_ready();

#endif // TSC2046_H_
