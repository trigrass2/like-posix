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
 *
 * @defgroup logger Logger
 *
 * configurable logging...
 *
 *  - may be turned on/off with the macro USE_LOGGER
 *  - supports multiple handlers
 *  - glovbally filtered by level
 *
 * @file logger.c
 * @{
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "logger.h"


static log_level_t _log_level = LOG_SYSLOG;
static int handlers[MAX_LOG_HANDLERS] = {-1};

/**
 * logger that may be used by any module
 */
static logger_t _syslog = {
    .name = "root logger"
};

char* levelstr[] = {
	"\t: syslog :\t",
	"\t: edebug :\t",
	"\t: debug  :\t",
	"\t: info   :\t",
	"\t: warning:\t",
	"\t: error  :\t",
};

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
}

/**
 * adds a file number as a log handler.
 *
 * @param	file is the file number of an open file.
 */
void log_add_handler(int file)
{
	for(int i = 0; i < MAX_LOG_HANDLERS; i++)
	{
		if(handlers[i] == -1)
		{
			handlers[i] = file;
			return;
		}
	}
}

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
			handlers[i] = -1;
	}
}

/**
 * @param sets the global log level.
 * 			all log levels below 'level' will not be logged.
 */
void log_level(log_level_t level)
{
	_log_level = level;
}

/**
 * log record writer...
 */
static inline void write_log_record(logger_t* logger, log_level_t level, char* message, va_list va_args)
{
	if(level < _log_level)
		return;

	char buf[512];
	vsprintf(buf, message, va_args);

	logger = logger != NULL ? logger : &_syslog;

	for(int i = 0; i < MAX_LOG_HANDLERS; i++)
	{
		if(handlers[i] != -1)
		{
			write(handlers[i], logger->name, strlen(logger->name));
			write(handlers[i], levelstr[level], strlen(levelstr[level]));
			write(handlers[i], buf, strlen(buf));
			write(handlers[i], "\n", 1);

			if(isatty(handlers[i]) == 0)
			{
				fsync(handlers[i]);
			}
		}
	}
}

/**
 * creates a log record at the level LOG_SYSLOG
 *
 * @param	logger is a pointer to the particular logger to use,
 * 			or NULL to use the root logger.
 * @param	message is a pointer to the message string.
 */
void log_syslog(logger_t* logger, char* message, ...)
{
	va_list va_args;
	va_start(va_args, message);
	write_log_record(logger, LOG_SYSLOG, message, va_args);
	va_end(va_args);
}

/**
 * creates a log record at the level LOG_EDEBUG
 *
 * @param	logger is a pointer to the particular logger to use,
 * 			or NULL to use the root logger.
 * @param	message is a pointer to the message string.
 */
void log_edebug(logger_t* logger, char* message, ...)
{
	va_list va_args;
	va_start(va_args, message);
	write_log_record(logger, LOG_EDEBUG, message, va_args);
	va_end(va_args);
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
