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
 * @defgroup strutils string utilities
 *
 * non standard string manipulation utilities.
 *
 * @file strutils.c
 * @{
 */

#include "strutils.h"
#include <ctype.h>



/**
 * convert lowercase items in a string to uppercase, in place.
 */
char* strtoupper(char* s)
{
	char* d = s;
	while(*d)
	{
		*d = toupper((int)*d);
		d++;
	}
	return s;
}

/**
 * convert uppercase items in a string to lowercase, in place.
 */
char* strtolower(char* s)
{
	char* d = s;
	while(*d)
	{
		*d = tolower((int)*d);
		d++;
	}
	return s;
}

/**
 * ascii digit to integer.
 *
 * suppprts digits 0-9,a-f,A-F
 * Eg:
 * 	- adtoi('0')==0
 * 	- adtoi('1')==1
 * 	- adtoi('8')==8
 * 	- adtoi('9')==9
 * 	- adtoi('a')==10
 * 	- adtoi('b')==11
 * 	- adtoi('f')==15
 * 	- adtoi('A')==10
 * 	- adtoi('B')==11
 * 	- adtoi('F')==15
 *
 * 	returns -1 if the digit is not decodable.
 */
char adtoi(char d)
{
	d = tolower((int)d);
	if (d >= '0' && d <= '9')
		return d - '0';
	if (d >= 'a' && d <= 'f')
		return d - 'a' + 10;
	return -1;
}

/**
 * converts a hexadecimal string to integer, not case sensitive.
 *
 * Eg:
 * 	- ahtoi("01")==1
 * 	- ahtoi("ab")==171
 * 	- ahtoi("1243DEF")==19090927
 *
 * 	returns -1 if the string is not decodable.
 */
int ahtoi(char* s)
{
	int value = 0;
	int temp;
	while(*s)
	{
		value *= 16;
		temp = adtoi(*s);
		if(temp == -1)
			return -1;
		value += temp;
		s++;
	}
	return value;
}

/**
 * @brief   checks if a key exists in a list of strings (array of string pointers).
 *          the list should be null terminated, eg:
@code
    const char* list[] = {"abc", "def", NULL};
@endcode
 *
 * @param key is the key string to check.
 * @param key_length is the maximum length of the key string.
 * @param list is an array of string pointers to check against.
 * @retval returns the index into list if the key was found in the list, -1 otherwise.
 */
int string_in_list(const char* str, unsigned short str_len, const char** list)
{
    int i = 0;
    while(list[i] != NULL)
    {
        if(!strncmp(list[i], str, str_len))
            return i;
        i++;
    }
    return -1;
}

/**
 * @}
 */
