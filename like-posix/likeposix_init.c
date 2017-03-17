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

#if USE_FREERTOS
#if USE_PTHREADS
#include <pthread.h>
typedef void*(*threadfunc)(void*);
#else
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
typedef TaskHandle_t threadfunc;
#endif
#endif

#include "likeposix_init.h"

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

uint32_t __likeposix_crt_flags;

// also externed in startup.c
extern int main();

void LikePosix_Init()
{
	__likeposix_crt_flags = 0;

#if USE_DRIVER_SYSTEM_TIMER
    init_systime();
#endif
#if USE_LOGGER
    logger_init();
#endif
#if USE_DRIVER_USART && USE_STDIO_USART
    int usarth = (int)usart_create_polled(CONSOLE_USART, true, USART_FULLDUPLEX, USART_DEFAULT_BAUDRATE);
    usart_set_stdio_usart(usarth);
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

#if USE_FREERTOS
    __likeposix_crt_flags |= SHEDULER_ENABLED;
#if USE_PTHREADS
	pthread_t main_thread;
	pthread_attr_t main_attr;
	pthread_attr_init(&main_attr);
	pthread_attr_setstacksize(&main_attr, configMAIN_STACK_SIZE);
	pthread_attr_setdetachstate(&main_attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&main_thread, &main_attr, (threadfunc)main, NULL);
	pthread_attr_destroy(&main_attr);
#else
    xTaskCreate((threadfunc)main, "main", configMAIN_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
#endif
	vTaskStartScheduler();
#endif
}

