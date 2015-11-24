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


#include <stdlib.h>
#include <string.h>
#include "pthread.h"

static const pthread_attr_t __default_pattr = {
		.__stacksize = PTHREAD_STACK_MIN,
		.__state = PTHREAD_CREATE_JOINABLE
};

pthread_t __thread_table[PTHREAD_THREADS_MAX];
SemaphoreHandle_t __thread_table_lock;

static inline pthread_t new_pthread()
{
	int i;
	xSemaphoreTake(__thread_table_lock, portMAX_DELAY);
	for(i = 0; i < PTHREAD_THREADS_MAX; i++)
	{
		if(__thread_table[i] == NULL)
		{
			__thread_table[i] = malloc(sizeof(struct _pthread_t));
			break;
		}
	}
	xSemaphoreGive(__thread_table_lock);
	return __thread_table[i];
}

static inline void delete_pthread(pthread_t thread)
{
	int i;
	xSemaphoreTake(__thread_table_lock, portMAX_DELAY);
	for(i = 0; i < PTHREAD_THREADS_MAX; i++)
	{
		if(__thread_table[i] == thread)
		{
			__thread_table[i] = NULL;
			free(thread);
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
	pthread_t pthread;
	if(!attr)
		attr = &__default_pattr;

	if(!__thread_table_lock)
	{
		__thread_table_lock = xSemaphoreCreateMutex();
		assert_true(__thread_table_lock);
	}

	pthread = new_pthread();
	if(pthread)
	{
		pthread->__status = NULL;
		pthread->__join = NULL;
		if(attr->__state == PTHREAD_CREATE_JOINABLE)
			pthread->__join = xSemaphoreCreateBinary();

		ok = xTaskCreate((TaskFunction_t)start_routine, NULL, attr->__stacksize,
								arg, PTHREAD_TASK_PRIO, &pthread->__taskhandle);
		if(ok)
		{
			ret= 0;
			*thread = pthread;
		}
		else if(pthread->__join)
			vSemaphoreDelete(pthread->__join);

		if(ret != 0)
			delete_pthread(pthread);
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
		delete_pthread(thread);
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
		if(status)
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
	memcpy(attr, &__default_pattr, sizeof(pthread_attr_t));
	return -1;
}

int pthread_attr_destroy(pthread_attr_t *attr)
{
	(void)attr;
	return 0;
}

int pthread_attr_getstacksize(pthread_attr_t *attr, size_t * stacksize)
{
	*stacksize = attr->__stacksize;
	return 0;
}

int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize)
{
	attr->__stacksize = stacksize;
	return 0;
}

int pthread_attr_setdetachstate(pthread_attr_t *attr, int state)
{
	attr->__state = state;
	return 0;
}

int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *state)
{
	*state = attr->__state;
	return 0;
}
