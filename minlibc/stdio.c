/*
 * Copyright (c) 2015 Michael Stuart.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
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
 * @defgroup minlibc minimal libc
 *
 * tiny versions of some stdio functions.
 *

int vsprintf(char* dst, const char * fmt, va_list argp)
int sprintf(char* dst, const char* fmt, ...)
int vprintf(const char * fmt, va_list argp)
int printf(const char * fmt, ...)

void init_minlibc() (must be called in crt initialization (init_services under appleseed))

int setvbuf(FILE *stream, char *buf, int mode, size_t size) (not functional)
FILE* fopen(const char * filename, const char * mode)
FILE* fdopen(int fd, const char *mode)
FILE* freopen(const char* filename, const char* mode, FILE* file)
int fclose(FILE* stream)
int fprintf(FILE* stream, const char * fmt, ...)
int getc(FILE* stream) (not provided)
int fgetc(FILE* stream)
int ungetc(int c, FILE* stream)
int fputc(int character, FILE* stream)
int putc(int character, FILE* stream) (not provided)
int fputs(const char* str, FILE* stream)
char* fgets(char* str, int num, FILE* stream)
long int ftell(FILE* stream)
int fseek(FILE * stream, long int offset, int origin)
size_t fwrite(const void *data, size_t size, size_t count, FILE *stream)
size_t fread(void *data, size_t size, size_t count, FILE *stream)
int fflush(FILE* stream)
int fileno(FILE* stream) (possibly broken)
int rename(const char *oldname, const char *newname)
int remove(const char *name)
FILE* tmpfile(void)
char* tmpnam(char *result)

Note: the maximum number of open files depends on a few different settings:
_FS_LOCK: specified in ffconf.h, set in fatfs.mk, and can be set in the project makefile.
            sets the max number of simultaneously open regular files.
MINLIBC_FOPEN_MAX: specified in _stdio.h, my be overidden from the project makefile.
            sets the number of files in the file stream table (can be any type of file).
FILE_TABLE_LENGTH: specified in like_posix_config.h (per project).
            sets the number of files in the file descriptor table (can be any type of file)

 * @file stdio.c
 * @{
 */


#include <stdarg.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h> // STDOUT_FILENO etc
#include <ctype.h> // isdigit() etc
#include <fcntl.h>

#include "strutils.h"

#include "minlibc/stdio.h"
#include "minlibc/stdlib.h"
#include "minlibc/string.h"

fake__FILE __fstab[FOPEN_MAX];
static char __tmpnambuf[L_tmpnam];
fake__FILE* __tmpfs[TMP_MAX];

#define PLUS_FLAG 1
#define MINUS_FLAG 2
#define SPACE_FLAG 4
#define HASH_FLAG 8
#define ZERO_FLAG 16
#define LONG_FLAG 32
#define SHORT_FLAG 64
#define FLOAT_FLAG 128

typedef char*(*putx_t)(int, char**, const char*);

static inline char* __minlibc_putc(int fd, char** dst, const char*src)
{
    if(*src)
    {
        _write(fd, (char*)src, 1);
        (*dst)++;
    }
    return *dst;
}

static inline char* __minlibc_puts(int fd, char** dst, const char*src)
{
    int l = strlen(src);
    if(l > 0)
    {
        _write(fd, (char*)src, l);
        *dst += l;
    }
    return *dst;
}

static inline char* memputc(int fd, char** dst, const char*src)
{
    (void)fd;
    **dst = *src;
    (*dst)++;
    return *dst;
}

static inline char* memputs(int fd, char** dst, const char*src)
{
    (void)fd;
    while(*src)
    {
        **dst = *src;
        (*dst)++;
        src++;
    }
    return *dst;
}

static inline void plusflag(int fd, putx_t _put_char, unsigned int flags, char** dst)
{
    if(flags&PLUS_FLAG)
    {
        char c = '+';
        _put_char(fd, dst, &c);
    }
}

