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
 * @addtogroup usart
 * @{
 *
 * @file    usart.h
 */

#include <stdint.h>
#include <stdbool.h>

#include "usart_config.h"
#include "usart_it.h"

#ifndef USART_H_
#define USART_H_

/**
 * TODO add other USART modes
 */
typedef enum {USART_FULLDUPLEX, USART_ONEWIRE} usart_mode_t;

typedef struct {
	USART_TypeDef* usart;
	usart_mode_t mode;
}usart_ioctl_t;


bool usart_init(USART_TypeDef* usart, char* install_as, bool enable, usart_mode_t mode);

void usart_init_device(USART_TypeDef* usart, bool enable, usart_mode_t mode);
void usart_init_gpio(USART_TypeDef* usart, usart_mode_t mode);
void usart_init_interrupt(USART_TypeDef* device, uint8_t priority, bool enable);

void usart_set_baudrate(USART_TypeDef* usart, uint32_t br);
uint32_t usart_get_baudrate(USART_TypeDef* usart);

void set_console_usart(USART_TypeDef* usart);
char phy_getc(void);
void phy_putc(char c);

#endif // USART_H_

/**
 * @}
 */
