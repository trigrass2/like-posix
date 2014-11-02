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
 * This file is part of the stm32-device-support project, <https://github.com/drmetal/stm32-device-support>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */


#ifndef LCD_CONF_H_
#define LCD_CONF_H_


#define TOUCH_TASK_PRIORITY         1
#define TOUCH_TASK_STACK         	0


// TODO this has no effect on touch screen rotation
// #define LCD_DRIVER_ID   S6D1121_DEVICE_ID
#define LCD_DRIVER_ID   ILI9325_DEVICE_ID
// #define LCD_DRIVER_ID LGDP4532_DEVICE_ID
#define LCD_ROTATION    ROTATE_0_DEGREES

// address to access LCD RAM -  register select pin
#define LCD_RAM_IO_ADDRESS 0x60020000 					// RS = 1
#define LCD_REG_IO_ADDRESS 0x60000000 					// RS = 0
#define LCD_RS_ADDR_PIN    GPIO_Pin_11	  				// A16 pin
#define LCD_RS_ADDR_PINSOURCE    GPIO_PinSource11
#define LCD_RS_ADDR_PORT   GPIOD

#define LCD_NRST_PIN    GPIO_Pin_1
#define LCD_NRST_PORT   GPIOE

#define LCD_BL_PIN    GPIO_Pin_12
#define LCD_BL_PORT   GPIOD

#endif  //LCD_CONF_H_
