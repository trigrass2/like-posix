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
 * This file is part of the like-posix project, <https://github.com/drmetal/like-posix>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

/**
 * @addtogroup syscalls System Calls
 *
 * relies upon:
 * - FreeRTOS (optional)
 * - get_hw_time() must be defined somewhere in the device drivers.
\code
  void get_hw_time(unsigned long* secs, unsigned long* usecs);
\endcode
 *
 * @file syscalls.c
 * @{
 */

#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#ifndef USE_FREERTOS
#define USE_FREERTOS 0
#endif

#if USE_LIKEPOSIX
#include "likeposix_config.h"
#endif

#if USE_FREERTOS
#pragma message("building with clock system call")
#include "FreeRTOS.h"
#include "task.h"
#else
#pragma message("building without clock() support")
#endif


#if USE_DRIVER_SYSTEM_TIMER
#pragma message("building with hardware timer support")
extern void get_hw_time(unsigned long* secs, unsigned long* usecs);
#else
#pragma message("building without hardware timer support")
#define get_hw_time(sec, usec) (void)sec;(void)usec
#endif

#ifndef TIMEZONE_OFFSET
#pragma message("building timezone offset set to 0 - normally defined in likeposix_config.h")
#define TIMEZONE_OFFSET 0
#endif

int gettimeofday(struct timeval *tp, void *tzp)
{
    (void)tzp;
    get_hw_time((unsigned long*)&tp->tv_sec, (unsigned long*)&tp->tv_usec);
    tp->tv_sec += TIMEZONE_OFFSET;
    return 0;
}

time_t time(time_t* time)
{
    time_t sec = 0;
    time_t usec;
    get_hw_time((unsigned long*)&sec, (unsigned long*)&usec);
    sec += TIMEZONE_OFFSET;
    if(time)
        *time = sec;
    return sec;
}

double difftime(time_t time1, time_t time0)
{
    return time1 - time0;
}

/**
 * clock_t defined in milliseconds
 */
clock_t clock()
{
#if USE_FREERTOS
    return xTaskGetTickCount()/portTICK_RATE_MS;
#else
    return (clock_t)-1;
#endif
}

/* Nonzero if `y' is a leap year, else zero. */
#define leap(y) (((y) % 4 == 0 && (y) % 100 != 0) || (y) % 400 == 0)

/* Number of leap years from 1970 to `y' (not including `y' itself). */
#define nleap(y) (((y) - 1969) / 4 - ((y) - 1901) / 100 + ((y) - 1601) / 400)

struct tm __localtime;

char __monthdays[] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

const char* __weekday[] = {
        "Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday",
};

const char* __month[] = {
        "January","February","March","April","May","June","July", "August", "September", "October", "November", "December"
};

#define H_PER_DAY 24
#define M_PER_HOUR 60
#define S_PER_MIN 60

time_t mktime(struct tm *brokentime)
{
    int year = brokentime->tm_year + 1900;
    int months = brokentime->tm_mon;
    int days = brokentime->tm_mday-1;

    if(months > 1 && leap (year))
        days++;
    while (months-- > 0)
        days += __monthdays[months];
    brokentime->tm_yday = days;

    days += 365 * (year - 1970) + nleap (year);
    brokentime->tm_wday = (days + 4) % 7;
    brokentime->tm_isdst = 0;

    return (60*60*24 * days) + (60*60 * brokentime->tm_hour) + (60 * brokentime->tm_min) + brokentime->tm_sec;
}

struct tm * gmtime(const time_t *time)
{
    time_t t = *time;

    // find days since the unix epoch
    int days = t / (H_PER_DAY * M_PER_HOUR * S_PER_MIN);
    t = t - (days*H_PER_DAY * M_PER_HOUR * S_PER_MIN);
    __localtime.tm_hour = t / (M_PER_HOUR * S_PER_MIN);
    t = t - (__localtime.tm_hour * M_PER_HOUR * S_PER_MIN);
    __localtime.tm_min =  t / S_PER_MIN;
    __localtime.tm_sec = t - (__localtime.tm_min * S_PER_MIN);


    int d = days;
    unsigned int year = 1900;
    __localtime.tm_year = 1970-1900;
    int ndays;
    while(d > 0)
    {
        ndays = leap(year) ? 366 : 365;
        d -= ndays;
        if(d > 0)
        {
            __localtime.tm_year++;
            year++;
        }
        else
            __localtime.tm_yday = ndays + d;
    }

