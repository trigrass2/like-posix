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
#include "board_config.h"
#include "lcd.h"
#include "lcd_backlight.h"
#if USE_DRIVER_TOUCH_PANEL
#include "touch_panel.h"
#else
#pragma message("building LCD Backlight without touch panel support")
#endif
#include "lcd_config.h"

/**
 * initializes lcd backlight pin.
 */
void lcd_backlight_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
    GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull  = GPIO_NOPULL;

    // Backlight pin
#ifdef LCD_BL_PIN
    GPIO_InitStructure.Pin = LCD_BL_PIN;
    HAL_GPIO_Init(LCD_BL_PORT, &GPIO_InitStructure);
#endif
}

/**
 * enable backlight off on inactivity timeout.
 */
void lcd_backlight_auto_off(bool enable)
{
#if USE_DRIVER_TOUCH_PANEL
    if(enable)
        touch_panel_set_activity_callbacks(lcd_backlight_enable, lcd_backlight_disable);
    else
        touch_panel_set_activity_callbacks(NULL, NULL);
#endif
}

/**
 * sets backlight off on inactivity timeout period in milliseconds.
 */
void lcd_backlight_timeout(int timeout)
{
#if USE_DRIVER_TOUCH_PANEL
    touch_panel_set_inactivity_timeout(timeout/TOUCH_TASK_POLL_RATE);
#endif
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
        HAL_GPIO_WritePin(LCD_BL_PORT, LCD_BL_PIN, GPIO_PIN_SET);
    else
        HAL_GPIO_WritePin(LCD_BL_PORT, LCD_BL_PIN, GPIO_PIN_RESET);
#else
    (void)enable;
#endif
}
