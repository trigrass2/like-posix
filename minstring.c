/**
 * @defgroup minstring minimal string
 *
 * tiny versions of some string functions. slower and less complete than the standard implementations, but can compile for a much smaller flash usage.
 *
 * this file is part of the minstdlibs project.
 *
 * https://github.com/drmetal/minstdlibs
 *
 * copyright 2015 Mike Stuart
 *
 * @file minstring.c
 * @{
 */

#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include "strutils.h"
#include "minstring.h"

size_t strlen(const char* str)
{
	size_t len = 0;
	while(*str)
	{
		len++;
		str++;
	}
	return len;
}

int strcmp(const char* str1, const char* str2)
{
	int res = *str1-*str2;

	while(*str1 && *str2 && !res)
	{
		str1++;
		str2++;
		res = *str1-*str2;
	}

	return res;
}

int strncmp(const char* str1, const char* str2, size_t num)
{
	int res = 0;
	do
	{
		res = *str1-*str2;
		num--;
	}while(*str1++ && *str2++ && !res && num);

	return res;
}

char* strcat(char* dst, const char* src)
{
	char* d = dst;
	while(*d)
		d++;
	while(*src)
		*d++ = *src++;
	*d = 0;
	return dst;
}

char* strcpy(char* dst, const char* src)
{
	char* d = dst;
	while(*src)
		*d++ = *src++;
	*d = 0;
	return dst;
}

char* strncpy(char* dst, const char* src, size_t len)
{
	char* d = dst;
	while(*src && len--)
		*d++ = *src++;
	*d = 0;
	return dst;
}

void* memcpy(void* dst, const void* src, size_t len)
{
	char* d = (char*)dst;
	char* s = (char*)src;
	while(len--)
		*d++ = *s++;
	return dst;
}

void* memset(void* dst, int num, size_t len)
{
	char* d = (char*)dst;
	while(len--)
		*d++ = (char)num;
	return dst;
}

int memcmp(const void* p1, const void* p2, size_t len)
{
	const char* tp1 = (const char*)p1;
	const char* tp2 = (const char*)p2;
	while(len--)
	{
		if(*tp1 != *tp2)
			return (int)(*tp1 - *tp2);
		tp1++;
		tp2++;
	}
	return 0;
}

/**
 * @}
 */