static inline void hashflag(int fd, putx_t _put_str, unsigned int flags, char** dst)
{
    if(flags&HASH_FLAG)
        _put_str(fd, dst, (const char*)"0x");
}

static inline int strfmt(int fd, putx_t _put_char, putx_t _put_str, char** dst, const char * fmt, va_list argp)
{
    float d;
#if MINLIBC_INCLUDE_FLOAT_SUPPORT
    float dps = DEFAULT_FTOA_DECIMAL_PLACES;
#endif
    int ret = -1;
    char* start = *dst;
    void* v;
    unsigned int u;
    int i;
    int padding;
    char padchar;
    const char* s;
    char c;
    char intbuf[28];
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
                        case '.':
                            flags |= FLOAT_FLAG;
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
                    // use intbuf as working area
                    padding = 0;
                    if(padchar)
                    {
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
                        case 'f':
#if MINLIBC_INCLUDE_FLOAT_SUPPORT
                            if(padding)
                                dps = padding;
#endif
                        break;
                    }

                    // handle type
                    switch(*fmt)
                    {
                        case 'c':
                            c = (char)va_arg(argp, int);
                            _put_char(fd, dst, &c);
                        break;

                        case 's':
                            s = (char*)va_arg(argp, char*);
                            if(!s)
                                s = (const char*)"(null)";
                            if(flags&SPACE_FLAG)
                            {
                                padding -= strlen(s);
                                if(padding > 0)
                                {
                                    char padbuf[padding+1];
                                    padbuf[padding] = '\0';
                                    while(padding)
                                    {
                                        padding--;
                                        padbuf[padding] = padchar;
                                    }
                                    _put_str(fd, dst, padbuf);
                                }
                            }
                            _put_str(fd, dst, s);
                        break;

                        case 'i':
                        case 'd':
                            i = (int)va_arg(argp, int);
                            if(i >= 0)
                                plusflag(fd, _put_char, flags, dst);

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
                                    _put_str(fd, dst, padbuf);
                                }
                            }
                            _put_str(fd, dst, intbuf);
                        break;

                        case 'u':
                            u = (unsigned int)va_arg(argp, unsigned int);
                            plusflag(fd, _put_char, flags, dst);

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
                                    _put_str(fd, dst, padbuf);
                                }
                            }

                            _put_str(fd, dst, intbuf);
                        break;

                        case 'x':
                            u = (unsigned int)va_arg(argp, unsigned int);
                            hashflag(fd, _put_str, flags, dst);

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
                                    _put_str(fd, dst, padbuf);
                                }
                            }

                            _put_str(fd, dst, intbuf);
                        break;

                        case 'X':
                            u = (unsigned int)va_arg(argp, unsigned int);
                            hashflag(fd, _put_str, flags, dst);

                            ditoa((int64_t)u, intbuf, 16);
                            // TODO this causes liker error in the test cases!! fix that
    //                      strtoupper(intbuf);

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
                                    _put_str(fd, dst, padbuf);
                                }
                            }

                            _put_str(fd, dst, intbuf);

                        break;

                        case 'p':
                            v = (void*)va_arg(argp, void*);
                            _put_str(fd, dst, (const char*)"0x");

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
                                    _put_str(fd, dst, padbuf);
                                }
                            }
                            _put_str(fd, dst, intbuf);
                        break;

                        case 'f':
                            d = va_arg(argp, double);
                            plusflag(fd, _put_char, flags, dst);
#if MINLIBC_INCLUDE_FLOAT_SUPPORT
                            _put_str(fd, dst, dtoascii(intbuf, d, dps));
#else
                            _put_str(fd, dst, itoa((int)d, intbuf, 10));
#endif
                        break;

                        case '%':
                            c = '%';
                            _put_char(fd, dst, &c);
                        break;
                    }
                break;

                // all other charcters
                default:
                    _put_char(fd, dst, fmt);
                break;
            }
        }

        // null terminate
        c = '\0';
        _put_char(fd, dst, &c);

        ret = (int)(*dst - start);
    }

    return ret;
}

