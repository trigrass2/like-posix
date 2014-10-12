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

 /**
  * the number of log handlers that are allowed at one time
  */
#define MAX_LOG_HANDLERS	5

typedef enum {
	LOG_SYSLOG=0,
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
}logger_t;

#if USE_LOGGER
void log_init(logger_t* logger, const char* name);
void log_add_handler(int file);
void log_remove_handler(int file);
void log_level(log_level_t level);
void log_syslog(logger_t* logger, char* message, ...);
void log_edebug(logger_t* logger, char* message, ...);
void log_debug(logger_t* logger, char* message, ...);
void log_info(logger_t* logger, char* message, ...);
void log_warning(logger_t* logger, char* message, ...);
void log_error(logger_t* logger, char* message, ...);
#else
#define log_init(...) {}
#define log_add_handler(...) {}
#define log_remove_handler(...) {}
#define log_level(...) {}
#define log_syslog(...) {}
#define log_edebug(...) {}
#define log_debug(...) {}
#define log_info(...) {}
#define log_warning(...) {}
#define log_error(...) {}
#endif

#ifdef __cplusplus
 }
#endif

#endif /* LOGGER_H_ */

 /**
  * @}
  */
