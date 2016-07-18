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
 * This file is part of the lollyjar project, <https://github.com/drmetal/lollyjar>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

/**
 * @defgroup minlibc minimal libc
 *
 * tiny versions of some standard string functions.
 *
 * @file string.c
 * @{
 */

#include <stdlib.h>
#include <stdint.h>
#include "strutils.h"
#include "minlibc/string.h"

const char* __errno_strings[] = {
    "UNKNOWN (0)",
    "EPERM (1) Operation not permitted",
    "ENOENT (2) No such file or directory",
    "ESRCH (3) No such process",
    "EINTR (4) Interrupted system call",
    "EIO (5) I/O error",
    "ENXIO (6) No such device or address",
    "E2BIG (7) Argument list too long",
    "ENOEXEC (8) Exec format error",
    "EBADF (9) Bad file number",
    "ECHILD (10) No child processes",
    "EAGAIN (11) Try again",
    "ENOMEM (12) Out of memory",
    "EACCES (13) Permission denied",
    "EFAULT (14) Bad address",
    "ENOTBLK (15) Block device required",
    "EBUSY (16) Device or resource busy",
    "EEXIST (17) File exists",
    "EXDEV (18) Cross-device link",
    "ENODEV (19) No such device",
    "ENOTDIR (20) Not a directory",
    "EISDIR (21) Is a directory",
    "EINVAL (22) Invalid argument",
    "ENFILE (23) File table overflow",
    "EMFILE (24) Too many open files",
    "ENOTTY (25) Not a typewriter",
    "ETXTBSY (26) Text file busy",
    "EFBIG (27) File too large",
    "ENOSPC (28) No space left on device",
    "ESPIPE (29) Illegal seek",
    "EROFS (30) Read-only file system",
    "EMLINK (31) Too many links",
    "EPIPE (32) Broken pipe",
    "EDOM (33) Math argument out of domain of func",
    "ERANGE (34) Math result not representable",
    "EDEADLK (35) Resource deadlock would occur",
    "ENAMETOOLONG (36) File name too long",
    "ENOLCK (37) No record locks available",
    "ENOSYS (38) Function not implemented",
    "ENOTEMPTY (39) Directory not empty",
    "ELOOP (40) Too many symbolic links encountered",
    "UNKNOWN (41)",
    "ENOMSG (42) No message of desired type",
    "EIDRM (43) Identifier removed",
    "ECHRNG (44) Channel number out of range",
    "EL2NSYNC (45) Level 2 not synchronized",
    "EL3HLT (46) Level 3 halted",
    "EL3RST (47) Level 3 reset",
    "ELNRNG (48) Link number out of range",
    "EUNATCH (49) Protocol driver not attached",
    "ENOCSI (50) No CSI structure available",
    "EL2HLT (51) Level 2 halted",
    "EBADE (52) Invalid exchange",
    "EBADR (53) Invalid request descriptor",
    "EXFULL (54) Exchange full",
    "ENOANO (55) No anode",
    "EBADRQC (56) Invalid request code",
    "EBADSLT (57) Invalid slot",
    "UNKNOWN (58)",
    "EBFONT (59) Bad font file format",
    "ENOSTR (60) Device not a stream",
    "ENODATA (61) No data available",
    "ETIME (62) Timer expired",
    "ENOSR (63) Out of streams resources",
    "ENONET (64) Machine is not on the network",
    "ENOPKG (65) Package not installed",
    "EREMOTE (66) Object is remote",
    "ENOLINK (67) Link has been severed",
    "EADV (68) Advertise error",
    "ESRMNT (69) Srmount error",
    "ECOMM (70) Communication error on send",
    "EPROTO (71) Protocol error",
    "EMULTIHOP (72) Multihop attempted",
    "EDOTDOT (73) RFS specific error",
    "EBADMSG (74) Not a data message",
    "EOVERFLOW (75) Value too large for defined data type",
    "ENOTUNIQ (76) Name not unique on network",
    "EBADFD (77) File descriptor in bad state",
    "EREMCHG (78) Remote address changed",
    "ELIBACC (79) Can not access a needed shared library",
    "ELIBBAD (80) Accessing a corrupted shared library",
    "ELIBSCN (81) .lib section in a.out corrupted",
    "ELIBMAX (82) Attempting to link in too many shared libraries",
    "ELIBEXEC (83) Cannot exec a shared library directly",
    "EILSEQ (84) Illegal byte sequence",
    "ERESTART (85) Interrupted system call should be restarted",
    "ESTRPIPE (86) Streams pipe error",
    "EUSERS (87) Too many users",
    "ENOTSOCK (88) Socket operation on non-socket",
    "EDESTADDRREQ (89) Destination address required",
    "EMSGSIZE (90) Message too long",
    "EPROTOTYPE (91) Protocol wrong type for socket",
    "ENOPROTOOPT (92) Protocol not available",
    "EPROTONOSUPPORT (93) Protocol not supported",
    "ESOCKTNOSUPPORT (94) Socket type not supported",
    "EOPNOTSUPP (95) Operation not supported on transport endpoint",
    "EPFNOSUPPORT (96) Protocol family not supported",
    "EAFNOSUPPORT (97) Address family not supported by protocol",
    "EADDRINUSE (98) Address already in use",
    "EADDRNOTAVAIL (99) Cannot assign requested address",
    "ENETDOWN (100) Network is down",
    "ENETUNREACH (101) Network is unreachable",
    "ENETRESET (102) Network dropped connection because of reset",
    "ECONNABORTED (103) Software caused connection abort",
    "ECONNRESET (104) Connection reset by peer",
    "ENOBUFS (105) No buffer space available",
    "EISCONN (106) Transport endpoint is already connected",
    "ENOTCONN (107) Transport endpoint is not connected",
    "ESHUTDOWN (108) Cannot send after transport endpoint shutdown",
    "ETOOMANYREFS (109) Too many references: cannot splice",
    "ETIMEDOUT (110) Connection timed out",
    "ECONNREFUSED (111) Connection refused",
    "EHOSTDOWN (112) Host is down",
    "EHOSTUNREACH (113) No route to host",
    "EALREADY (114) Operation already in progress",
    "EINPROGRESS (115) Operation now in progress",
    "ESTALE (116) Stale NFS file handle",
    "EUCLEAN (117) Structure needs cleaning",
    "ENOTNAM (118) Not a XENIX named type file",
    "ENAVAIL (119) No XENIX semaphores available",
    "EISNAM (120) Is a named type file",
    "EREMOTEIO (121) Remote I/O error",
    "EDQUOT (122) Quota exceeded",
    "ENOMEDIUM (123) No medium found",
    "EMEDIUMTYPE (124) Wrong medium type",
    "ECANCELED (125) Operation Canceled",
    "ENOKEY (126) Required key not available",
    "EKEYEXPIRED (127) Key has expired",
    "EKEYREVOKED (128) Key has been revoked",
    "EKEYREJECTED (129) Key was rejected by service",
    "EOWNERDEAD (130) Owner died",
    "ENOTRECOVERABLE (131) State not recoverable"
};




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

