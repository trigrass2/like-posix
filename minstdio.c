/**
 * @defgroup minstdio minimal stdio
 *
 * tiny versions of some stdio functions. slower and less complete than the standard implementations, but can compile for a much smaller flash usage.
 *
 * To use:
 *
 *     the function _write must be defined somewhere in your project. this is typically defined in syscalls.c...
 *     just #include <stdio.h> like normal wherever you want to use it.
\code
// _write, function to write a characters to a device or file
int _write(int file, char *buffer, unsigned int count)
{
	int n = EOF;

	if(file == STDOUT_FILENO || file == STDERR_FILENO)
	{
		for(n = 0; n < (int)count; n++) {
			phy_putc(*buffer++); // some code here to write a serial device, or memory, or whatever
		}
	}
	return n;
}
\endcode
 *
 * supported string formatting:
 *
 * %%		print % symbol
 * %c		print character
 * %s		print null terminated string
 * %i		print signed int
 * %d		print signed int
 * %u		print unsigned int
 * %x		print unsigned int, in hexadecimal format, lower case
 * %X		print unsigned int, in hexadecimal format, upper case
 * %p		print pointer address, in hexadecimal format, lower case, with '0x' presceeding
 * %f		print floating point value
 * 0, space	padding with space and 0 characters are supported by i, u, d, x, X, p
 * 			Eg printf("padded integer: %06d", 123);
 * 				-> padded integer: 000123
 * 			Eg printf("padded integer: % 6d\n", 123);
 * 			   printf("padded integer: % 6d", 4567);
 * 				-> padded integer:    123
 * 				-> padded integer:   4567
 * #     	a '0x' preceeds hexadecimal formatted numbers when # is inserted. supported by x, X:
 * 			Eg printf("modified hex: %#x", 1234);
 * 			   printf("modified hex: %#X", 1234);
 * 				-> modified hex: 0x4d2
 * 				-> modified hex: 0x4D2
 * l,h		ignored.
 * 			Eg printf("ignore lh: %llu %lld %lu %ld %hu %hd", 1234, 1234, 1234, 1234, 1234, 1234);
 *				-> ignore lh: 1234 1234 1234 1234 1234 1234
 * +		a + preceeds numeric formatted numbers when + is inserted. supported by i, u, d, f:
 *
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
#include <unistd.h>
#include <ctype.h>
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

typedef char*(*putx_t)(char**, const char*);

extern int _write(int file, char *buffer, unsigned int count);

static inline char* phyputc(char** dst, const char*src)
{
	if(*src)
	{
		_write(STDOUT_FILENO, (char*)src, 1);
		(*dst)++;
	}
	return *dst;
}

static inline char* phyputs(char** dst, const char*src)
{
	int l = strlen(src);
	if(l > 0)
	{
		_write(STDOUT_FILENO, (char*)src, l);
		*dst += l;
	}
	return *dst;
}

static inline char* memputc(char** dst, const char*src)
{
	if(*src)
	{
		**dst = *src;
		(*dst)++;
	}
	return *dst;
}

static inline char* memputs(char** dst, const char*src)
{
	while(*src)
	{
		**dst = *src;
		(*dst)++;
		src++;
	}
	return *dst;
}

static inline void plusflag(putx_t _put_char, unsigned int flags, char** dst)
{
	if(flags&PLUS_FLAG)
	{
		char c = '+';
		_put_char(dst, &c);
	}
}

static inline void hashflag(putx_t _put_str, unsigned int flags, char** dst)
{
	if(flags&HASH_FLAG)
		_put_str(dst, (const char*)"0x");
}

static inline int strfmt(putx_t _put_char, putx_t _put_str, char** dst, const char * fmt, va_list argp)
{
	double d;
	int ret = -1;
	char* start = *dst;
	void* v;
	unsigned int u;
	int i;
	int padding;
	char padchar;
	const char* s;
	char c;
	char intbuf[128];
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
					if(padchar)
					{
						padding = 0;
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
					}

					// handle type
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
								plusflag(_put_char, flags, dst);

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
									_put_str(dst, padbuf);
								}
							}
							_put_str(dst, intbuf);
						break;

						case 'u':
							u = (unsigned int)va_arg(argp, unsigned int);
							plusflag(_put_char, flags, dst);

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
									_put_str(dst, padbuf);
								}
							}

							_put_str(dst, intbuf);
						break;

						case 'x':
							u = (unsigned int)va_arg(argp, unsigned int);
							hashflag(_put_str, flags, dst);

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
									_put_str(dst, padbuf);
								}
							}

							_put_str(dst, intbuf);
						break;

						case 'X':
							u = (unsigned int)va_arg(argp, unsigned int);
							hashflag(_put_str, flags, dst);

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
									_put_str(dst, padbuf);
								}
							}

							_put_str(dst, intbuf);

						break;

						case 'p':
							v = (void*)va_arg(argp, void*);
							_put_str(dst, (const char*)"0x");

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
									_put_str(dst, padbuf);
								}
							}
							_put_str(dst, intbuf);
						break;

						case 'f':
							d = (double)va_arg(argp, double);
							plusflag(_put_char, flags, dst);
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
