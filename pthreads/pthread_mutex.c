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
 * This file is part of the Appleseed project, <https://github.com/drmetal/app-l-seed>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

#include <string.h>
#include "pthread.h"

const pthread_mutexattr_t __default_pmattr = {
    .__type = PTHREAD_MUTEX_DEFAULT,
};

int pthread_mutex_init(pthread_mutex_t * mutex, const pthread_mutexattr_t *attr)
{
    int ret = -1;

    if(mutex)
    {
        if(attr)
            mutex->__attr = *attr;
        else
            mutex->__attr = __default_pmattr;

        switch(mutex->__attr.__type)
        {
            case PTHREAD_MUTEX_ERRORCHECK:
            case PTHREAD_MUTEX_DEFAULT:
            case PTHREAD_MUTEX_NORMAL:
                mutex->__mutex = xSemaphoreCreateMutex();
            break;
            case PTHREAD_MUTEX_RECURSIVE:
                mutex->__mutex = xSemaphoreCreateRecursiveMutex();
            break;
        }

        if(mutex->__mutex)
            ret = 0;
    }
    return ret;
}

int pthread_mutex_destroy(pthread_mutex_t * mutex)
{
    vSemaphoreDelete(mutex->__mutex);
    return 0;
}

int pthread_mutex_lock(pthread_mutex_t * mutex)
{
    int ret = -1;
    switch(mutex->__attr.__type)
    {
        case PTHREAD_MUTEX_ERRORCHECK:
        case PTHREAD_MUTEX_DEFAULT:
        case PTHREAD_MUTEX_NORMAL:
            ret = xSemaphoreTake(mutex->__mutex, portMAX_DELAY) == pdTRUE ? 0 : -1;
        break;

        case PTHREAD_MUTEX_RECURSIVE:
            ret = xSemaphoreTakeRecursive(mutex->__mutex, portMAX_DELAY) == pdTRUE ? 0 : -1;
        break;
    }
    return ret;
}

int pthread_mutex_trylock(pthread_mutex_t * mutex)
{
    int ret = -1;
    switch(mutex->__attr.__type)
    {
        case PTHREAD_MUTEX_ERRORCHECK:
        case PTHREAD_MUTEX_DEFAULT:
        case PTHREAD_MUTEX_NORMAL:
            ret = xSemaphoreTake(mutex->__mutex, 0) == pdTRUE ? 0 : -1;
        break;

        case PTHREAD_MUTEX_RECURSIVE:
            ret = xSemaphoreTakeRecursive(mutex->__mutex, 0) == pdTRUE ? 0 : -1;
        break;
    }
    return ret;
}

int pthread_mutex_unlock(pthread_mutex_t * mutex)
{
    int ret = -1;
    switch(mutex->__attr.__type)
    {
        case PTHREAD_MUTEX_ERRORCHECK:
        case PTHREAD_MUTEX_DEFAULT:
        case PTHREAD_MUTEX_NORMAL:
            xSemaphoreGive(mutex->__mutex) == pdTRUE ? 0 : -1;
        break;

        case PTHREAD_MUTEX_RECURSIVE:
            xSemaphoreGiveRecursive(mutex->__mutex) == pdTRUE ? 0 : -1;
        break;
    }
    return ret;
}


int pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
    memcpy(attr, &__default_pmattr, sizeof(pthread_mutexattr_t));
    return -1;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
    (void)attr;
    return 0;
}

int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type)
{
    *type = attr->__type;
    return 0;
}

int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type)
{
    attr->__type = type;
    return 0;
}

