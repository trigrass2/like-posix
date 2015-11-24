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
 * based heavily on the code by Zizzle at: https://github.com/Zizzle/stm32_freertos_example/tree/master/drivers
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

/**
 * Compatible list:
 * ILI9320 - specify ILI9320_DEVICE_ID
 * ILI9325 - specify ILI9325_DEVICE_ID
 * LGDP4531 - specify LGDP4531_DEVICE_ID
 */

#include <stdio.h>
#include <stdint.h>
#include "lcd_config.h"

#include "board_config.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"

#include "ili932x.h"
#include "system.h"


#ifndef LCD_H_
#define LCD_H_

/**
 * display rotation
 */
#define ROTATE_0_DEGREES                    1
#define ROTATE_90_DEGREES                   2
#define ROTATE_180_DEGREES                  3
#define ROTATE_270_DEGREES                  4


#ifndef LCD_DRIVER_ID
// defined in lcd_config.h
// #define LCD_DRIVER_ID ILI9320_DEVICE_ID
#endif
#ifndef LCD_ROTATION
// defined in lcd_config.h
// #define LCD_ROTATION  ROTATE_0_DEGREES
#endif

#define LCD_RED_MASK	0xF800
#define LCD_GREEN_MASK	0x07E0
#define LCD_BLUE_MASK	0x001F

#define LCD_RED_MAX		32
#define LCD_GREEN_MAX	64
#define LCD_BLUE_MAX	32

#define LCD_RED_SHIFT	11
#define LCD_GREEN_SHIFT	5
#define LCD_BLUE_SHIFT	0


/**
 * returns the value of the individual colour words given a combined colour word.
 */
#define LCD_RED_BYTE(colour)                ((colour & LCD_RED_MASK) >> LCD_RED_SHIFT)
#define LCD_GREEN_BYTE(colour)              ((colour & LCD_GREEN_MASK) >> LCD_GREEN_SHIFT)
#define LCD_BLUE_BYTE(colour)               (colour & LCD_BLUE_MASK)

 /**
  * inserts colour data back into a colour word.
  */
#define LCD_INSERT_RED_BYTE(colour, red)    (colour |= red << LCD_RED_SHIFT)
#define LCD_INSERT_GREEN_BYTE(colour, green)    (colour |= green << LCD_GREEN_SHIFT)
#define LCD_INSERT_BLUE_BYTE(colour, blue)  (colour |= blue)


#if LCD_DRIVER_ID == LGDP4532_DEVICE_ID

