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
 * This file is part of the minstdlibs project, <https://github.com/drmetal/minstdlibs>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

/**
 * @defgroup minstdio minimal stdio
 *
 * tiny versions of some stdio functions.
 *
 * @file minstdio.c
 * @{
 */

#include <stdarg.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h> // open(), close() etc
#include <ctype.h> // isdigit() etc
#include <fcntl.h>
#include "minstdio.h"
#include "minstdlib.h"
#include "minstring.h"
#include "strutils.h"

#define PLUS_FLAG 1
#define MINUS_FLAG 2
#define SPACE_FLAG 4
#define HASH_FLAG 8
#define ZERO_FLAG 16
#define LONG_FLAG 32
#define SHORT_FLAG 64
#define FLOAT_FLAG 128

typedef char*(*putx_t)(int, char**, const char*);

extern int _write(int file, char *buffer, unsigned int count);
extern int _read(int file, char *buffer, unsigned int count);
extern int _lseek(int file, int offset, int whence);
extern long int _ftell(int fd);

static inline char* phyputc(int fd, char** dst, const char*src)
{
	if(*src)
	{
		_write(fd, (char*)src, 1);
		(*dst)++;
	}
	return *dst;
}

static inline char* phyputs(int fd, char** dst, const char*src)
{
	int l = strlen(src);
	if(l > 0)
	{
		_write(fd, (char*)src, l);
		*dst += l;
	}
	return *dst;
}

static inline char* memputc(int fd, char** dst, const char*src)
{
	(void)fd;
	**dst = *src;
	(*dst)++;
	return *dst;
}

static inline char* memputs(int fd, char** dst, const char*src)
{
	(void)fd;
	while(*src)
	{
		**dst = *src;
		(*dst)++;
		src++;
	}
	return *dst;
}

static inline void plusflag(int fd, putx_t _put_char, unsigned int flags, char** dst)
{
	if(flags&PLUS_FLAG)
	{
		char c = '+';
		_put_char(fd, dst, &c);
	}
}

static inline void hashflag(int fd, putx_t _put_str, unsigned int flags, char** dst)
{
	if(flags&HASH_FLAG)
		_put_str(fd, dst, (const char*)"0x");
}

