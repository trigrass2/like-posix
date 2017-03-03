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
 * based heavily on the code by Zizzle at: https://github.com/Zizzle/stm32_freertos_example/tree/master/drivers
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

#include "tsc2046.h"
#include "lcd.h"
#include "spi.h"


#define TSC2046_START  0x80
#define TSC2046_CH_X   0x50
#define TSC2046_CH_Y   0x10
#define TSC2046_MAX_X  4096
#define TSC2046_MAX_Y  4096

#if TOUCH_PANEL_ROTATION == ROTATE_0_DEGREES

#pragma message "building TSC2046 touch panel rotated by 0 degrees"

#define TSC2046_PIX_X           LCD_WIDTH
#define TSC2046_X_OFFSET        0
#define TSC2046_PIX_Y           LCD_HEIGHT
#define TSC2046_Y_OFFSET        0

#define TSC2046_READ_X          (TSC2046_START | TSC2046_CH_X)
#define TSC2046_READ_Y          (TSC2046_START | TSC2046_CH_Y)

#elif TOUCH_PANEL_ROTATION == ROTATE_90_DEGREES

#pragma message "building TSC2046 touch panel rotated by 90 degrees"

#define TSC2046_PIX_X           LCD_WIDTH
#define TSC2046_X_OFFSET        0
#define TSC2046_PIX_Y           -LCD_HEIGHT
#define TSC2046_Y_OFFSET        LCD_HEIGHT-1

#define TSC2046_READ_X          (TSC2046_START | TSC2046_CH_Y)
#define TSC2046_READ_Y          (TSC2046_START | TSC2046_CH_X)

#elif TOUCH_PANEL_ROTATION == ROTATE_180_DEGREES

#pragma message "building TSC2046 touch panel rotated by 180 degrees"

#define TSC2046_PIX_X           -LCD_WIDTH
#define TSC2046_X_OFFSET        LCD_WIDTH-1
#define TSC2046_PIX_Y           -LCD_HEIGHT
#define TSC2046_Y_OFFSET        LCD_HEIGHT-1

#define TSC2046_READ_X          (TSC2046_START | TSC2046_CH_X)
#define TSC2046_READ_Y          (TSC2046_START | TSC2046_CH_Y)

#elif TOUCH_PANEL_ROTATION == ROTATE_270_DEGREES

#pragma message "building TSC2046 touch panel rotated by 270 degrees"

#define TSC2046_PIX_X           -LCD_WIDTH
#define TSC2046_X_OFFSET        LCD_WIDTH-1
#define TSC2046_PIX_Y           LCD_HEIGHT
#define TSC2046_Y_OFFSET        0

#define TSC2046_READ_X          (TSC2046_START | TSC2046_CH_Y)
#define TSC2046_READ_Y          (TSC2046_START | TSC2046_CH_X)

#endif


#define tsc2046_txrx(data)      spi_transfer(TSC2046_SPI_PERIPH, data)
#define tsc2046_select()        spi_assert_nss(TSC2046_SPI_PERIPH)
#define tsc2046_deselect()      spi_deassert_nss(TSC2046_SPI_PERIPH)

void tsc2046_init()
{
    assert_true(spi_init(TSC2046_SPI_PERIPH, NULL, true, 0, SPI_FIRSTBIT_MSB, SPI_PHASE_1EDGE, SPI_POLARITY_LOW, SPI_DATASIZE_8BIT));
    spi_set_prescaler(TSC2046_SPI_PERIPH, TSC2046_SPI_PRESC);

    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
//    GPIO_InitStructure.Pin = TSC2046_BUSY_PIN;
//    HAL_GPIO_Init(TSC2046_BUSY_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Pin = TSC2046_IRQ_PIN;
    HAL_GPIO_Init(TSC2046_IRQ_PORT, &GPIO_InitStructure);
}


uint16_t tsc2046_x()
{
    uint16_t x;
    tsc2046_select();
    tsc2046_txrx(TSC2046_READ_X);
    x = ((tsc2046_txrx(0) & 0x7F) << 8);
    x |= tsc2046_txrx(0);
    x >>= 3;
    x = ((x * TSC2046_PIX_X) / TSC2046_MAX_X) + TSC2046_X_OFFSET;
    tsc2046_deselect();
    return x;
}

uint16_t tsc2046_y()
{
    uint16_t y;
    tsc2046_select();
    tsc2046_txrx(TSC2046_READ_Y);
    y = ((tsc2046_txrx(0) & 0x7F) << 8);
    y |= tsc2046_txrx(0);
    y >>= 3;
    y = ((y * TSC2046_PIX_Y) / TSC2046_MAX_Y) + TSC2046_Y_OFFSET;
    tsc2046_deselect();
    return y;
}
