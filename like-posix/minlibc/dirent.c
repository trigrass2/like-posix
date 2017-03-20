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
 *
 * @file dirent.c
 * @{
 */

#include "minlibc/string.h"
#include "minlibc/stdlib.h"
#include "dirent.h"

static struct dirent _dirent;

/**
 * allocates and populates a DIR info struct.
 * returns NULL if there was no memory allocated or the directory specified didnt exist.
 * the directory must be closed with closedir() by the user.
 */
DIR* opendir(const char *name)
{
    DIR* dir = malloc(sizeof(DIR));

    if(dir)
    {
        if(f_opendir(dir, (const TCHAR*)name) != FR_OK)
        {
            free(dir);
            dir = NULL;
        }
    }

    return dir;
}

/**
 * closes a directory opened with opendir.
 * returns 0 on success, or -1 on error.
 */
int closedir(DIR *dir)
{
    FRESULT res = FR_INVALID_OBJECT;
    if(dir)
    {
        res = f_closedir(dir);
        free(dir);
    }

    return res;
}

/**
 * reads directory info. returns a pointer to a struct dirent,
 * as long as there are entries in the directory.
 * returns NULL when there are no other entries in the directory.
 */
struct dirent* readdir(DIR *dirp)
{
#if r11
    FILINFO info;

    info.lfname = _dirent.d_name;
    info.lfsize = sizeof(_dirent.d_name);

    _dirent.d_name[0] = '\0';
    _dirent.d_type = DT_REG;

    if(f_readdir(dirp, &info) != FR_OK || !info.fname[0])
        return NULL;

    if(_dirent.d_name[0] == '\0')
        strcpy(_dirent.d_name, info.fname);

    if(info.fattrib & AM_DIR)
        _dirent.d_type = DT_DIR;

    return &_dirent;
#else
    FILINFO info;

    _dirent.d_name = info.fname;
    _dirent.d_name[0] = '\0';
    _dirent.d_type = DT_REG;

    if(f_readdir(dirp, &info) != FR_OK || !info.fname[0])
        return NULL;

    if(_dirent.d_name[0] == '\0')
        _dirent.d_name = info.altname;

    if(info.fattrib & AM_DIR)
        _dirent.d_type = DT_DIR;

    return &_dirent;
#endif
}


/**
 * @}
 */
