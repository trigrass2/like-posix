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
 * This file is part of the cutensils project, <https://github.com/drmetal/cutensils>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

/**
 * @addtogroup logger
 *
 * @file logger.h
 * @{
 */
#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdbool.h>
#include <stdarg.h>

#ifdef __cplusplus
 extern "C" {
#endif

#ifndef USE_LOGGER
 /**
  * enable logging
  */
#define USE_LOGGER	0
#endif

#ifndef USE_UDP_LOGGER
 /**
  * enable udp logging
  */
#define USE_UDP_LOGGER  0
#endif

#ifndef MAX_LOG_HANDLERS
 /**
  * the number of log handlers that are allowed at one time
  */
#define MAX_LOG_HANDLERS	5
#endif

#ifndef USE_MUTEX
 /**
  * puts a mutex around writing a log line, when set to 1
  */
#define USE_MUTEX   USE_FREERTOS
#endif

#ifndef LOG_BUFFER_SIZE
 /**
  * this is the size in bytes of the buffer used to format the log message.
  */
#define LOG_BUFFER_SIZE   512
#endif

#ifndef LOG_TIMESTAMP_BUFFER_SIZE
 /**
  * this is the size in bytes of the buffer used to format the log message.
  */
#define LOG_TIMESTAMP_BUFFER_SIZE   32
#endif

 /**
  * used to calculate tabs needed to pad name field.
  */
#define TAB_WIDTH       8

typedef enum {
	LOG_LEVEL_CHECK = -1,
	LOG_SYSLOG,
	LOG_EDEBUG,
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARNING,
	LOG_ERROR,
	LOG_DISABLED
} log_level_t;

 /**
  * logger definition
  */
typedef struct {
	const char* name;
    char pad;
}logger_t;

#if USE_LOGGER
void logger_init();
void log_init(logger_t* logger, const char* name);
int add_udp_log_handler(const char* host, int port);
void log_add_handler(int file);
void log_remove_handler(int file);

log_level_t log_level(log_level_t level);
void log_timestamp(bool ts);
void log_coloured(bool c);

void log_syslog(logger_t* logger, char* message, ...);
void log_edebug(logger_t* logger, char* message, ...);
void log_debug(logger_t* logger, char* message, ...);
void log_info(logger_t* logger, char* message, ...);
void log_warning(logger_t* logger, char* message, ...);
void log_error(logger_t* logger, char* message, ...);

#if USE_MUTEX
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#define	LOGGER_TIMEOUT		1000
typedef  SemaphoreHandle_t logger_mutex_t;
#define create_mutex()		xSemaphoreCreateMutex()
#define take_mutex(mutex)		xSemaphoreTake(mutex, LOGGER_TIMEOUT/portTICK_RATE_MS)
#define give_mutex(mutex)		xSemaphoreGive(mutex)
#else
#define create_mutex()
#define take_mutex(mutex)
#define give_mutex(mutex)
#endif

#else
#define logger_init()
#define log_init(l, n) {(void)l;(void)n;}
#define log_add_handler(i, ...) {(void)i;}
#define add_udp_log_handler(host, port){(void)host;(void)port;}
#define log_remove_handler(i, ...) {(void)i;}
#define log_level(l)        0
#define log_timestamp(ts)    {(void)ts;}
#define log_coloured(c)    {(void)c;}
#define log_syslog(l, ...) {(void)l;}
#define log_edebug(l, ...) {(void)l;}
#define log_debug(l, ...) {(void)l;}
#define log_info(l, ...) {(void)l;}
#define log_warning(l, ...) {(void)l;}
#define log_error(l, ...) {(void)l;}

#define create_mutex()
#define take_mutex(mutex)
#define give_mutex(mutex)
#endif

#ifdef __cplusplus
 }
#endif

#endif /* LOGGER_H_ */

 /**
  * @}
  */
