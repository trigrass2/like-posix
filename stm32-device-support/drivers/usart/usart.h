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
 * @addtogroup usart
 * @{
 *
 * @file    usart.h
 */

#include <stdint.h>
#include <stdbool.h>
#include "usart_peripheral.h"

#ifndef USART_H_
#define USART_H_

USART_HANDLE_t usart_create_polled(USART_TypeDef* usart, bool enable, usart_mode_t mode, uint32_t baudrate);
void usart_set_stdio_usart(int usarth);
void usart_stdio_tx(const char data);
char usart_stdio_rx();

USART_HANDLE_t usart_create_async(USART_TypeDef* usart, bool enable, usart_mode_t mode, uint32_t baudrate, uint32_t buffersize);
int32_t usart_put_async(USART_HANDLE_t usarth, const uint8_t* data, int32_t length);
int32_t usart_get_async(USART_HANDLE_t usarth, uint8_t* data, int32_t length, uint32_t timeout);

#if USE_LIKEPOSIX
USART_HANDLE_t usart_create_dev(char* filename, USART_TypeDef* usart, bool enable, usart_mode_t mode, uint32_t baudrate, uint32_t buffersize);
#endif

inline bool _usart_rx_isr(USART_HANDLE_t usarth);
inline bool _usart_tx_isr(USART_HANDLE_t usarth);
#endif // USART_H_

/**
 * @}
 */
