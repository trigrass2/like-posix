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
 * This file is part of the lollyjar project, <https://github.com/drmetal/lollyjar>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

#include "services.h"

#if USE_DRIVER_SYSTEM_TIMER
#include "systime.h"
#endif
#if USE_DRIVER_LEDS
#include "leds.h"
#endif
#if USE_DRIVER_USART
#include "usart.h"
#endif
#if USE_LIKEPOSIX
#include "syscalls.h"
#endif
#if USE_LOGGER
#include <unistd.h>
#include "cutensils.h"
#endif
#if USE_MINLIBC
#include "minlibc/stdio.h"
#endif

void init_services()
{
#if USE_DRIVER_SYSTEM_TIMER
    init_systime();
#endif
#if USE_LOGGER
    logger_init();
#endif
#if USE_DRIVER_USART && USE_STDIO_USART
    usart_init(CONSOLE_USART, NULL, true, USART_FULLDUPLEX);
    set_console_usart(CONSOLE_USART);
#if USE_LOGGER
    log_add_handler(STDOUT_FILENO);
#endif
#endif
#if USE_DRIVER_LEDS
    init_leds();
#endif
#if USE_LIKEPOSIX
    init_likeposix();
#endif
#if USE_MINLIBC
    init_minlibc();
#endif
}


