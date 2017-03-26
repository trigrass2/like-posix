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

#define TSC2046_TRANSEFR_BYTES 6
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

TSC2046_t tsc2046_init()
{
#if TSC2046_POLLED_SPI
	TSC2046_t tsc2046 = spi_create_polled(TSC2046_SPI_PERIPH, true, SPI_FIRSTBIT_MSB, SPI_PHASE_1EDGE, SPI_POLARITY_LOW, SPI_DATASIZE_8BIT, TSC2046_SPI_BAUDRATE);
#else
	TSC2046_t tsc2046 = spi_create_async(TSC2046_SPI_PERIPH, true, SPI_FIRSTBIT_MSB, SPI_PHASE_1EDGE, SPI_POLARITY_LOW, SPI_DATASIZE_8BIT, TSC2046_SPI_BAUDRATE, TSC2046_TRANSEFR_BYTES);
#endif

    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Pin = TSC2046_NIRQ_PIN;
    HAL_GPIO_Init(TSC2046_NIRQ_PORT, &GPIO_InitStructure);

    return tsc2046;
}


bool tsc2046_ready()
{
	return HAL_GPIO_ReadPin(TSC2046_NIRQ_PORT, TSC2046_NIRQ_PIN) != GPIO_PIN_SET;
}

void tsc2046_read(TSC2046_t tsc2046, int16_t* x, int16_t* y)
{
	int16_t _x;
	int16_t _y;
    spi_clear_ss(tsc2046);
#if TSC2046_POLLED_SPI
    spi_transfer_polled(tsc2046, TSC2046_READ_X);
    _x = ((spi_transfer_polled(tsc2046, 0) & 0x7F) << 8);
    _x |= spi_transfer_polled(tsc2046, 0);
    _x >>= 3;
    _x = ((_x * TSC2046_PIX_X) / TSC2046_MAX_X) + TSC2046_X_OFFSET;
    spi_transfer_polled(tsc2046, TSC2046_READ_Y);
    _y = ((spi_transfer_polled(tsc2046, 0) & 0x7F) << 8);
    _y |= spi_transfer_polled(tsc2046, 0);
    _y >>= 3;
    _y = ((_y * TSC2046_PIX_Y) / TSC2046_MAX_Y) + TSC2046_Y_OFFSET;
#else
    uint8_t buffer[6] = {TSC2046_READ_X, 0,0, TSC2046_READ_Y, 0, 0};
    spi_put_async(tsc2046, buffer, sizeof(buffer));
    spi_get_async(tsc2046, buffer, sizeof(buffer), 1000);
    _x = (((buffer[1] & 0x7F) << 8) | buffer[2]) >> 3;
    _x = ((_x * TSC2046_PIX_X) / TSC2046_MAX_X) + TSC2046_X_OFFSET;
    _y = (((buffer[4] & 0x7F) << 8) | buffer[5]) >> 3;
    _y = ((_y * TSC2046_PIX_Y) / TSC2046_MAX_Y) + TSC2046_Y_OFFSET;
#endif
    spi_set_ss(tsc2046);

    if(x) {
    	*x = _x;
    }
    if(y) {
    	*y = _y;
    }
}
