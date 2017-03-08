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

#include <stdint.h>
#include <stdbool.h>
#include "spi_peripheral.h"

#ifndef SPI_H_
#define SPI_H_

SPI_HANDLE_t spi_create_polled(SPI_TypeDef* spi, bool enable, uint32_t baudrate, uint32_t bit_order, uint32_t clock_phase, uint32_t clock_polarity, uint32_t data_width);
uint8_t spi_transfer_polled(SPI_HANDLE_t spih, uint8_t data);

SPI_HANDLE_t spi_create_async(SPI_TypeDef* spi, bool enable, uint32_t baudrate, uint32_t bit_order, uint32_t clock_phase, uint32_t clock_polarity, uint32_t data_width, uint32_t buffersize);
int32_t spi_put_async(SPI_HANDLE_t spih, const uint8_t* data, int32_t length);
int32_t spi_get_async(SPI_HANDLE_t spih, uint8_t* data, int32_t length, uint32_t timeout);


void spi_init_ss_gpio(SPI_HANDLE_t spi);
void spi_clear_ss(SPI_HANDLE_t spi);
void spi_set_ss(SPI_HANDLE_t spi);

#if USE_LIKEPOSIX
SPI_HANDLE_t spi_create_dev(char* filename, SPI_TypeDef* spi, bool enable, uint32_t baudrate, uint32_t bit_order, uint32_t clock_phase, uint32_t clock_polarity, uint32_t data_width);
#endif

inline bool _spi_tx_isr(SPI_HANDLE_t spih);
inline bool _spi_rx_isr(SPI_HANDLE_t spih);

#endif /* SPI_H_ */
