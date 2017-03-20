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

/**
 *
 * @defgroup logger Logger
 *
 * configurable logging...
 *
 *  - may be turned on/off with the macro USE_LOGGER
 *  - supports multiple handlers
 *  - globally filtered by level
 *
 * @file logger.c
 * @{
 */
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "logger.h"

#if USE_UDP_LOGGER
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include "sock_utils.h"
#endif

static log_level_t _log_level = LOG_DEBUG;
static bool _log_coloured = true;
static bool _log_timestamp = true;
static int handlers[MAX_LOG_HANDLERS];
#if USE_MUTEX
static logger_mutex_t _logger_write_mutex = NULL;
#endif
#if USE_UDP_LOGGER
static struct sockaddr udp_handlers[MAX_LOG_HANDLERS];
#define is_udp_handler(i)   ((*(struct sockaddr_in*)(udp_handlers+i)).sin_family == AF_INET)
#else
#define is_udp_handler(i)       0
#endif

/**
 * the log buffer is static, not stacked, in order to eliminate blowing task stack sizes out.
 */
static char log_buf[LOG_BUFFER_SIZE];
#if USE_LOGGER_TIMESTAMP
static char ts_buf[LOG_TIMESTAMP_BUFFER_SIZE];
#endif
struct tm* ts_lt;
struct timeval ts_tv;

/**
 * logger that may be used by any module
 */
static logger_t _syslog;

static const char* levelstr[] = {
	"debug\t",
	"info\t",
	"warning\t",
	"error\t",
};

static const char* colourstart[] = {
    "\x1b[1;34m",
    "\x1b[32m",
    "\x1b[33m",
    "\x1b[31m"
};

static const char colourstop[] = "\x1b[0m";

static const char* tabs = "\t\t\t\t\t";
#define PAD_TO          (char)((sizeof(tabs)-2)*TAB_WIDTH)

/**
 * initializes the global logger.
 */
void logger_init()
{
    for(int i = 0; i < MAX_LOG_HANDLERS; i++)
    {
        handlers[i] = -1;
#if USE_UDP_LOGGER
        memset(&udp_handlers[i], 0, sizeof(udp_handlers[i]));
#endif
    }

    log_init(&_syslog, "root logger");
}

/**
 * initialise a logger
 *
 * @param	logger is the logger to init.
 * @param	name is the name of the logger.
 * @retval	returns true if successful, false otherwise.
 */
void log_init(logger_t* logger, const char* name)
{
	logger->name = name;
	logger->pad = 1;

	int pad = strlen(logger->name);
	if(pad < PAD_TO)
	{
	    pad = PAD_TO - pad;
	    logger->pad += (pad / TAB_WIDTH) + (pad % TAB_WIDTH ? 1 : 0);
	}
}

/**
 * adds a file number as a log handler.
 *
 * @param	file is the file number of an open file.
 */
void log_add_handler(int file)
{
#if USE_MUTEX
	// create mutex once, at the time the first handler is added.
	if(_logger_write_mutex == NULL)
		_logger_write_mutex = create_mutex();
#endif
	for(int i = 0; i < MAX_LOG_HANDLERS; i++)
	{
		if(handlers[i] == -1)
		{
			handlers[i] = file;
			return;
		}
	}
}

#if USE_UDP_LOGGER

/**
 * adds a udp socket log handler.
 *
 * Never call this function if the network interface has not been initialized!
 *
 * @param   host is the remote host to send to.
 * @param   port is UDP port to send on.
 * @retval  returns the socket file descriptor, or -1 on error.
 */
int add_udp_log_handler(const char* host, int port)
{
#if USE_MUTEX
    // create mutex once, at the time the first handler is added.
    if(_logger_write_mutex == NULL)
        _logger_write_mutex = create_mutex();
#endif
    for(int i = 0; i < MAX_LOG_HANDLERS; i++)
    {
        if(handlers[i] == -1)
        {
            handlers[i] = sock_connect(host, port, SOCK_DGRAM, &udp_handlers[i]);
            return handlers[i];
        }
    }
    return -1;
}
#endif

/**
 * removes a file number as a log handler.
 *
 * @param	file is the file number of a file.
 */
void log_remove_handler(int file)
{
	for(int i = 0; i < MAX_LOG_HANDLERS; i++)
	{
		if(handlers[i] == file)
		{
			handlers[i] = -1;
#if USE_UDP_LOGGER
			memset(&udp_handlers[i], 0, sizeof(udp_handlers[i]));
#endif
        }
	}
}

/**
 * @param sets the global log level.
 * 			all log levels below 'level' will not be logged.
 * @retval returns the current log level.
 */
log_level_t log_level(log_level_t level)
{
	if(level <= LOG_DISABLED && level >= LOG_DEBUG)
		_log_level = level;
	return _log_level;
}

/**
 * @param enables the global log timestamp.
 * @retval returns the current log level.
 */
void log_timestamp(bool ts)
{
    _log_timestamp = ts;
}

/**
 * @param enables global log colouring.
 * @retval returns the current log level.
 */
