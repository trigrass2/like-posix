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
 * This file is part of the graphics project, <https://github.com/drmetal/graphics>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

#include "text.h"


point_t text_bounds(const char* str, const font_t* font)
{
	point_t size = {0, font->size};

	while(*str)
	{
		size.x += font->characters[*str - font->base_id]->xadvance;
		str++;
	}

	return size;
}

point_t text_justify(text_t* text, point_t location)
{
    // center text on shape
    point_t text_location;
    if(text->buffer)
        text_location = text_bounds(text->buffer, text->font);
    else
        text_location = (point_t){0, 0};

    if(text->justify & JUSTIFY_LEFT)
        text_location.x = location.x + TEXT_DEFAULT_MARGIN_LEFT;
    else if(text->justify & JUSTIFY_RIGHT)
        text_location.x = location.x + text->shape.size.x - TEXT_DEFAULT_MARGIN_RIGHT - text_location.x;
    else
        text_location.x = location.x + (text->shape.size.x / 2) - (text_location.x / 2);

    if(text->justify & JUSTIFY_TOP)
        text_location.y = location.y + TEXT_DEFAULT_MARGIN_TOP;
    else if(text->justify & JUSTIFY_BOTTOM)
        text_location.y = location.y + text->shape.size.y - TEXT_DEFAULT_MARGIN_BOTTOM - text_location.y;
    else
        text_location.y = location.y + (text->shape.size.y / 2) - (text_location.y / 2);

    return text_location;
}

int16_t text_get_height(text_t* text)
{
    return text->font->size;
}

void text_set_justification(text_t* text, justify_t justification)
{
    text->justify = justification;
}

void text_set_font(text_t* text, const font_t* font)
{
    text->font = font;
}

void text_set_colour(text_t* text, colour_t colour)
{
    text->colour = colour;
}

void text_set_background_colour(text_t* text, colour_t colour)
{
    text->shape.fill_colour = colour;
}

void text_set_border_colour(text_t* text, colour_t colour)
{
    text->shape.border_colour = colour;
}

void text_set_buffer(text_t* text, const char* str)
{
    text->buffer = str;
}

void text_blank_text(text_t* text, point_t location)
{
    point_t text_location = text_justify(text, location);
    text_draw_raw(text, text_location, true);
}

void text_update_text(text_t* text, const char* str, point_t location)
{
    point_t text_location = text_justify(text, location);
    text_draw_raw(text, text_location, true);
    text->buffer = str;
    text_draw_raw(text, text_location, false);
}

void text_draw(text_t* text, point_t location)
{
    point_t text_location = text_justify(text, location);
    draw_shape(&text->shape, location);
    text_draw_raw(text, text_location, false);
}

void text_redraw_text(text_t* text, point_t location)
{
    point_t text_location = text_justify(text, location);
    text_draw_raw(text, text_location, false);
}

void text_redraw_background(text_t* text, point_t location)
{
    draw_shape(&text->shape, location);
}

void text_draw_raw(text_t* text, point_t location, bool blank)
{
    LCD_LOCK();

    int16_t run;
    int16_t run_count;
    int16_t pix;
    uint8_t alpha = 0;
    const character_t* ch;
    const char* str;

    if(text->buffer)
        str = text->buffer;
    else
        str = (const char*)"";

    lcd_set_entry_mode_window();

    for(;*str;str++)
    {
        ch = text->font->characters[*str - text->font->base_id];
        run = 0;
        run_count = 0;

        lcd_set_window(location.x, location.y + ch->yoffset, ch->width, ch->height);
        lcd_set_cursor(0, 0);
        lcd_rw_gram();

        pix = ch->width * ch->height;

        if(blank)
        {
            while(pix--)
                write_data(text->shape.fill_colour);
        }
        else
        {
            while(pix--)
            {
                if(!run_count)
                {
                    run_count = ch->data[run];
                    alpha = ch->data[run+1];
                    run += 2;
                }
                run_count--;

                // speed up writes, no math for alpha = 0 or 255
                if(alpha == 0)
                    write_data(text->shape.fill_colour);
                else if(alpha == MAX_ALPHA)
                    write_data(text->colour);
                else
                    write_data(blend_colour(text->colour, alpha, text->shape.fill_colour, MAX_ALPHA));
            }
        }
        location.x += ch->xadvance;
    }

    lcd_set_entry_mode_normal();

    LCD_UNLOCK();
}

