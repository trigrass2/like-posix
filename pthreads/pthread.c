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


#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "pthread.h"

struct _pthread_attr_t{
	size_t 	__stacksize;
	int 	__state;
};

struct _pthread_t {
	TaskHandle_t __taskhandle;
	SemaphoreHandle_t __join;	// used throughout not only to perform joining, but to indicate joinableness.
	void* __status;
};

static const struct _pthread_attr_t __default_pattr = {
		.__stacksize = PTHREAD_STACK_MIN,
		.__state = PTHREAD_CREATE_JOINABLE
};

pthread_t __thread_table[PTHREAD_THREADS_MAX];
SemaphoreHandle_t __thread_table_lock;

static inline void insert_pthread(pthread_t thread)
{
	int i;
	xSemaphoreTake(__thread_table_lock, portMAX_DELAY);
	for(i = 0; i < PTHREAD_THREADS_MAX; i++)
	{
		if(!__thread_table[i])
		{
			__thread_table[i] = thread;
			break;
		}
	}
	xSemaphoreGive(__thread_table_lock);
}

static inline void remove_pthread(pthread_t thread)
{
	int i;
	xSemaphoreTake(__thread_table_lock, portMAX_DELAY);
	for(i = 0; i < PTHREAD_THREADS_MAX; i++)
	{
		if(__thread_table[i] == thread)
		{
			__thread_table[i] = NULL;
			break;
		}
	}
	xSemaphoreGive(__thread_table_lock);
}

static inline pthread_t find_pthread()
{
	int i;
	pthread_t thread = NULL;
	xSemaphoreTake(__thread_table_lock, portMAX_DELAY);
	TaskHandle_t task = xTaskGetCurrentTaskHandle();
	for(i = 0; i < PTHREAD_THREADS_MAX; i++)
	{
		if(__thread_table[i]->__taskhandle == task)
		{
			thread = __thread_table[i];
			break;
		}
	}
	xSemaphoreGive(__thread_table_lock);
	return thread;
}


int pthread_create(pthread_t* thread, const pthread_attr_t* attr, void* (*start_routine)(void*), void* arg)
{
	int ret = -1;
	BaseType_t ok = pdFALSE;
	pthread_t pthread = malloc(sizeof(struct _pthread_t));
	pthread_attr_t pattr;
	if(attr)
		pattr = *attr;
	else
		pattr = (pthread_attr_t)&__default_pattr;

	if(!__thread_table_lock)
	{
		__thread_table_lock = xSemaphoreCreateMutex();
		assert_true(__thread_table_lock);
	}

	if(pthread)
	{
		pthread->__status = NULL;
		pthread->__join = NULL;
		if(pattr->__state == PTHREAD_CREATE_JOINABLE)
			pthread->__join = xSemaphoreCreateBinary();

		ok = xTaskCreate((TaskFunction_t)start_routine, NULL, pattr->__stacksize,
								arg, PTHREAD_TASK_PRIO, &pthread->__taskhandle);
		if(ok)
		{
			insert_pthread(pthread);
			ret= 0;
			*thread = pthread;
		}
		else if(pthread->__join)
			vSemaphoreDelete(pthread->__join);

		if(ret != 0)
			free(pthread);
	}

	return ret;
}

void pthread_exit(void* status)
{
	pthread_t thread = find_pthread();
	if(thread)
	{
		thread->__status = status;
		if(thread->__join)
			xSemaphoreGive(thread->__join);
		remove_pthread(thread);
		vTaskDelete(NULL);
	}
}

int pthread_join(pthread_t thread, void** status)
{
	BaseType_t ok = pdFALSE;
	if(thread && thread->__join)
	{
		ok = xSemaphoreTake(thread->__join, portMAX_DELAY);
		vSemaphoreDelete(thread->__join);
		*status = thread->__status;
	}
	return ok ? 0 : -1;
}

int pthread_detach(pthread_t thread)
{
	if(thread && thread->__join)
	{
		xSemaphoreGive(thread->__join);
		vSemaphoreDelete(thread->__join);
		thread->__join = NULL;
		return 0;
	}
	return -1;
}

int pthread_equal(pthread_t thread_1, pthread_t thread_2)
{
	return thread_1 && (thread_1 == thread_2) ? 0 : -1;
}

pthread_t pthread_self(void)
{
	return find_pthread();
}


int pthread_attr_init(pthread_attr_t *attr)
{
#if STATIC_PTHREAD_ATTR
	memcpy(*attr, &__default_pattr, sizeof(__default_pattr));
#else
	pthread_attr_t pattr = malloc(sizeof(struct _pthread_attr_t));
	if(pattr)
	{
		memcpy(pattr, &__default_pattr, sizeof(__default_pattr));
		*attr = pattr;
		return 0;
	}
#endif
	return -1;
}

int pthread_attr_destroy(pthread_attr_t *attr)
{
#if STATIC_PTHREAD_ATTR
	(void)attr;
#else
	if(*attr)
		free(*attr);
#endif
	return 0;
}

int pthread_attr_getstacksize(pthread_attr_t *attr, size_t * stacksize)
{
#if STATIC_PTHREAD_ATTR
	*stacksize = attr->__stacksize;
#else
	*stacksize = (*attr)->__stacksize;
#endif
	return 0;
}

int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize)
{
#if STATIC_PTHREAD_ATTR
	attr->__stacksize = stacksize;
#else
	(*attr)->__stacksize = stacksize;
#endif
	return 0;
}

int pthread_attr_setdetachstate(pthread_attr_t *attr, int state)
{
#if STATIC_PTHREAD_ATTR
	attr->__state = state;
#else
	(*attr)->__state = state;
#endif
	return 0;
}

int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *state)
{
#if STATIC_PTHREAD_ATTR
	*state = attr->__state;
#else
	*state = (*attr)->__state;
#endif
	return 0;
}