#if LCD_ROTATION == ROTATE_0_DEGREES
#define LCD_WIDTH                           240
#define LCD_HEIGHT                          320
#define LCD_Y_SCAN_CONTROL_REG_VALUE        0x00
#define LCD_X_SCAN_CONTROL_REG_VALUE        ILI932x_GATE_SCAN_CTRL1_GS
#define LCD_ENTRYMODE_AM_VALUE              0x00
#define LCD_SET_CURSOR_X                    ILI932x_H_GRAM_ADDR_SET
#define LCD_SET_CURSOR_Y                    ILI932x_V_GRAM_ADDR_SET
#define LCD_SET_WINDOW_X1                   ILI932x_HSA
#define LCD_SET_WINDOW_Y1                   ILI932x_VSA
#define LCD_SET_WINDOW_X2                   ILI932x_HEA
#define LCD_SET_WINDOW_Y2                   ILI932x_VEA
#elif LCD_ROTATION == ROTATE_90_DEGREES
#define LCD_WIDTH                           320
#define LCD_HEIGHT                          240
#define LCD_Y_SCAN_CONTROL_REG_VALUE        ILI932x_DRIVER_OUTPUT_CTRL_SS
#define LCD_X_SCAN_CONTROL_REG_VALUE        ILI932x_GATE_SCAN_CTRL1_GS
#define LCD_ENTRYMODE_AM_VALUE              ILI932x_EM_AM
#define LCD_SET_CURSOR_X                    ILI932x_V_GRAM_ADDR_SET
#define LCD_SET_CURSOR_Y                    ILI932x_H_GRAM_ADDR_SET
#define LCD_SET_WINDOW_X1                   ILI932x_VSA
#define LCD_SET_WINDOW_Y1                   ILI932x_HSA
#define LCD_SET_WINDOW_X2                   ILI932x_VEA
#define LCD_SET_WINDOW_Y2                   ILI932x_HEA
#elif LCD_ROTATION == ROTATE_180_DEGREES
#define LCD_WIDTH                           240
#define LCD_HEIGHT                          320
#define LCD_Y_SCAN_CONTROL_REG_VALUE        ILI932x_DRIVER_OUTPUT_CTRL_SS
#define LCD_X_SCAN_CONTROL_REG_VALUE        0x00
#define LCD_ENTRYMODE_AM_VALUE              0x00
#define LCD_SET_CURSOR_X                    ILI932x_H_GRAM_ADDR_SET
#define LCD_SET_CURSOR_Y                    ILI932x_V_GRAM_ADDR_SET
#define LCD_SET_WINDOW_X1                   ILI932x_HSA
#define LCD_SET_WINDOW_Y1                   ILI932x_VSA
#define LCD_SET_WINDOW_X2                   ILI932x_HEA
#define LCD_SET_WINDOW_Y2                   ILI932x_VEA
#elif LCD_ROTATION == ROTATE_270_DEGREES
#define LCD_WIDTH                           320
#define LCD_HEIGHT                          240
#define LCD_Y_SCAN_CONTROL_REG_VALUE        0x00
#define LCD_X_SCAN_CONTROL_REG_VALUE        0x00
#define LCD_ENTRYMODE_AM_VALUE              ILI932x_EM_AM
#define LCD_SET_CURSOR_X                    ILI932x_V_GRAM_ADDR_SET
#define LCD_SET_CURSOR_Y                    ILI932x_H_GRAM_ADDR_SET
#define LCD_SET_WINDOW_X1                   ILI932x_VSA
#define LCD_SET_WINDOW_Y1                   ILI932x_HSA
#define LCD_SET_WINDOW_X2                   ILI932x_VEA
#define LCD_SET_WINDOW_Y2                   ILI932x_HEA
#endif

#elif LCD_DRIVER_ID == ILI9325_DEVICE_ID

#if LCD_ROTATION == ROTATE_0_DEGREES
#define LCD_WIDTH                           240
#define LCD_HEIGHT                          320
#define LCD_Y_SCAN_CONTROL_REG_VALUE        0x00
#define LCD_X_SCAN_CONTROL_REG_VALUE        0x00
#define LCD_ENTRYMODE_AM_VALUE              0x00
#define LCD_SET_CURSOR_X                    ILI932x_H_GRAM_ADDR_SET
#define LCD_SET_CURSOR_Y                    ILI932x_V_GRAM_ADDR_SET
#define LCD_SET_WINDOW_X1                   ILI932x_HSA
#define LCD_SET_WINDOW_Y1                   ILI932x_VSA
#define LCD_SET_WINDOW_X2                   ILI932x_HEA
#define LCD_SET_WINDOW_Y2                   ILI932x_VEA
#elif LCD_ROTATION == ROTATE_90_DEGREES
#define LCD_WIDTH                           320
#define LCD_HEIGHT                          240
#define LCD_Y_SCAN_CONTROL_REG_VALUE        ILI932x_DRIVER_OUTPUT_CTRL_SS
#define LCD_X_SCAN_CONTROL_REG_VALUE        0x00
#define LCD_ENTRYMODE_AM_VALUE              ILI932x_EM_AM
#define LCD_SET_CURSOR_X                    ILI932x_V_GRAM_ADDR_SET
#define LCD_SET_CURSOR_Y                    ILI932x_H_GRAM_ADDR_SET
#define LCD_SET_WINDOW_X1                   ILI932x_VSA
#define LCD_SET_WINDOW_Y1                   ILI932x_HSA
#define LCD_SET_WINDOW_X2                   ILI932x_VEA
#define LCD_SET_WINDOW_Y2                   ILI932x_HEA
#elif LCD_ROTATION == ROTATE_180_DEGREES
#define LCD_WIDTH                           240
#define LCD_HEIGHT                          320
#define LCD_Y_SCAN_CONTROL_REG_VALUE        ILI932x_DRIVER_OUTPUT_CTRL_SS
#define LCD_X_SCAN_CONTROL_REG_VALUE        ILI932x_GATE_SCAN_CTRL1_GS
#define LCD_ENTRYMODE_AM_VALUE              0x00
#define LCD_SET_CURSOR_X                    ILI932x_H_GRAM_ADDR_SET
#define LCD_SET_CURSOR_Y                    ILI932x_V_GRAM_ADDR_SET
#define LCD_SET_WINDOW_X1                   ILI932x_HSA
#define LCD_SET_WINDOW_Y1                   ILI932x_VSA
#define LCD_SET_WINDOW_X2                   ILI932x_HEA
#define LCD_SET_WINDOW_Y2                   ILI932x_VEA
#elif LCD_ROTATION == ROTATE_270_DEGREES
#define LCD_WIDTH                           320
#define LCD_HEIGHT                          240
#define LCD_Y_SCAN_CONTROL_REG_VALUE        0x00
#define LCD_X_SCAN_CONTROL_REG_VALUE        ILI932x_GATE_SCAN_CTRL1_GS
#define LCD_ENTRYMODE_AM_VALUE              ILI932x_EM_AM
#define LCD_SET_CURSOR_X                    ILI932x_V_GRAM_ADDR_SET
#define LCD_SET_CURSOR_Y                    ILI932x_H_GRAM_ADDR_SET
#define LCD_SET_WINDOW_X1                   ILI932x_VSA
#define LCD_SET_WINDOW_Y1                   ILI932x_HSA
#define LCD_SET_WINDOW_X2                   ILI932x_VEA
#define LCD_SET_WINDOW_Y2                   ILI932x_HEA
#endif
#endif