static inline int strfmt(int fd, putx_t _put_char, putx_t _put_str, char** dst, const char * fmt, va_list argp)
{
	float d;
    float prescision = DEFAULT_FTOA_PRECISION;
	int ret = -1;
	char* start = *dst;
	void* v;
	unsigned int u;
	int i;
	int padding;
	char padchar;
	const char* s;
	char c;
	char intbuf[28];
	unsigned int flags;

	if(fmt)
	{
		for(;*fmt;fmt++)
		{
			flags = 0;

			// print fmt chars
			switch(*fmt)
			{
				// print va_args
				case '%':
					fmt++;
					// handle sign and #
					switch(*fmt)
					{
						case '-':
							// ignore left justification for now
							// flags |= MINUS_FLAG;
							fmt++;
						break;
						case '+':
							flags |= PLUS_FLAG;
							fmt++;
						break;
						case '#':
							flags |= HASH_FLAG;
							fmt++;
						break;
                        case '.':
                            flags |= FLOAT_FLAG;
                            fmt++;
                        break;
					}

					// handle padding
					padchar = 0;
					switch(*fmt)
					{
						case ' ':
							padchar = ' ';
							flags |= SPACE_FLAG;
							fmt++;
						break;
						case '0':
							padchar = '0';
							fmt++;
							flags |= ZERO_FLAG;
						break;
						default:
							if(isdigit((int)*fmt))
							{
								padchar = ' ';
								flags |= SPACE_FLAG;
							}
						break;
					}

					// determine overall padded length
					// use intbuf as working area
					padding = 0;
					if(padchar)
					{
						while(isdigit((int)*fmt))
						{
							intbuf[padding] = *fmt;
							padding++;
							fmt++;
						}
						intbuf[padding] = '\0';
						padding = atoi(intbuf);
					}

					// handle long/short
					switch(*fmt)
					{
						case 'h':
						case 'l':
							// flags |= LONG_FLAG;
							// flags |= SHORT_FLAG;
							// ignore length sub specifiers for now
							c = *fmt;
							fmt++;
							while(*fmt == c)
								fmt++;
						break;
						case 'f':
						    prescision = 1 / pow(10, padding);
						break;
					}

					// handle type
					switch(*fmt)
					{
						case 'c':
							c = (char)va_arg(argp, int);
							_put_char(fd, dst, &c);
						break;

						case 's':
							s = (char*)va_arg(argp, char*);
							if(!s)
								s = (const char*)"(null)";
							_put_str(fd, dst, s);
						break;

						case 'i':
						case 'd':
							i = (int)va_arg(argp, int);
							if(i >= 0)
								plusflag(fd, _put_char, flags, dst);

							itoa(i, intbuf, 10);
							if((flags&ZERO_FLAG) || (flags&SPACE_FLAG))
							{
								padding -= strlen(intbuf);
								if(padding > 0)
								{
									char padbuf[padding+1];
									padbuf[padding] = '\0';
									while(padding)
									{
										padding--;
										padbuf[padding] = padchar;
									}
									_put_str(fd, dst, padbuf);
								}
							}
							_put_str(fd, dst, intbuf);
						break;

						case 'u':
							u = (unsigned int)va_arg(argp, unsigned int);
							plusflag(fd, _put_char, flags, dst);

							ditoa((int64_t)u, intbuf, 10);

							if((flags&ZERO_FLAG) || (flags&SPACE_FLAG))
							{
								padding -= strlen(intbuf);
								if(padding > 0)
								{
									char padbuf[padding+1];
									padbuf[padding] = '\0';
									while(padding)
									{
										padding--;
										padbuf[padding] = padchar;
									}
									_put_str(fd, dst, padbuf);
								}
							}

							_put_str(fd, dst, intbuf);
						break;

						case 'x':
							u = (unsigned int)va_arg(argp, unsigned int);
							hashflag(fd, _put_str, flags, dst);

							ditoa((int64_t)u, intbuf, 16);

							if((flags&ZERO_FLAG) || (flags&SPACE_FLAG))
							{
								padding -= strlen(intbuf);
								if(padding > 0)
								{
									char padbuf[padding+1];
									padbuf[padding] = '\0';
									while(padding)
									{
										padding--;
										padbuf[padding] = padchar;
									}
									_put_str(fd, dst, padbuf);
								}
							}

							_put_str(fd, dst, intbuf);
						break;

						case 'X':
							u = (unsigned int)va_arg(argp, unsigned int);
							hashflag(fd, _put_str, flags, dst);

							ditoa((int64_t)u, intbuf, 16);
							// TODO this causes liker error in the test cases!! fix that
	//						strtoupper(intbuf);

							i = 0;
							while(intbuf[i])
							{
								intbuf[i] = toupper((int)intbuf[i]);
								i++;
							}

							if((flags&ZERO_FLAG) || (flags&SPACE_FLAG))
							{
								padding -= strlen(intbuf);
								if(padding > 0)
								{
									char padbuf[padding+1];
									padbuf[padding] = '\0';
									while(padding)
									{
										padding--;
										padbuf[padding] = padchar;
									}
									_put_str(fd, dst, padbuf);
								}
							}

							_put_str(fd, dst, intbuf);

						break;

						case 'p':
							v = (void*)va_arg(argp, void*);
							_put_str(fd, dst, (const char*)"0x");

							ditoa((intptr_t)v, intbuf, 16);
							if((flags&ZERO_FLAG) || (flags&SPACE_FLAG))
							{
								padding -= strlen(intbuf);
								if(padding > 0)
								{
									char padbuf[padding+1];
									padbuf[padding] = '\0';
									while(padding)
									{
										padding--;
										padbuf[padding] = padchar;
									}
									_put_str(fd, dst, padbuf);
								}
							}
							_put_str(fd, dst, intbuf);
						break;

						case 'f':
							d = (float)va_arg(argp, double);
							plusflag(fd, _put_char, flags, dst);
							_put_str(fd, dst, ftoa(intbuf, d, prescision));
						break;

						case '%':
							c = '%';
							_put_char(fd, dst, &c);
						break;
					}
				break;

				// all other charcters
				default:
					_put_char(fd, dst, fmt);
				break;
			}
		}

		// null terminate
		c = '\0';
		_put_char(fd, dst, &c);

		ret = (int)(*dst - start);
	}

	return ret;
}

