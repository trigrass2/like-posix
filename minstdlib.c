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
 * @defgroup minstdlib minimal stdlib
 *
 * tiny versions of some stdlib functions.
 *
 * @file minstdlib.c
 * @{
 */

#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include <string.h>
#include "minstdlib.h"

void strreverse(char* begin, char* end)
{
	char aux;
	while(end>begin)
		aux=*end, *end--=*begin, *begin++=aux;
}

char* itoa(int value, char* str, int base)
{
	static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	char* wstr=str;
	int sign;

	// Validate base
	if (base<2 || base>35)
	{
		*wstr='\0';
		return 0;
	}
	// Take care of sign
	if ((sign=value) < 0)
		value = -value;
	// Conversion. Number is reversed.
	do
		*wstr++ = num[value%base];
	while(value /= base);
	// append sign
	if((base == 10) && (sign < 0))
		*wstr++='-';

	// terminate
	*wstr='\0';
	// Reverse string
	strreverse(str,wstr-1);

	return str;
}

char* ditoa(int64_t value, char* str, int base)
{
	static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	char* wstr=str;
	int64_t sign;

	// Validate base
	if (base<2 || base>35)
	{
		*wstr='\0';
		return 0;
	}
	// Take care of sign
	if ((sign=value) < 0)
		value = -value;
	// Conversion. Number is reversed.
	do
		*wstr++ = num[value%base];
	while(value /= base);
	// append sign
	if((base == 10) && (sign < 0))
		*wstr++='-';

	// terminate
	*wstr='\0';
	// Reverse string
	strreverse(str,wstr-1);

	return str;
}

static double PRECISION = 0.00000000000001;

char * ftoa(char *s, double n)
{
	// handle special cases
	if (isnan(n)) {
		strcpy(s, "nan");
	} else if (isinf(n)) {
		strcpy(s, "inf");
	} else if (n == (double)0.0) {
		strcpy(s, "0");
	} else {
		int digit, m, m1 = 0;
		char *c = s;
		int neg = (n < 0);
		if (neg)
			n = -n;
		// calculate magnitude
		m = log10(n);
		int useExp = (m >= 14 || (neg && m >= 9) || m <= -9);
		if (neg)
			*(c++) = '-';
		// set up for scientific notation
		if (useExp) {
			if (m < 0)
				m -= (double)1.0;
			n = n / pow((double)10.0, m);
			m1 = m;
			m = 0;
		}
		if (m < (double)1.0) {
			m = 0;
		}
		// convert the number
		while (n > PRECISION || m >= 0) {
			double weight = pow((double)10.0, m);
			if (weight > 0 && !isinf(weight)) {
				digit = floor(n / weight);
				n -= (digit * weight);
				*(c++) = '0' + digit;
			}
			if (m == 0 && n > 0)
				*(c++) = '.';
			m--;
		}
		if (useExp) {
			// convert the exponent
			int i, j;
			*(c++) = 'e';
			if (m1 > 0) {
				*(c++) = '+';
			} else {
				*(c++) = '-';
				m1 = -m1;
			}
			m = 0;
			while (m1 > 0) {
				*(c++) = '0' + m1 % 10;
				m1 /= 10;
				m++;
			}
			c -= m;
			for (i = 0, j = m-1; i<j; i++, j--) {
				// swap without temporary
				c[i] ^= c[j];
				c[j] ^= c[i];
				c[i] ^= c[j];
			}
			c += m;
		}
		*(c) = '\0';
	}
	return s;
}

/**
 * @}
 */