/**
 * lcd IO macros.
 */
#define LCD_REG                             (*((volatile unsigned short *) LCD_REG_IO_ADDRESS))
#define LCD_RAM                             (*((volatile unsigned short *) LCD_RAM_IO_ADDRESS))
#define write_cmd(cmd)                      LCD_REG = cmd
#define write_data(data)                    LCD_RAM = data
#define write_reg(reg, value)               write_cmd(reg);write_data(value)
#define read_data()                         LCD_RAM
#define read_reg(reg, result)               write_cmd(reg);result = read_data()


/**
 * lcd control macros.
 */
#define lcd_set_cursor_x(x)                 write_reg(LCD_SET_CURSOR_X, x)
#define lcd_set_cursor_y(y)                 write_reg(LCD_SET_CURSOR_Y, y)
#define lcd_set_cursor(x, y)                lcd_set_cursor_y(y);lcd_set_cursor_x(x)
#define lcd_set_window(x, y, w, h)          write_reg(LCD_SET_WINDOW_X1, x); \
                                            write_reg(LCD_SET_WINDOW_Y1, y); \
                                            write_reg(LCD_SET_WINDOW_X2, x+(w-1)); \
                                            write_reg(LCD_SET_WINDOW_Y2, y+(h-1))
#define lcd_rw_gram()                       write_cmd(ILI932x_CMD_WRITE_GRAM)
#define lcd_set_entry_mode_normal()         write_reg(ILI932x_ENTRY_MODE, LCD_ENTRYMODE_AM_VALUE|ILI932x_EM_65K|ILI932x_EM_VERTICAL_INC|ILI932x_EM_HORIZONTAL_INC|ILI932x_EM_BGR); \
                                            lcd_set_window(0, 0, LCD_WIDTH, LCD_HEIGHT);
#define lcd_set_entry_mode_window()         write_reg(ILI932x_ENTRY_MODE, LCD_ENTRYMODE_AM_VALUE|ILI932x_EM_65K|ILI932x_EM_VERTICAL_INC|ILI932x_EM_HORIZONTAL_INC|ILI932x_EM_BGR|ILI932x_EM_ORG);

/**
 * lock for non atomic display operations.
 */
extern SemaphoreHandle_t xLcdMutex;
#define LCD_LOCK() assert_param(xSemaphoreTake(xLcdMutex, 1000/portTICK_RATE_MS))
#define LCD_UNLOCK() xSemaphoreGive(xLcdMutex)


void lcd_init(void);
unsigned int lcd_getdeviceid(void);


#endif // LCD_H_