    d = __localtime.tm_yday;
    int m = 0;
    int leapday = leap(year) ? 1 : 0;
    while(d > 0)
    {
        d -= __monthdays[m];
        if(m == 1)
            d -= leapday;

        if(d > 0)
            m++;
        else
        {
            __localtime.tm_mday = __monthdays[m] + d + 1;
            if(m == 1)
                __localtime.tm_mday = leapday;
        }
    }
    __localtime.tm_mon = m;

    // 01/01/1970 was a thursday
    __localtime.tm_wday = (days + 4) % 7;

    return &__localtime;
}

struct tm * localtime(const time_t *time)
{
	time_t t = *time + TIMEZONE_OFFSET;
    return gmtime(&t);
}

/**
 *
%a   Abbreviated weekday name *  Thu
%A  Full weekday name *     Thursday
%b  Abbreviated month name *    Aug
%B  Full month name *   August
%c  Date and time representation *  Thu Aug 23 14:55:02 2001
%C  Year divided by 100 and truncated to integer (00-99)    20
%d  Day of the month, zero-padded (01-31)   23
%D  Short MM/DD/YY date, equivalent to %m/%d/%y 08/23/01
%e  Day of the month, space-padded ( 1-31)  23
%F  Short YYYY-MM-DD date, equivalent to %Y-%m-%d   2001-08-23
%g  Week-based year, last two digits (00-99)    01
%G  Week-based year 2001
%h  Abbreviated month name * (same as %b)   Aug
%H  Hour in 24h format (00-23)  14
%I  Hour in 12h format (01-12)  02
%j  Day of the year (001-366)   235
%m  Month as a decimal number (01-12)   08
%M  Minute (00-59)  55
%n  New-line character ('\n')
%p  AM or PM designation    PM
%r  12-hour clock time *    02:55:02 pm
%R  24-hour HH:MM time, equivalent to %H:%M 14:55
%S  Second (00-61)  02
%t  Horizontal-tab character ('\t')
%T  ISO 8601 time format (HH:MM:SS), equivalent to %H:%M:%S 14:55:02
%u  ISO 8601 weekday as number with Monday as 1 (1-7)   4
%U  Week number with the first Sunday as the first day of week one (00-53)  33
%V  ISO 8601 week number (00-53)    34
%w  Weekday as a decimal number with Sunday as 0 (0-6)  4
%W  Week number with the first Monday as the first day of week one (00-53)  34
%x  Date representation *   08/23/01
%X  Time representation *   14:55:02
%y  Year, last two digits (00-99)   01
%Y  Year    2001
%z  ISO 8601 offset from UTC in timezone (1 minute=1, 1 hour=100)
If timezone cannot be termined, no characters   +100
%Z  Timezone name or abbreviation *
If timezone cannot be termined, no characters   CDT
%%  A % sign    %
*/
size_t strftime(char *s, size_t size, const char *t, const struct tm *brokentime)
{
    int length = 0;
    int len = 0;
    int year = brokentime->tm_year+1900;

    for(; size > 0 && *t; t++)
    {
        if(*t == '%')
        {
            t++;
            switch(*t)
            {
                case 'a':
                    len = snprintf(s, size, "%.3s", __weekday[brokentime->tm_wday]);
                break;
                case 'A':
                    len = snprintf(s, size, "%s", __weekday[brokentime->tm_wday]);
                break;
                case 'h':
                case 'b':
                    len = snprintf(s, size, "%.3s", __month[brokentime->tm_mon]);
                break;
                case 'B':
                    len = snprintf(s, size, "%s", __month[brokentime->tm_mon]);
                break;
                case 'c':
                    // "Thu Aug 23 14:55:02 2001"
                    len = snprintf(s, size, "%.3s %.3s %2d %02d:%02d:%02d %d",
                                __weekday[brokentime->tm_wday], __month[brokentime->tm_mon], brokentime->tm_mday,
                                brokentime->tm_hour, brokentime->tm_min, brokentime->tm_sec, year);
                break;
                case 'C':
                    len = snprintf(s, size, "%d", year/100);
                break;
                case 'd':
                    len = snprintf(s, size, "%02d", brokentime->tm_mday);
                break;
                case 'D':
                    // "MM/DD/YY"
                    len = snprintf(s, size, "%02d/%02d/%02d", brokentime->tm_mon+1,brokentime->tm_mday,
                            year - ((year/100)*100));
                break;
                case 'e':
                    len = snprintf(s, size, "%2d", brokentime->tm_mday);
                break;
                case 'F':
                    // "2001-08-23"
                    len = snprintf(s, size, "%d-%02d-%02d", year, brokentime->tm_mon+1, brokentime->tm_mday);
                break;
                case 'g':
                    len = snprintf(s, size, "%d", year - ((year/100)*100));
                break;
                case 'G':
                    len = snprintf(s, size, "%d", year);
                break;
                case 'H':
                    len = snprintf(s, size, "%02d", brokentime->tm_hour);
                break;
                case 'I':
                    len = snprintf(s, size, "%02d", !brokentime->tm_hour ? 12 : ((brokentime->tm_hour-1)%12) + 1);
                    printf("%d %d %d\n", -1%12, 0 %12, 1%12);
                break;

                case 'j':  //Day of the year (001-366)   235
                    len = snprintf(s, size, "%03d", brokentime->tm_yday+1);
                break;
                case 'm':  //Month as a decimal number (01-12)   08
                    len = snprintf(s, size, "%02d", brokentime->tm_mon+1);
                break;
                case 'M':  //Minute (00-59)  55
                    len = snprintf(s, size, "%02d", brokentime->tm_min);
                break;
                case 'n':  //New-line character ('\n')
                    *s = '\n';
                    len = 1;
                break;
                case 'p':  //AM or PM designation    PM
                    len = snprintf(s, size, "%s", brokentime->tm_hour >=12 ? "PM" : "AM");
                break;
                case 'r':  //12-hour clock time *    02:55:02 pm
                    len = snprintf(s, size, "%02d:%02d:%02d %s", !brokentime->tm_hour ? 12 : ((brokentime->tm_hour-1)%12) + 1,  brokentime->tm_min,  brokentime->tm_sec,
                            brokentime->tm_hour >=12 ? "PM" : "AM");
                break;
                case 'R':  //24-hour HH:MM time, equivalent to %H:%M 14:55
                    len = snprintf(s, size, "%02d:%02d", brokentime->tm_hour,  brokentime->tm_min);
                break;
                case 'S':  //Second (00-61)  02
                    len = snprintf(s, size, "%02d", brokentime->tm_sec);
                break;
                case 't':  //Horizontal-tab character ('\t')
                    *s = '\t';
                    len = 1;
                break;
                case 'T':  //ISO 8601 time format (HH:MM:SS), equivalent to %H:%M:%S 14:55:02
                    len = snprintf(s, size, "%02d:%02d:%02d", brokentime->tm_hour,  brokentime->tm_min,  brokentime->tm_sec);
                break;
                case 'u':  //ISO 8601 weekday as number with Monday as 1 (1-7)   4
                    len = snprintf(s, size, "%d", brokentime->tm_wday ? brokentime->tm_wday : 7);
                break;
                case 'U':  //Week number with the first Sunday as the first day of week one (00-53)33
//                    len = snprintf(s, size, "%d", brokentime->tm_yday/7);
                     len = 0;
                break;
                case 'V':  //ISO 8601 week number (00-53)    34
//                     len = snprintf(s, size, "%d", brokentime->tm_yday/7);
                     len = 0;
                break;
                case 'w':  //Weekday as a decimal number with Sunday as 0 (0-6)  4
                    len = snprintf(s, size, "%d", brokentime->tm_wday);
                break;
                case 'W':  //Week number with the first Monday as the first day of week one (00-53) 34
//                    len = snprintf(s, size, "%d", brokentime->tm_yday/7);
                    len = 0;
                break;
                case 'x':  //Date representation *   08/23/01
                    len = snprintf(s, size, "%02d/%02d/%02d", brokentime->tm_mon+1,  brokentime->tm_mday,  year - ((year/100)*100));
                break;
                case 'X':  //Time representation *   14:55:02
                    len = snprintf(s, size, "%02d:%02d:%02d", brokentime->tm_hour,  brokentime->tm_min,  brokentime->tm_sec);
                break;
                case 'y':  //Year, last two digits (00-99)   01
                    len = snprintf(s, size, "%d", year - ((year/100)*100));
                break;
                case 'Y':  //Year    2001
                    len = snprintf(s, size, "%d", year);
                break;
                case 'z':  //ISO 8601 offset from UTC in timezone (1 minute=1, 1 hour=100)If timezone cannot be termined, no characters   +100
                    len = 0;
                break;
                case 'Z':  //Timezone name or abbreviation *
                    len = 0;
                break;
                case '%':
                *s = *t;
                len = 1;
                break;
            }
        }
        else
        {
            *s = *t;
            len = 1;
        }
        length += len;
        size -= len;
        s += len;
    }

    *s = 0;

    return length;
}

/**
 * @}
 */