int vsprintf(char* dst, const char * fmt, va_list argp)
{
	return strfmt(-1, memputc, memputs, &dst, fmt, argp) -1;
}

int sprintf(char* dst, const char* fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);
	int ret = strfmt(-1, memputc, memputs, &dst, fmt, argp) -1;
	va_end(argp);

	return ret;
}

int vprintf(const char * fmt, va_list argp)
{
	char* dst = NULL;
	return strfmt(STDOUT_FILENO, phyputc, phyputs, &dst, fmt, argp);
}

int printf(const char * fmt, ...)
{
	char* dst = NULL;
	va_list argp;
	va_start(argp, fmt);
	int ret = strfmt(STDOUT_FILENO, phyputc, phyputs, &dst, fmt, argp);
	va_end(argp);
	return ret;
}

FILE* fopen(const char * filename, const char * mode)
{
	int flags = 0;
	char m = mode[0];
	char update = ((mode[1] == '+') || (mode[2] == '+'));

	if(m == 'r')
		flags = update ? O_RDWR : O_RDONLY;
	else if(m == 'w')
		flags = O_CREAT | O_TRUNC | (update ? O_RDWR : O_WRONLY);
	else if(m == 'a')
		flags = O_CREAT | O_APPEND | (update ? O_RDWR : O_WRONLY);

	intptr_t fd = open(filename, flags);

	return fd != EOF ? (FILE*)fd : NULL;
}

FILE* fdopen(int fd, const char *mode)
{
    (void)mode;
    if(fd != -1)
        return (FILE*)fd;
    return NULL;
}

int fclose(FILE* stream)
{
	return close((intptr_t)stream);
}

int fprintf(FILE* stream, const char * fmt, ...)
{
	char* dst = NULL;
	va_list argp;
	va_start(argp, fmt);
	int ret = strfmt((intptr_t)stream, phyputc, phyputs, &dst, fmt, argp);
	va_end(argp);
	return ret;
}

int fputc(int character, FILE* stream)
{
	return _write((intptr_t)stream, (char*)&character, 1);;
}

int fputs(const char* str, FILE* stream)
{
	return _write((intptr_t)stream, (char*)str, strlen(str));;
}

int fgetc(FILE* stream)
{
	int c = EOF;
	_read((intptr_t)stream, (char*)&c, 1);
	return c;
}

char* fgets(char* str, int num, FILE* stream)
{
	char* sptr = str;
	long int pos = _ftell((intptr_t)stream);
	int ret = _read((intptr_t)stream, str, num-1);

	// ensure the string is terminated somewhere...
	if(ret > 0)
	{
		sptr[ret] = '\0';

		while((*sptr != '\n') && (*sptr != '\0'))
			sptr++;

		if(*sptr == '\n')
		{
			sptr++;
			*sptr = '\0';
		}
		// set file pointer to the end of the read line
		_lseek((intptr_t)stream, pos + (sptr - str), SEEK_SET);
	}

	return (sptr - str) > 0 ? str : NULL;
}

long int ftell(FILE* stream)
{
	return _ftell((intptr_t)stream);
}

int fseek(FILE * stream, long int offset, int origin)
{
	return _lseek((intptr_t)stream, offset, origin);
}

size_t fwrite(const void *data, size_t size, size_t count, FILE *stream)
{
	return _write((intptr_t)stream, (char*)data, size*count);
}

size_t fread(void *data, size_t size, size_t count, FILE *stream)
{
	return _read((intptr_t)stream, (char*)data, size*count);
}

int fflush(FILE* stream)
{
    return fsync((intptr_t)stream);
}




/**
 * @}
 */
