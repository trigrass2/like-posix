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


#ifndef TSC2046_CONFIG_H_
#define TSC2046_CONFIG_H_

#include "lcd_config.h"

#define TOUCH_PANEL_ROTATION    	LCD_ROTATION

#define TOUCH_I2C_PERIPH  			I2C1
#define TOUCH_I2C_CLOCK   			RCC_APB1Periph_I2C1
#define TOUCH_I2C_CONFIG   			GPIO_AF_I2C1

#define TOUCH_I2C_SCL_PORT   		GPIOB
#define TOUCH_I2C_SCL_PIN    		GPIO_PIN_8
#define TOUCH_I2C_SCL_PINSOURCE		GPIO_PinSource8
#define TOUCH_I2C_SDA_PORT   		GPIOB
#define TOUCH_I2C_SDA_PIN    		GPIO_PIN_9
#define TOUCH_I2C_SDA_PINSOURCE		GPIO_PinSource9
#define TOUCH_I2C_SDA_PORT   		GPIOB
#define TOUCH_I2C_SDA_PIN    		GPIO_PIN_9
#define TOUCH_I2C_SDA_PINSOURCE		GPIO_PinSource9
#define TOUCH_NRST_PIN    			GPIO_PIN_7
#define TOUCH_NRST_PORT   			GPIOH
#define TOUCH_INT_PIN    			GPIO_PIN_5
#define TOUCH_INT_PORT   			GPIOJ

#endif // TSC2046_CONFIG_H_
