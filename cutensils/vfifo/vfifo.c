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

#include "vfifo.h"

/**
  * @brief 	creates a void pointer fifo.
  * @param	fifo is a pointer to a fifo structure.
  * @param  buf is a pointer to the buffer memory.
  * @param  size is the number of slots of type vfifo_primitive_t in the data space.
  */
void vfifo_init(vfifo_t* fifo, void* buf, int32_t slots)
{
	if(fifo)
    {
    	fifo->head = 0;
    	fifo->tail = 0;
    	fifo->usage = 0;
    	fifo->buf = (vfifo_primitive_t*)buf;
    	fifo->size = !buf ? 0 : slots;
    	fifo->free = slots > 0 ? slots-1 : 0;
    }
}

/**
  * @brief 	puts data into a fifo.
  * @param	fifo is a pointer to a fifo structure.
  * @param	data is a pointer to a data value to be inserted into the fifo.
  * @retval	returns false if the fifo was full, true otherwise.
  */
bool vfifo_put(vfifo_t* fifo, const void* data)
{
	if(fifo->size > 0)
	{
		int32_t next = (fifo->head + 1) % fifo->size;
		if(next == fifo->tail)
			return false;
		fifo->buf[fifo->head] = *(vfifo_primitive_t*)data;
		fifo->head = next;
		fifo->free--;
		fifo->usage++;
		return true;
	}
	return false;
}

/**
  * @brief 	gets data out of a fifo.
  * @param	fifo is a pointer to a fifo structure.
  * @param	data is a pointer to the destination memory.
  * @retval	returns false if the fifo was empty, true otherwise.
  */
bool vfifo_get(vfifo_t* fifo, void* data)
{
	if(fifo->size > 0)
	{
		if(fifo->head == fifo->tail)
			return false;

		*(vfifo_primitive_t*)data = fifo->buf[fifo->tail];
		fifo->tail = (fifo->tail + 1) % fifo->size;
		fifo->free++;
		fifo->usage--;
		return true;
	}
	return false;
}

int32_t vfifo_put_block(vfifo_t* fifo, const void* data, int32_t size)
{
	int32_t i = 0;
	const vfifo_primitive_t* d = data;

	if(fifo->size > 0)
	{
		while(i < size) {
			int32_t next = (fifo->head + 1) % fifo->size;
			if(next == fifo->tail)
				break;
			fifo->buf[fifo->head] = *d;
			fifo->head = next;
			fifo->free--;
			fifo->usage++;
			i++;
			d++;
		}
	}
	return i;
}

int32_t vfifo_get_block(vfifo_t* fifo, void* data, int32_t size)
{
	int32_t i = 0;
	vfifo_primitive_t* d = data;

	if(fifo->size > 0)
	{
		while(i < size) {
			if(fifo->head == fifo->tail)
				break;
			*d = fifo->buf[fifo->tail];
			fifo->tail = (fifo->tail + 1) % fifo->size;
			fifo->free++;
			fifo->usage--;
			i++;
			d++;
		}
	}
	return i;
}

/**
  * @brief	returns the total number of locations in the fifo data space that are in use.
  * @param	fifo is a pointer to a fifo structure.
  * @retval	returns the number of used locations in the fifo.
  */
int32_t vfifo_used_slots(vfifo_t* fifo)
{
	return fifo->usage;
}

/**
  * @brief	returns the total number of locations in the fifo data space that are in free.
  * @param	fifo is a pointer to a fifo structure.
  * @retval	returns number of free locations in the fifo.
  */
int32_t vfifo_free_slots(vfifo_t* fifo)
{
	return fifo->free;
}

/**
  * @param	fifo is a pointer to a fifo structure.
  * @retval	returns the total usable size of the fifo.
  */
int32_t	vfifo_number_of_slots(vfifo_t* fifo)
{
	return fifo->size > 0 ? fifo->size - 1 : 0;
}

/**
  * @brief	resets the fifo buffer pointers, effectively emptying the fifo.
  * @param	fifo is a pointer to a fifo structure.
  */
void vfifo_reset(vfifo_t* fifo)
{
	fifo->usage = 0;
	fifo->head = 0;
	fifo->tail = 0;
	fifo->free =  fifo->size > 0 ? fifo->size - 1 : 0;
}