int vsprintf(char* dst, const char * fmt, va_list argp)
{
    return strfmt(-1, memputc, memputs, &dst, fmt, argp) -1;
}

int sprintf(char* dst, const char* fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
    int ret = strfmt(-1, memputc, memputs, &dst, fmt, argp) -1;
    va_end(argp);

    return ret;
}

int vprintf(const char * fmt, va_list argp)
{
    char* dst = NULL;
    return strfmt(STDOUT_FILENO, __minlibc_putc, __minlibc_puts, &dst, fmt, argp);
}

int printf(const char * fmt, ...)
{
    char* dst = NULL;
    va_list argp;
    va_start(argp, fmt);
    int ret = strfmt(STDOUT_FILENO, __minlibc_putc, __minlibc_puts, &dst, fmt, argp);
    va_end(argp);
    return ret;
}


/**
 * this function initializes the FILE stream table, no f-functions should be used before this has been called.
 */
void init_minlibc()
{
    stdin = (__FILE*)&__fstab[FILE_STREAM_TABLE_INDEX_STDIN];
    stdout = (__FILE*)&__fstab[FILE_STREAM_TABLE_INDEX_STDOUT];
    stderr = (__FILE*)&__fstab[FILE_STREAM_TABLE_INDEX_STDERR];

    short i;

    for(i=0; i < FOPEN_MAX; i++)
    {
        __fstab[i]._ub._base = __fstab[i]._ubuf;
        __fstab[i]._ub._size = 0;
        __fstab[i]._bf._base = NULL;
        __fstab[i]._bf._size = 0;
        __fstab[i]._flags = 0;
        __fstab[i]._file = -1;
    }

    __fstab[FILE_STREAM_TABLE_INDEX_STDIN]._file = STDIN_FILENO;
    __fstab[FILE_STREAM_TABLE_INDEX_STDIN]._flags = __SRD|__SNBF;

    __fstab[FILE_STREAM_TABLE_INDEX_STDOUT]._file = STDOUT_FILENO;
    __fstab[FILE_STREAM_TABLE_INDEX_STDOUT]._flags = __SWR|__SNBF;

    __fstab[FILE_STREAM_TABLE_INDEX_STDERR]._file = STDERR_FILENO;
    __fstab[FILE_STREAM_TABLE_INDEX_STDERR]._flags = __SWR|__SNBF;
}

static inline FILE* __get_stream_descriptor(int fdes, int flags)
{
    short i;
    if(fdes == EOF)
        return NULL;
    else if(fdes == STDIN_FILENO)
        return (FILE*)&__fstab[FILE_STREAM_TABLE_INDEX_STDIN];
    else if(fdes == STDOUT_FILENO)
        return (FILE*)&__fstab[FILE_STREAM_TABLE_INDEX_STDOUT];
    else if(fdes == STDERR_FILENO)
        return (FILE*)&__fstab[FILE_STREAM_TABLE_INDEX_STDERR];

    // add 1 to make checking easier with FREAD/FWRITE
    flags++;

    for(i=FILE_STREAM_TABLE_START_REGULAR; i<FOPEN_MAX; i++)
    {
        if(__fstab[i]._flags == 0)
        {
            if(flags & FREAD)
                __fstab[i]._flags |= __SRD;
            else if(flags & FWRITE)
                __fstab[i]._flags |= __SWR;
            else if(flags & (FREAD|FWRITE))
                __fstab[i]._flags |= __SRW;

            __fstab[i]._file = fdes;

            return (FILE*)&__fstab[i];
        }
    }
    return NULL;
}

