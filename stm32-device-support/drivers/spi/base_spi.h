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

#ifndef BASE_SPI_H_
#define BASE_SPI_H_

#include "spi_config.h"

#define NUM_ONCHIP_SPIS 3

/**
 * in @ref board_SPI.h set SPI1_FULL_REMAP to 1 to enable remap, otherwise set to 0.
 */
#if SPI1_FULL_REMAP
#pragma message("SPI1 used in full remap mode")
#define SPI1_REMAP GPIO_Remap_SPI1
#define SPI1_PORT GPIOB
#define SPI1_SCK_PIN GPIO_PIN_3
#define SPI1_MISO_PIN GPIO_PIN_4
#define SPI1_MOSI_PIN GPIO_PIN_5
#else
#pragma message("SPI1 used in non remap mode")
#define SPI1_PORT GPIOA
#define SPI1_SCK_PIN GPIO_PIN_5
#define SPI1_MISO_PIN GPIO_PIN_6
#define SPI1_MOSI_PIN GPIO_PIN_7
#endif

#pragma message("SPI2 used in non remap mode")
#define SPI2_PORT GPIOB
#define SPI2_SCK_PIN GPIO_PIN_13
#define SPI2_MISO_PIN GPIO_PIN_14
#define SPI2_MOSI_PIN GPIO_PIN_15

/**
 * in @ref board_SPI.h set SPI3_FULL_REMAP to 1 to enable remap, otherwise set to 0.
 */

#if SPI3_FULL_REMAP
#pragma message("SPI3 used in full remap mode")
#define SPI3_REMAP GPIO_Remap_SPI3
#define SPI3_PORT GPIOC
#define SPI3_SCK_PIN GPIO_PIN_10
#define SPI3_MISO_PIN GPIO_PIN_11
#define SPI3_MOSI_PIN GPIO_PIN_12
#else
#pragma message("SPI3 used in non remap mode")
#define SPI3_PORT GPIOB
#define SPI3_SCK_PIN GPIO_PIN_3
#define SPI3_MISO_PIN GPIO_PIN_4
#define SPI3_MOSI_PIN GPIO_PIN_5
#endif

#endif /* BASE_SPI_H_ */
