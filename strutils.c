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

/**
 * convert lowercase items in a string to uppercase, in place.
 */
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

/**
 * convert uppercase items in a string to lowercase, in place.
 */
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
	d = tolower(d);
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
 * @}
 */
