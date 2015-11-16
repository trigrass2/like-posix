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
 * This file is part of the Appleseed project, <https://github.com/drmetal/app-l-seed>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */
#include "board_config.h"
#include "lcd.h"
#include "touch_panel.h"
#include "lcd_backlight.h"
#include "../../board/uemb4.bsp/lcd_config.h"

/**
 * initializes lcd backlight pin.
 */
void lcd_backlight_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
#if FAMILY==STM32F1
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
#elif FAMILY==STM32F4
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
#endif

    // Backlight pin
#ifdef LCD_BL_PIN
    GPIO_InitStructure.GPIO_Pin = LCD_BL_PIN;
    GPIO_Init(LCD_BL_PORT, &GPIO_InitStructure);
#endif
}

/**
 * enable backlight off on inactivity timeout.
 */
void lcd_backlight_auto_off(bool enable)
{
    if(enable)
        touch_panel_set_activity_callbacks(lcd_backlight_enable, lcd_backlight_disable);
    else
        touch_panel_set_activity_callbacks(NULL, NULL);
}

/**
 * sets backlight off on inactivity timeout period in milliseconds.
 */
void lcd_backlight_timeout(int timeout)
{
    touch_panel_set_inactivity_timeout(timeout/TOUCH_TASK_POLL_RATE);
}

/**
 * turns the backight off.
 */
void lcd_backlight_disable()
{
    lcd_backlight(false);
}

/**
 * turns the backight on.
 */
void lcd_backlight_enable()
{
    lcd_backlight(true);
}

/**
 * turns the backight on or off.
 */
void lcd_backlight(bool enable)
{
#ifdef LCD_BL_PIN
    if(enable)
        GPIO_SetBits(LCD_BL_PORT, LCD_BL_PIN);
    else
        GPIO_ResetBits(LCD_BL_PORT, LCD_BL_PIN);
#else
    (void)enable;
#endif
}
