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
 * This file is part of the Appleseed project, <https://github.com/drmetal/app-l-seed>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

/**
 * @defgroup minstring minimal string
 *
 * tiny versions of some standard string functions.
 *
 * @file minstring.c
 * @{
 */

#include <stdlib.h>
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
