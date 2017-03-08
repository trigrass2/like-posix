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


#include <stddef.h>
#include "base_spi.h"
#include "syscalls.h"
#include "vfifo.h"

#ifndef SPI_PERIPHERAL_H_
#define SPI_PERIPHERAL_H_

typedef enum {
	SPI1_HANDLE = 0,
	SPI2_HANDLE,
	SPI3_HANDLE,
	SPI_INVALID_HANDLE,
}SPI_HANDLE_t;

typedef struct {
    SPI_TypeDef* spi;
	uint32_t baudrate;
	uint32_t bit_order;
	uint32_t clock_phase;
	uint32_t clock_polarity;
	uint32_t data_width;
	bool sending;
	vfifo_t* rxfifo;
	vfifo_t* txfifo;
#if USE_FREERTOS
	SemaphoreHandle_t rx_sem;
	int32_t rx_expect;
#endif
} spi_ioctl_t;

SPI_HANDLE_t spi_init_device(SPI_TypeDef* spi, bool enable, uint32_t baudrate, uint32_t bit_order, uint32_t clock_phase, uint32_t clock_polarity, uint32_t data_width);
void spi_init_gpio(SPI_HANDLE_t spih);
void spi_init_interrupt(SPI_HANDLE_t spih, uint8_t priority, bool enable);

SPI_HANDLE_t get_spi_handle(SPI_TypeDef* spi);
SPI_TypeDef* get_spi_peripheral(SPI_HANDLE_t spih);
spi_ioctl_t* get_spi_ioctl(SPI_HANDLE_t spih);

void spi_set_prescaler(SPI_HANDLE_t spih, uint16_t presc);
void spi_set_baudrate(SPI_HANDLE_t spih, uint32_t baudrate);
uint32_t spi_get_baudrate(SPI_HANDLE_t spih);

inline bool spi_rx_inwaiting(SPI_HANDLE_t spih);
inline bool spi_tx_readytosend(SPI_HANDLE_t spih);
inline void spi_enable_rx_int(SPI_HANDLE_t spih);
inline void spi_enable_tx_int(SPI_HANDLE_t spih);
inline void spi_disable_rx_int(SPI_HANDLE_t spih);
inline void spi_disable_tx_int(SPI_HANDLE_t spih);

void spi_tx(SPI_HANDLE_t spih, const uint8_t data);
uint8_t spi_rx(SPI_HANDLE_t spih);

#endif // SPI_PERIPHERAL_H_
