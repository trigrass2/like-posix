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

#ifndef LCD_CONF_H_
#define LCD_CONF_H_

#define TOUCH_TASK_PRIORITY         1
#define TOUCH_TASK_STACK            100

// #define LCD_DRIVER_ID    LGDP4532_DEVICE_ID
// #define LCD_DRIVER_ID    S6D1121_DEVICE_ID
// #define LCD_DRIVER_ID    ILI9325_DEVICE_ID
#define LCD_DRIVER_ID       LGDP4532_DEVICE_ID
#define LCD_ROTATION        ROTATE_0_DEGREES

// address to access LCD RAM (RS = 1)
#define LCD_RAM_IO_ADDRESS 0x60020000
// address to access LCD register (RS = 0)
#define LCD_REG_IO_ADDRESS 0x60000000

#define FSMC_A16_PIN    GPIO_PIN_11
#define FSMC_A16_PORT   GPIOD

#define FSMC_D0_PIN     GPIO_PIN_14
#define FSMC_D0_PORT    GPIOD
#define FSMC_D1_PIN     GPIO_PIN_15
#define FSMC_D1_PORT    GPIOD
#define FSMC_D2_PIN     GPIO_PIN_0
#define FSMC_D2_PORT    GPIOD
#define FSMC_D3_PIN     GPIO_PIN_1
#define FSMC_D3_PORT    GPIOD

#define FSMC_D4_PIN     GPIO_PIN_7
#define FSMC_D4_PORT    GPIOE
#define FSMC_D5_PIN     GPIO_PIN_8
#define FSMC_D5_PORT    GPIOE
#define FSMC_D6_PIN     GPIO_PIN_9
#define FSMC_D6_PORT    GPIOE
#define FSMC_D7_PIN     GPIO_PIN_10
#define FSMC_D7_PORT    GPIOE
#define FSMC_D8_PIN     GPIO_PIN_11
#define FSMC_D8_PORT    GPIOE
#define FSMC_D9_PIN     GPIO_PIN_12
#define FSMC_D9_PORT    GPIOE
#define FSMC_D10_PIN    GPIO_PIN_13
#define FSMC_D10_PORT   GPIOE
#define FSMC_D11_PIN    GPIO_PIN_14
#define FSMC_D11_PORT   GPIOE
#define FSMC_D12_PIN    GPIO_PIN_15
#define FSMC_D12_PORT   GPIOE

#define FSMC_D13_PIN    GPIO_PIN_8
#define FSMC_D13_PORT   GPIOD
#define FSMC_D14_PIN    GPIO_PIN_9
#define FSMC_D14_PORT   GPIOD
#define FSMC_D15_PIN    GPIO_PIN_10
#define FSMC_D15_PORT   GPIOD

#define FSMC_RD_PIN    GPIO_PIN_4
#define FSMC_RD_PORT   GPIOD
#define FSMC_WR_PIN    GPIO_PIN_5
#define FSMC_WR_PORT   GPIOD
#define FSMC_NE1_NCE2_PIN    GPIO_PIN_7
#define FSMC_NE1_NCE2_PORT   GPIOD

#define FSMC_PINS  {FSMC_D0_PIN, FSMC_D1_PIN, FSMC_D2_PIN, FSMC_D3_PIN, FSMC_D4_PIN, FSMC_D5_PIN, FSMC_D6_PIN, FSMC_D7_PIN, FSMC_D8_PIN, FSMC_D9_PIN, FSMC_D10_PIN, FSMC_D11_PIN, FSMC_D12_PIN, FSMC_D13_PIN, FSMC_D14_PIN, FSMC_D15_PIN, FSMC_A16_PIN, FSMC_RD_PIN, FSMC_WR_PIN, FSMC_NE1_NCE2_PIN}
#define FSMC_PORTS {FSMC_D0_PORT, FSMC_D1_PORT, FSMC_D2_PORT, FSMC_D3_PORT, FSMC_D4_PORT, FSMC_D5_PORT, FSMC_D6_PORT, FSMC_D7_PORT, FSMC_D8_PORT, FSMC_D9_PORT, FSMC_D10_PORT, FSMC_D11_PORT, FSMC_D12_PORT, FSMC_D13_PORT, FSMC_D14_PORT, FSMC_D15_PORT, FSMC_A16_PORT, FSMC_RD_PORT, FSMC_WR_PORT, FSMC_NE1_NCE2_PORT}

#define LCD_NRST_PIN    GPIO_PIN_1
#define LCD_NRST_PORT   GPIOE

//#define LCD_BL_PIN    GPIO_PIN_12
//#define LCD_BL_PORT   GPIOD

#endif  //LCD_CONF_H_