void __release_stream_descriptor(FILE* stream)
{
    fake__FILE* s = (fake__FILE*)stream;
    if(stream && stream != stdin && stream != stdout && stream != stderr)
    {
        s->_ub._size = 0;
        s->_bf._size = 0;
        s->_flags = 0;
        s->_file = -1;
    }
}

static inline int __get_fileno(FILE* stream)
{
    fake__FILE* s = (fake__FILE*)stream;
    return s ? s->_file : EOF;
}

static inline int __stream_mode(const char* mode)
{
    int flags = 0;
    char m = mode[0];
    char update = ((mode[1] == '+') || (mode[2] == '+'));

    if(m == 'r')
        flags = update ? O_RDWR : O_RDONLY;
    else if(m == 'w')
        flags = O_CREAT | O_TRUNC | (update ? O_RDWR : O_WRONLY);
    else if(m == 'a')
        flags = O_CREAT | O_APPEND | (update ? O_RDWR : O_WRONLY);

    return flags;
}

static inline int __find_tmpindex()
{
    int i;
    for(i = 0; i < TMP_MAX; i++)
    {
        if(!__tmpfs[i])
            break;
    }
    return i == TMP_MAX ? -1 : i;
}

/**
 * not really implemented, will work OK if the buffer is se to NULL, ie specifying system allocated buffer,
 * and when mode is set to _IOFBF. other modes will simply behave as for _IOFBF.
 */
int setvbuf(FILE *stream, char *buf, int mode, size_t size)
{
    fake__FILE* s = (fake__FILE*)stream;
    if(buf)
        return -1;

    switch(mode)
    {
        case _IOFBF:       /* setvbuf should set fully buffered */
            s->_bf._base = buf;
            s->_bf._size = size;
        break;
        case _IOLBF:       /* setvbuf should set line buffered */
            s->_bf._base = buf;
            s->_bf._size = size;
        break;
        case _IONBF:       /* setvbuf should set unbuffered */
            s->_bf._base = buf;
            s->_bf._size = size;
        break;
    }

    return 0;
}

/**
 * Note:
 *      the same file may be opened more than once (and used, this is according to the policy of the like-posix _open() system call)
 *      the file must be closed once for every open made.
 */
FILE* fopen(const char * filename, const char * mode)
{
    FILE* file = NULL;
    int flags = __stream_mode(mode);

    int fd = _open(filename, flags, MINLIBC_STDIO_BUFFER_SIZE);

    if(fd != EOF)
    {
        file = __get_stream_descriptor(fd, flags);
        if(!file)
            _close(fd);
    }

    return file;
}

FILE* fdopen(int fd, const char *mode)
{
    return __get_stream_descriptor(fd, __stream_mode(mode));
}

FILE* freopen(const char* filename, const char* mode, FILE* stream)
{
    _fsync(__get_fileno(stream));
    fclose(stream);
    return fopen(filename, mode);
}

int fclose(FILE* stream)
{
    char buf[16];
    int i = 0;
    int res = EOF;

    if(stream)
    {
        if(stream != stdin && stream != stdout && stream != stderr)
            res = _close(__get_fileno(stream));

        __release_stream_descriptor(stream);

        while(i < TMP_MAX)
        {
            if(__tmpfs[i] == (fake__FILE*)stream)
            {
                __tmpfs[i] = NULL;
                sprintf(buf, P_tmpdir P_tmpfilename, i);
                _unlink(buf);
                break;
            }
            i++;
        }
    }

    return res;
}

int fprintf(FILE* stream, const char * fmt, ...)
{
    char* dst = NULL;
    va_list argp;
    va_start(argp, fmt);
    int ret = strfmt(__get_fileno(stream), __minlibc_putc, __minlibc_puts, &dst, fmt, argp);
    va_end(argp);
    return ret;
}

#undef getc
int getc(FILE* stream)
{
    return fgetc(stream);
}

