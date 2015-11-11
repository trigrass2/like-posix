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

#ifndef _PTHREAD_H_
#define _PTHREAD_H_

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#define STATIC_PTHREAD_ATTR			0
#define PTHREAD_TASK_PRIO			tskIDLE_PRIORITY + 1

#define PTHREAD_THREADS_MAX			32
#define PTHREAD_KEYS_MAX			0
#define PTHREAD_STACK_MIN			configMINIMAL_STACK_SIZE
#define PTHREAD_CREATE_JOINABLE		0
#define PTHREAD_CREATE_DETACHED		1


#if STATIC_PTHREAD_ATTR
typedef struct _pthread_attr_t pthread_attr_t;
#else
typedef struct _pthread_attr_t* pthread_attr_t;
#endif
typedef struct _pthread_t* pthread_t;

int pthread_attr_init(pthread_attr_t *attr);
int pthread_attr_destroy(pthread_attr_t *attr);
int pthread_attr_getstacksize(pthread_attr_t *attr, size_t * stacksize);
int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize);
int pthread_attr_setdetachstate(pthread_attr_t *attr, int state);
int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *state);

int pthread_create(pthread_t* thread, const pthread_attr_t* attr, void* (*start_routine)(void*), void* arg);
void pthread_exit(void* status);
int pthread_join(pthread_t thread, void** status);
int pthread_equal(pthread_t thread_1, pthread_t thread_2);
pthread_t pthread_self(void);
int pthread_detach(pthread_t thread);

//int pthread_mutex_init(pthread_mutex_t * mutex, const pthread_mutex_attr *attr);
//int pthread_mutex_destroy(pthread_mutex_t * mutex);
//int pthread_mutex_lock(pthread_mutex_t * mutex);
//int pthread_mutex_trylock(pthread_mutex_t * mutex);
//int pthread_mutex_unlock(pthread_mutex_t * mutex);


#endif /* _PTHREAD_H_ */
