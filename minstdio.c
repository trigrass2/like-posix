/**
 * @defgroup minstdio minimal stdio
 *
 * tiny versions of some stdio functions. slower and less complete than the standard implementations, but can compile for a much smaller flash usage.
 *
 * To use:
 *
 *     the function phy_putc must be defined somewhere in your project.
 *     just #include <stdio.h> like normal wherever you want to use it.
\code
// phy_putc, function to write a character to a serial port, used by minstdio
void phy_putc(char* c)
{
// code to wait for serial ready
// code to write 'c' to serial port
}
\endcode
 * this file is part of the minstdlibs project.
 *
 * https://github.com/drmetal/minstdlibs
 *
 * copyright 2015 Mike Stuart
 *
 * @file minstdio.c
 * @{
 */

#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
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

typedef char*(*putx_t)(char**, const char*);

static inline char* phyputc(char** dst, const char*src)
{
	if(*src)
	{
		phy_putc(*src);
		(*dst)++;
	}
	return *dst;
}

static inline char* phyputs(char** dst, const char*src)
{
	while(*src++)
	{
		phy_putc(*src);
		(*dst)++;
	}
	return *dst;
}

static inline char* memputc(char** dst, const char*src)
{
	**dst = *src;
	(*dst)++;
	return *dst;
}

static inline char* memputs(char** dst, const char*src)
{
	while(*src++)
	{
		**dst = *src;
		(*dst)++;
	}
	return *dst;
}

static inline int strfmt(putx_t _put_char, putx_t _put_str, char** dst, const char * fmt, va_list argp)
{
	double d;
	int ret = -1;
	char* start = *dst;
	void* v;
	unsigned int u;
	int i;
	const char* s;
	char c;
	char intbuf[32];
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
				switch(*fmt)
				{
					case '-':
						flags |= MINUS_FLAG;
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
					case '0':
						flags |= ZERO_FLAG;
						fmt++;
					break;
					case ' ':
						flags |= SPACE_FLAG;
						fmt++;
					break;
				}

				switch(*fmt)
				{
					case 'c':
						c = (char)va_arg(argp, int);
						_put_char(dst, &c);
					break;

					case 's':
						s = (char*)va_arg(argp, char*);
						if(!s)
							s = (const char*)"(null)";
						_put_str(dst, s);
					break;

					case 'i':
					case 'd':
						i = (int)va_arg(argp, int);
						if(i >= 0)
						{
							if(flags&PLUS_FLAG)
							{
								c = '+';
								_put_char(dst, &c);
							}
							else if(flags&SPACE_FLAG)
							{
								c = ' ';
								_put_char(dst, &c);
							}
						}
						_put_str(dst, itoa(i, intbuf, 10));
					break;

					case 'u':
						u = (unsigned int)va_arg(argp, unsigned int);
						if(flags&PLUS_FLAG)
						{
							c = '+';
							_put_char(dst, &c);
						}
						else if(flags&SPACE_FLAG)
						{
							c = ' ';
							_put_char(dst, &c);
						}

						_put_str(dst, ditoa((int64_t)u, intbuf, 10));
					break;

					case 'x':
						u = (unsigned int)va_arg(argp, unsigned int);
						if(flags&HASH_FLAG)
							_put_str(dst, (const char*)"0x");
						_put_str(dst, ditoa((int64_t)u, intbuf, 16));
					break;

					case 'X':
						u = (unsigned int)va_arg(argp, unsigned int);
						if(flags&HASH_FLAG)
							_put_str(dst, (const char*)"0x");
						strtoupper(ditoa((int64_t)u, intbuf, 16));
						_put_str(dst, intbuf);
					break;

					case 'p':
						v = (void*)va_arg(argp, void*);
						_put_str(dst, (const char*)"0x");
						ditoa((int64_t)v, intbuf, 16);
						_put_str(dst, intbuf);
					break;

					case 'f':
						d = (double)va_arg(argp, double);
						_put_str(dst, ftoa(intbuf, d));
					break;

					case '%':
						c = '%';
						_put_char(dst, &c);
					break;
				}
				break;

				// all other charcters
				default:
					_put_char(dst, fmt);
				break;
			}
		}

		// null terminate
		c = '\0';
		_put_char(dst, &c);

		ret = (int)(*dst - start);
	}

	return ret;
}

int vsprintf(char* dst, const char * fmt, va_list argp)
{
	return strfmt(memputc, memputs, &dst, fmt, argp);
}

int sprintf(char* dst, const char* fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);
	int ret = strfmt(memputc, memputs, &dst, fmt, argp);
	va_end(argp);

	return ret;
}

int vprintf(const char * fmt, va_list argp)
{
	char* dst = NULL;
	return strfmt(phyputc, phyputs, &dst, fmt, argp);
}

int printf(const char * fmt, ...)
{
	char* dst = NULL;
	va_list argp;
	va_start(argp, fmt);
	int ret = strfmt(phyputc, phyputs, &dst, fmt, argp);
	va_end(argp);
	return ret;
}

/**
 * @}
 */
