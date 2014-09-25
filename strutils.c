/**
 * @defgroup strutils string utilities
 *
 * string manipulation utilities.
 *
 * this file is part of the minstdlibs project.
 *
 * https://github.com/drmetal/minstdlibs
 *
 * copyright 2015 Mike Stuart
 *
 * @file strutils.c
 * @{
 */

#include "strutils.h"
#include "minstring.h"

char* strtoupper(char* s)
{
	char* d = s;
	while(*d)
	{
		if((*d <= 'z') && (*d >= 'a'))
			*d -= 32;
		d++;
	}
	return s;
}

char* strtolower(char* s)
{
	char* d = s;
	while(*d)
	{
		if((*d <= 'Z') && (*d >= 'A'))
			*d += 32;
		d++;
	}
	return s;
}

char adtoi(char d)
{
	d = tolower(d);
	if (d >= '0' && d <= '9')
		return d - '0';
	if (d >= 'a' && d <= 'f')
		return d - 'a' + 10;
	return -1;
}

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
 * @}
 */
