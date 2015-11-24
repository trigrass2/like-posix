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
 * This file is part of the Appleseed project, <https://github.com/drmetal/appleseed>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */


#ifndef TSC2046_CONFIG_H_
#define TSC2046_CONFIG_H_

#include "lcd_config.h"

#define TOUCH_PANEL_ROTATION    LCD_ROTATION

#define TSC2046_SPI_PERIPH  SPI2
#define TSC2046_SPI_CLOCK   RCC_APB1Periph_SPI2
#define TSC2046_SPI_PRESC   SPI_BaudRatePrescaler_16    // SPI2@84MHz / 16 = 5.25MHz ~ 5MHz max
#define TSC2046_AF_CONFIG   GPIO_AF_SPI2

#define TSC2046_NCS_PORT    GPIOB
#define TSC2046_NCS_PIN     GPIO_Pin_12
#define TSC2046_IRQ_PORT    GPIOD
#define TSC2046_IRQ_PIN     GPIO_Pin_13

#define TSC2046_MISO_PORT   GPIOB
#define TSC2046_MISO_PIN    GPIO_Pin_14
#define TSC2046_MISO_PINSOURCE	GPIO_PinSource14
#define TSC2046_MOSI_PORT   GPIOB
#define TSC2046_MOSI_PIN    GPIO_Pin_15
#define TSC2046_MOSI_PINSOURCE	GPIO_PinSource15
#define TSC2046_SCK_PORT    GPIOB
#define TSC2046_SCK_PIN     GPIO_Pin_13
#define TSC2046_SCK_PINSOURCE	GPIO_PinSource13


#define TSC2046_FLIP_Y 		1
#define TSC2046_FLIP_X 		0

#endif // TSC2046_CONFIG_H_
