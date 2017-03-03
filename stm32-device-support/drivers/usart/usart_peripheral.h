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

#include <stddef.h>
#include "board_config.h"
#include "base_usart.h"
#include "syscalls.h"
#include "vfifo.h"

#ifndef USART_PERIPHERAL_H_
#define USART_PERIPHERAL_H_


typedef enum {USART_FULLDUPLEX, USART_ONEWIRE} usart_mode_t;

typedef enum {
	USART1_HANDLE = 0,
	USART2_HANDLE,
	USART3_HANDLE,
	UART4_HANDLE,
	UART5_HANDLE,
	USART6_HANDLE,
	USART_INVALID_HANDLE,
}USART_HANDLE_t;

typedef struct {
	USART_TypeDef* usart;
	uint32_t baudrate;
	usart_mode_t mode;
	bool sending;
	vfifo_t* rxfifo;
	vfifo_t* txfifo;
} usart_ioctl_t;



USART_HANDLE_t usart_init_device(USART_TypeDef* usart, bool enable, usart_mode_t mode, uint32_t baudrate);
void usart_init_gpio(USART_HANDLE_t usarth);
void usart_init_interrupt(USART_HANDLE_t usarth, uint8_t priority, bool enable);
inline void usart_enable_rx_int(USART_HANDLE_t usarth);
inline void usart_enable_tx_int(USART_HANDLE_t usarth);
inline void usart_disable_rx_int(USART_HANDLE_t usarth);
inline void usart_disable_tx_int(USART_HANDLE_t usarth);
USART_HANDLE_t get_usart_handle(USART_TypeDef* usart);
USART_TypeDef* get_usart_peripheral(USART_HANDLE_t usarth);
usart_ioctl_t* get_usart_ioctl(USART_HANDLE_t usarth);
void usart_tx(USART_HANDLE_t usarth, const uint8_t data);
uint8_t usart_rx(USART_HANDLE_t usarth);
void usart_set_baudrate(USART_HANDLE_t usarth, uint32_t baudrate);
uint32_t usart_get_baudrate(USART_HANDLE_t usarth);


#endif // USART_PERIPHERAL_H_

/**
 * @}
 */
