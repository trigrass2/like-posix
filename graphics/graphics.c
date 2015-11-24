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

#include "graphics.h"

/**
 *  holds the last background fill colour.
 */
static colour_t background_colour = BLACK;

/**
 * initializes any graphical stuff - not the display itself.
 */
void graphics_init()
{
    set_background_colour(background_colour);
}

/**
 * sets the whole display to one colour.
 */
void set_background_colour(colour_t colour)
{
    LCD_LOCK();

    background_colour = colour;
    lcd_set_cursor(0, 0);
    lcd_rw_gram(); /* Prepare to write GRAM */
    for(uint32_t i = 0; i < LCD_HEIGHT * LCD_WIDTH; i++)
		write_data(colour);

    LCD_UNLOCK();
}

/**
 * returns the last backgroud colour that was set.
 */
colour_t get_background_colour()
{
    return background_colour;
}

/**
 * algorithm came from here:
 * http://en.wikipedia.org/wiki/Alpha_compositing
 *
 */
colour_t blend_colour(colour_t fgc, alpha_t fga, colour_t bgc, alpha_t bga)
{
    colour_t rgb16 = 0;

    int16_t r, g, b;
    // split the foreground colours
    int16_t fgr = LCD_RED_BYTE(fgc);
    int16_t fgg = LCD_GREEN_BYTE(fgc);
    int16_t fgb = LCD_BLUE_BYTE(fgc);
    // split the background colours
    int16_t bgr = LCD_RED_BYTE(bgc);
    int16_t bgg = LCD_GREEN_BYTE(bgc);
    int16_t bgb = LCD_BLUE_BYTE(bgc);

    r = ((fgr * fga) + (bgr * bga) - ((bgr*fga*bga) / MAX_ALPHA)) / MAX_ALPHA;
    g = ((fgg * fga) + (bgg * bga) - ((bgg*fga*bga) / MAX_ALPHA)) / MAX_ALPHA;
    b = ((fgb * fga) + (bgb * bga) - ((bgb*fga*bga) / MAX_ALPHA)) / MAX_ALPHA;

    // combine forground and background
    LCD_INSERT_RED_BYTE(rgb16, r);
    LCD_INSERT_GREEN_BYTE(rgb16, g);
    LCD_INSERT_BLUE_BYTE(rgb16, b);

    return rgb16;
}
