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
 * This file is part of the Appleseed project, <https://github.com/drmetal/appleseed>
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
#include <ctype.h>
#include <minlibc/stdlib.h>
#include <string.h>

int atoi(const char* string)
{
    const char* s = string;
    int mult;
    int n;
    int result = 0;
    for(n=-1; isdigit((int)*s); s++, n++);
    for(mult = 1; n>=0; n--, mult*=10)
        result += (string[n]-'0') * mult;

    return result;
}

double atof(const char* string)
{
    double mandiv = 1;
    const char* mantissa = strchr(string, '.');

    double result = atoi(string);
    double manint;
    if(mantissa)
    {
        mantissa++;
        for(;*mantissa == '0';mantissa++)
            mandiv *= (double)10;

        manint = (double)atoi(mantissa);

        for(;*mantissa; mantissa++)
            mandiv *= (double)10;

        result += manint / mandiv;
    }
    return result;
}

double strtod(const char *string, char **tailptr)
{
    double result = NAN;
    int mandiv = 1;
    double manint;
    const char* mantissa;

    while(isspace((int)*string))
        string++;

    if(*string)
    {
        mantissa = strchr(string, '.');
        result = (double)atoi(string);

        if(mantissa)
        {
            mantissa++;
            for(;*mantissa == '0';mantissa++)
                mandiv *= 10;

            manint = (double)atoi(mantissa);

            for(;*mantissa && !isspace((int)*mantissa); mantissa++)
                mandiv *= 10;

            result += manint / mandiv;

            string = mantissa;

            if(tailptr && *string)
            {
                while(isspace((int)*string))
                    string++;
                *tailptr = (char*)string;
            }
        }
    }

    return result;
}

float strtof(const char *string, char **tailptr)
{
    float result = NAN;
    int mandiv = 1;
    float manint;
    const char* mantissa;

    while(isspace((int)*string))
        string++;

    if(*string)
    {
        mantissa = strchr(string, '.');
        result = (float)atoi(string);

        if(mantissa)
        {
            mantissa++;
            for(;*mantissa == '0';mantissa++)
                mandiv *= 10;

            manint = (float)atoi(mantissa);

            for(;*mantissa && !isspace((int)*mantissa); mantissa++)
                mandiv *= 10;

            result += manint / mandiv;

            string = mantissa;

            if(tailptr && *string)
            {
                while(isspace((int)*string))
                    string++;
                *tailptr = (char*)string;
            }
        }
    }

    return result;
}

char* getenv(const char* name)
{
    (void)name;
    return NULL;
}

static unsigned int __rand_xn = 1;
#define  __rand_a  1103515245
#define  __rand_c  12345
#define  __rand_m  2147483647

void srand(unsigned int seed)
{
    __rand_xn = seed;
}

int rand()
{
    __rand_xn = (__rand_a * __rand_xn + __rand_c) % __rand_m;
    return __rand_xn;
}

#if USE_SHELL
#pragma message("compiling stdlib system() with shell support")
#else
#pragma message("compiling stdlib system() without shell support")
#endif

int system(const char *command)
{
#if USE_SHELL
    return _system(command);
#else
    (void)command;
    return -1;
#endif
}

void exit(int code)
{
    _exit(code);
    while(1);
}

/**
 * @}
 */
