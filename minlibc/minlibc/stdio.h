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

#ifndef MINSTDIO_H_
#define MINSTDIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "minlibc/config.h"

/**
 * buffer size in bytes used by some stdio functions
 */
#ifndef MINLIBC_STDIO_BUFFER_SIZE
#define MINLIBC_STDIO_BUFFER_SIZE   128
#endif

/**
 * include floating point string formatting in xxprintf() functions
 */
#ifndef MINLIBC_INCLUDE_FLOAT_SUPPORT
#define MINLIBC_INCLUDE_FLOAT_SUPPORT 1
#endif

#ifdef MINLIBC_FOPEN_MAX
#define __FOPEN_MAX__       MINLIBC_FOPEN_MAX
#else
#define __FOPEN_MAX__       20
#endif

#ifdef MINLIBC_BUFSIZ
#define __BUFSIZ__       MINLIBC_BUFSIZ
#else
#define __BUFSIZ__       512
#endif

#ifdef MINLIBC_FILENAME_MAX
#define __FILENAME_MAX__    MINLIBC_FILENAME_MAX
#else
#define __FILENAME_MAX__       255
#endif

#ifdef MINLIBC_L_tmpnam
#define __L_tmpnam__    MINLIBC_L_tmpnam
#else
#define __L_tmpnam__    __FILENAME_MAX__
#endif

#ifndef P_tmpdir
#define P_tmpdir   "/tmp"
#endif
#ifndef P_tmpfilename
#define P_tmpfilename   "/%d.tmp"
#endif

//#define TMP_MAX     8

#define FILE_STREAM_TABLE_INDEX_STDIN       0
#define FILE_STREAM_TABLE_INDEX_STDOUT      1
#define FILE_STREAM_TABLE_INDEX_STDERR      2
#define FILE_STREAM_TABLE_START_REGULAR     3

#include <stdio.h>

/**
 * the __Sxxx macros are not there on all libc's
 */
#ifndef __SLBF
#define __SLBF  0x0001      /* line buffered */
#endif
#ifndef __SNBF
#define __SNBF  0x0002      /* unbuffered */
#endif
#ifndef __SRD
#define __SRD   0x0004      /* OK to read */
#endif
#ifndef __SWR
#define __SWR   0x0008      /* OK to write */
#endif
#ifndef __SRW
    /* RD and WR are never simultaneously asserted */
#define __SRW   0x0010      /* open for reading & writing */
#endif
#ifndef __SEOF
#define __SEOF  0x0020      /* found EOF */
#endif
#ifndef __SERR
#define __SERR  0x0040      /* found error */
#endif
#ifndef __SMBF
#define __SMBF  0x0080      /* _buf is from malloc */
#endif
#ifndef __SAPP
#define __SAPP  0x0100      /* fdopen()ed in append mode - so must  write to end */
#endif
#ifndef __SSTR
#define __SSTR  0x0200      /* this is an sprintf/snprintf string */
#endif
#ifndef __SOPT
#define __SOPT  0x0400      /* do fseek() optimisation */
#endif
#ifndef __SNPT
#define __SNPT  0x0800      /* do not do fseek() optimisation */
#endif
#ifndef __SOFF
#define __SOFF  0x1000      /* set iff _offset is in fact correct */
#endif
#ifndef __SORD
#define __SORD  0x2000      /* true => stream orientation (byte/wide) decided */
#endif

/**
 * the FREAD and FWRITE are not there on all libc's
 */
#ifndef FREAD
#define FREAD   1
#define FWRITE  2
#endif

/**
 * substitute __sbuf
 */
struct fake__sbuf {
    char *_base;
    int _size;
};

/**
 * extra small FILE type, replaces newlib FILE inside the stdio implementation
 */
typedef struct {
//    char *_p;  /* current position in (some) buffer */
//    int   _r;           /* read space left for getc() */
//    int   _w;           /* write space left for putc() */
    short _flags;       /* flags, below; this FILE is free if 0 */
    short _file;        /* fileno, if Unix descriptor, else -1 */
    struct fake__sbuf _bf;  /* the buffer (at least 1 byte, if !NULL) */
    struct fake__sbuf _ub;  /* ungetc buffer (at least 1 byte, if !NULL) */
    char _ubuf[1];   /* guarantee an ungetc() buffer */
} fake__FILE;


void init_minlibc();

/**
 * the following must be defined somewhere for stdio to link.
 * see like-posix syscalls.c
 */

extern int _open(const char *name, int flags, int mode);
extern int _close(int file);
extern int _write(int file, char *buffer, int count);
extern int _read(int file, char *buffer, int count);
extern int _lseek(int file, int offset, int whence);
extern long int _ftell(int fd);
extern int _unlink(char *name);
extern int _rename(const char *oldname, const char *newname);
extern int _mkdir(const char *pathname, mode_t mode);
extern int _fsync(int file);

/**
 * not all of the API is held in all libc's
 */
FILE* fdopen(int, const char*);

#ifdef __cplusplus
}
#endif

#endif /* MINSTDIO_H_ */