void log_coloured(bool c)
{
    _log_coloured = c;
}

/**
 * log record writer...
 */
static inline void write_log_record(logger_t* logger, log_level_t level, char* message, va_list va_args)
{
    int length;
#if USE_LOGGER_TIMESTAMP
    int tslength = 0;
    char* end;
#endif
	if(level < _log_level)
		return;
#if USE_MUTEX
	if(_logger_write_mutex == NULL)
		return;
#endif

	logger = logger != NULL ? logger : &_syslog;

	take_mutex(_logger_write_mutex);

    length = vsprintf(log_buf, message, va_args);

#if USE_LOGGER_TIMESTAMP
    if(_log_timestamp)
    {
        if(gettimeofday(&ts_tv, NULL) == 0)
        {
            ts_lt = localtime(&ts_tv.tv_sec);
            tslength = strftime(ts_buf, sizeof(ts_buf), "%Y-%m-%d %H:%M:%S", ts_lt);
            end = ts_buf + tslength;
            tslength += sprintf(end, ".%03d\t", ts_tv.tv_usec/1000);
        }
    }
#endif

	for(int i = 0; i < MAX_LOG_HANDLERS; i++)
	{
		if(handlers[i] != -1)
		{
		    if(!is_udp_handler(i))
		    {
#if USE_LOGGER_TIMESTAMP
            if(_log_timestamp)
                write(handlers[i], ts_buf, tslength);
#endif
                write(handlers[i], logger->name, strlen(logger->name));
                if(logger->pad > 0)
                    write(handlers[i], tabs, logger->pad);
                write(handlers[i], levelstr[level], strlen(levelstr[level]));
                if(_log_coloured)
                    write(handlers[i], colourstart[level], strlen(colourstart[level]));
                write(handlers[i], log_buf, length);
                if(_log_coloured)
                    write(handlers[i], colourstop, sizeof(colourstop)-1);
                write(handlers[i], "\n", 1);

                if(isatty(handlers[i]) == 0)
                    fsync(handlers[i]);
		    }
#if USE_UDP_LOGGER
		    else
		    {
#if USE_LOGGER_TIMESTAMP
                if(_log_timestamp)
                    sendto(handlers[i], ts_buf, tslength, 0, &udp_handlers[i], sizeof(struct sockaddr));
#endif
		        sendto(handlers[i], logger->name, strlen(logger->name), 0, &udp_handlers[i], sizeof(struct sockaddr));
                if(logger->pad > 0)
                    sendto(handlers[i], tabs, logger->pad, 0, &udp_handlers[i], sizeof(struct sockaddr));
                sendto(handlers[i], levelstr[level], strlen(levelstr[level]), 0, &udp_handlers[i], sizeof(struct sockaddr));
                if(_log_coloured)
                    sendto(handlers[i], colourstart[level], strlen(colourstart[level]), 0, &udp_handlers[i], sizeof(struct sockaddr));
                sendto(handlers[i], log_buf, length, 0, &udp_handlers[i], sizeof(struct sockaddr));
                if(_log_coloured)
                    sendto(handlers[i], colourstop, sizeof(colourstop)-1, 0, &udp_handlers[i], sizeof(struct sockaddr));
                sendto(handlers[i], "\n", 1, 0, &udp_handlers[i], sizeof(struct sockaddr));
		    }
#endif
		}
	}

	give_mutex(_logger_write_mutex);
}

/**
 * creates a log record at the level LOG_DEBUG
 *
 * @param	logger is a pointer to the particular logger to use,
 * 			or NULL to use the root logger.
 * @param	message is a pointer to the message string.
 */
void log_debug(logger_t* logger, char* message, ...)
{
	va_list va_args;
	va_start(va_args, message);
	write_log_record(logger, LOG_DEBUG, message, va_args);
	va_end(va_args);
}

/**
 * creates a log record at the level LOG_INFO
 *
 * @param	logger is a pointer to the particular logger to use,
 * 			or NULL to use the root logger.
 * @param	message is a pointer to the message string.
 */
void log_info(logger_t* logger, char* message, ...)
{
	va_list va_args;
	va_start(va_args, message);
	write_log_record(logger, LOG_INFO, message, va_args);
	va_end(va_args);
}

/**
 * creates a log record at the level LOG_WARNING
 *
 * @param	logger is a pointer to the particular logger to use,
 * 			or NULL to use the root logger.
 * @param	message is a pointer to the message string.
 */
void log_warning(logger_t* logger, char* message, ...)
{
	va_list va_args;
	va_start(va_args, message);
	write_log_record(logger, LOG_WARNING, message, va_args);
	va_end(va_args);
}

/**
 * creates a log record at the level LOG_ERROR
 *
 * @param	logger is a pointer to the particular logger to use,
 * 			or NULL to use the root logger.
 * @param	message is a pointer to the message string.
 */
void log_error(logger_t* logger, char* message, ...)
{
	va_list va_args;
	va_start(va_args, message);
	write_log_record(logger, LOG_ERROR, message, va_args);
	va_end(va_args);
}

/**
 * @}
 */
