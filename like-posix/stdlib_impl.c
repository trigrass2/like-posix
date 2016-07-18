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
 * @addtogroup syscalls System Calls
 *
 *
 * The implementation of _realloc_r was taken from Stefano Oliveri's syscalls_minimal.c
 *
 * relies upon:
 * - FreeRTOS
 *
 * @file stdlib_impl.c
 * @{
 */

#include "FreeRTOS.h"
#include <string.h>


/**
 *  definition of block structure, copied from heap2 allocator
 */
typedef struct A_BLOCK_LINK
{
	struct A_BLOCK_LINK *pxNextFreeBlock;	///< The next free block in the list
	size_t xBlockSize;						///< The size of the free block
} xBlockLink;

static const unsigned short heapSTRUCT_SIZE	= (sizeof(xBlockLink)+portBYTE_ALIGNMENT-(sizeof(xBlockLink)%portBYTE_ALIGNMENT));


_PTR _realloc_r(struct _reent *re, _PTR oldAddr, size_t newSize)
{
	(void)re;

	xBlockLink *block;
	size_t toCopy;
	void *newAddr;

	newAddr = pvPortMalloc(newSize);

	if (newAddr == NULL)
		return NULL;

	/* We need the block struct pointer to get the current size */
	block = oldAddr;
	block -= heapSTRUCT_SIZE;

	/* determine the size to be copied */
	toCopy = (newSize<block->xBlockSize)?(newSize):(block->xBlockSize);

	/* copy old block into new one */
	memcpy((void *)newAddr, (void *)oldAddr, (size_t)toCopy);

	vPortFree(oldAddr);

	return newAddr;
}

_PTR _calloc_r(struct _reent *re, size_t num, size_t size) {
	(void)re;
	size *= num;
    _PTR m = pvPortMalloc(size);
    if(m)
        memset(m, 0, size);
    return m;
}

_PTR _malloc_r(struct _reent *re, size_t size) {
	(void)re;
	return pvPortMalloc(size);
}

_VOID _free_r(struct _reent *re, _PTR ptr) {
	(void)re;
	vPortFree(ptr);
}

/**
 * @}
 */