char* strerror(int errnum)
{
    return (char*)__errno_strings[errnum];
}

char* strchr(const char* src, int character)
{
    while(*src && ((const char)character) != *src)
        src++;
    return ((const char)character) == *src ? (char*)src : (char*)NULL;
}

size_t strspn(const char* string, const char* skipset)
{
    const char* s;
    size_t len = 0;
    while(*string)
    {
        for(s=skipset; *s && *s != *string; s++);
        if(!*s)
            break;
        len++;
        string++;
    }
    return len;
}

char* strpbrk(const char*string, const char*stopset)
{
    const char* s;
    while(*string)
    {
        for(s=stopset; *s && *s != *string; s++);
        if(*s)
            return (char*)string;
        string++;
    }
    return (char*)NULL;
}

/*
 * just implement like strcmp for now
 */
int strcoll(const char *s1, const char *s2)
{
    return strcmp(s1, s2);
}

char* strstr(const char *haystack, const char *needle)
{
    const char* s;
    const char* h;

    if(!*needle)
        return (char*)haystack;


    while(*haystack)
    {
        h = haystack;
        s = needle;
        while(*s && *h)
        {
            if(!*s)
                return (char*)haystack;
            if(*s == *h)
            {
                s++;
                h++;
            }
            else
                break;
        }
        haystack++;
    }
    return NULL;
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

void* memchr(const void *block, int c, size_t size)
{
    const char* bl = (const char*)block;
    while(size && ((const char)c) != *bl)
    {
        bl++;
        size--;
    }
    return ((const char)c) == *bl ? (void*)bl : (void*)NULL;
}

/**
 * @}
 */