int fgetc(FILE* stream)
{
    fake__FILE* s = (fake__FILE*)stream;
    int c = EOF;
    if(s->_ub._size > 0)
    {
        s->_ub._size--;
        c = s->_ub._base[s->_ub._size];
    }
    else
        _read(__get_fileno(stream), (char*)&c, 1);
    return c;
}

int ungetc(int c, FILE* stream)
{
    fake__FILE* s = (fake__FILE*)stream;

    if(c == EOF)
        return EOF;
    if(s->_ub._size < (int)sizeof(s->_ubuf))
    {
        s->_ub._base[s->_ub._size] = c;
        s->_ub._size++;
        return 0;
    }
    return EOF;
}

int fputc(int character, FILE* stream)
{
    return _write(__get_fileno(stream), (char*)&character, 1) != EOF ? character : EOF;
}

#undef putc
int putc(int character, FILE* stream)
{
    return _write(__get_fileno(stream), (char*)&character, 1);;
}

int fputs(const char* str, FILE* stream)
{
    return _write(__get_fileno(stream), (char*)str, strlen(str));;
}

char* fgets(char* str, int num, FILE* stream)
{
    char* sptr = str;
    int i;
    int len;
    for(i = 0; i < num-1; )
    {
        len = _read(__get_fileno(stream), str, 1);

        if(len == -1)
            break;
        else if(len)
        {
            if(*str == '\n')
            {
                i++;
                str++;
                break;
            }
            str++;
            i++;
        }
    }
    *str = 0;

    printf(sptr);

    return sptr == str ? NULL : sptr;
}


//char* fgets(char* str, int num, FILE* stream)
//{
//    char* sptr = str;
//    int fd = __get_fileno(stream);
//    long int pos = _ftell(fd);
//    int ret = _read(fd, str, num-1);
//
//    // ensure the string is terminated somewhere...
//    if(ret > 0)
//    {
//        sptr[ret] = '\0';
//
//        while((*sptr != '\n') && (*sptr != '\0'))
//            sptr++;
//
//        if(*sptr == '\n')
//        {
//            sptr++;
//            *sptr = '\0';
//        }
//        // set file pointer to the end of the read line
//        _lseek(fd, pos + (sptr - str), SEEK_SET);
//    }
//
//    return (sptr - str) > 0 ? str : NULL;
//}

long int ftell(FILE* stream)
{
    return _ftell(__get_fileno(stream));
}

int fseek(FILE * stream, long int offset, int origin)
{
    fake__FILE* s = (fake__FILE*)stream;
    s->_ub._size = 0;
    return _lseek(__get_fileno(stream), offset, origin);
}

#if !MINLIBC_BUILD_FOR_TEST
size_t fwrite(const void *data, size_t size, size_t count, FILE *stream)
{
    return _write(__get_fileno(stream), (char*)data, size*count);
}

size_t fread(void *data, size_t size, size_t count, FILE *stream)
{
    return _read(__get_fileno(stream), (char*)data, size*count);
}
#endif

int _fflush(FILE* stream)
{
    return _fsync(__get_fileno(stream));
}

int fileno(FILE* stream)
{
    return __get_fileno(stream);
}

int rename(const char *oldname, const char *newname)
{
    return _rename(oldname, newname);
}

int remove(const char *name)
{
    return _unlink((char*)name);
}

FILE* tmpfile(void)
{
    int i = __find_tmpindex();

    if(i != -1)
    {
        mkdir(P_tmpdir, 0777);
        sprintf(__tmpnambuf, P_tmpdir P_tmpfilename, i);
        __tmpfs[i] = (fake__FILE*)fopen(__tmpnambuf, "w+");
        return (FILE*)__tmpfs[i];
    }

    return  NULL;
}

char* tmpnam(char *result)
{
    char* buf = result ? result : __tmpnambuf;
    int i = __find_tmpindex();

    if(i != -1)
    {
        sprintf(buf, P_tmpdir P_tmpfilename, i);
        return buf;
    }

    return NULL;
}



/**
 * @}
 */
