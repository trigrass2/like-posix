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

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "lcd.h"
#include "image.h"

void image_init(image_t* image, point_t location)
{
    image->location = location;
}

void image_draw(image_t* image)
{
    uint32_t i;
    LCD_LOCK();

    lcd_set_entry_mode_window();
    lcd_set_window(image->location.x, image->location.y, image->size.x, image->size.y);
    lcd_set_cursor(0, 0);
    lcd_rw_gram();
    for(i = 0; i < (uint32_t)(image->size.x * image->size.y); i++)
        write_data(image->data[i]);
    lcd_set_entry_mode_normal();

    LCD_UNLOCK();
}

void icon_setup(icon_t* icon, point_t location, colour_t foreground, colour_t background)
{
    icon->foreground = foreground;
    icon->background = background;
    icon->location = location;
}

void icon_draw(icon_t* icon)
{
    uint32_t i;
    uint8_t alpha;

    LCD_LOCK();

    lcd_set_entry_mode_window();
    lcd_set_window(icon->location.x, icon->location.y, icon->size.x, icon->size.y);
    lcd_set_cursor(0, 0);
    lcd_rw_gram();

    for(i = 0; i < (uint32_t)(icon->size.x * icon->size.y); i++)
    {
        alpha = icon->data[i];
        // speed up writes, no math for alpha = 0 or 255
        if(alpha == 0)
            write_data(icon->background);
        else if(alpha == MAX_ALPHA)
            write_data(icon->foreground);
        else
            write_data(blend_colour(icon->foreground, alpha, icon->background, MAX_ALPHA));
    }

    lcd_set_entry_mode_normal();

    LCD_UNLOCK();
}
