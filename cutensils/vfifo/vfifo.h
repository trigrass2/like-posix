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


#ifndef VFIFO_H_
#define VFIFO_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

/**
 * define VFIFO_PRIMITIVE externally to compile for a custom type.
 */
#ifndef VFIFO_PRIMITIVE
#define VFIFO_PRIMITIVE uint8_t
#endif

typedef VFIFO_PRIMITIVE vfifo_primitive_t;

/**
 * The VFIFO data type, defines one n-bit FIFO where n may be any multiple of 8.
 * Use @ref vfifo_init to create this object.
 *
 * have set volatile on the members...
 * when compiled using gcc-arm-none-eabi-5_4-2016q3 and running on stm32f407v
 * with -O2.
 * a hardfault occurs when accessed from an ISR, even if the owning structure is volatile.
 */
typedef struct {
   volatile int32_t head;			///< the FIFO head position indicator
   volatile int32_t tail;			///< the FIFO tail position indicator
   volatile vfifo_primitive_t* buf;	///< a pointer to the FIFO buffer data space
   volatile int32_t size;			///< the size in words of the FIFO
   volatile int32_t free;			///< the number of free spaces in the FIFO buffer
   volatile int32_t usage;			///< the number of filled spaces the FIFO buffer
} vfifo_t;

void vfifo_init(vfifo_t* fifo, void* buf, int32_t size);
bool vfifo_put(vfifo_t* fifo, const void* data);
bool vfifo_get(vfifo_t* fifo, void* data);
int32_t vfifo_put_block(vfifo_t* fifo, const void* data, int32_t size);
int32_t vfifo_get_block(vfifo_t* fifo, void* data, int32_t size);
int32_t vfifo_used_slots(vfifo_t* fifo);
int32_t	vfifo_free_slots(vfifo_t* fifo);
int32_t	vfifo_number_of_slots(vfifo_t* fifo);
bool vfifo_full(vfifo_t* fifo);
void	vfifo_reset(vfifo_t* fifo);

#endif /* VFIFO_H_ */
