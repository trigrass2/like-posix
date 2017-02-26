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
 * This file is part of the like-posix project, <https://github.com/drmetal/like-posix>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

#include "text.h"

const text_t text_defaults = {
    .colour = WHITE,
    .font = &Ubuntu_32,
    .justify = JUSTIFY_LEFT,
    .shape = {
        .type = SQUARE,
        .fill_colour = 0x3666,
        .border_colour = 0x3666,
        .fill = true,
        .size = {60, 40},
        .radius = 0
    }
};

/**
 * initializes a text box, filling all parameters from the default text structure @ref text_defaults.
 *
 * does not implicitly draw the text box, use the text_draw()
 * function after first initializing, then configuring the textbox.
 *
 * The text parameter need not be initialized before use, it will be overwritten anyway.
 *
 * to customize a text box, use the functions below.
 */
void text_init(text_t* text, point_t size, const char* buffer, uint16_t radius)
{
    *text = text_defaults;
    text->buffer = buffer;
    text->shape.size = size;
    text->shape.radius = radius;
}

/**
 * returns the size in pixels, x and y, of a text string given the specified font.
 * does not draw the text.
 */
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

/**
 * returns the coordinates where text shoud be drawn, to justify
 * it within the area of the specified textbox. location is the location of
 * the textbox itself.
 */
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

/**
 * returns the maximum height in pixels of the text in a text box.
 */
int16_t text_get_height(text_t* text)
{
    return text->font->size;
}

/**
 * sets the intended justification, does not redraw the textbox.
 */
void text_set_justification(text_t* text, justify_t justification)
{
    text->justify = justification;
}

/**
 * sets the intended background area size, does not redraw the textbox.
 */
void text_set_size(text_t* text, point_t size)
{
    text->shape.size = size;
}

/**
 * sets the intended text font, does not redraw the textbox.
 */
void text_set_font(text_t* text, const font_t* font)
{
    text->font = font;
}

/**
 * sets the intended text colour, does not redraw the textbox.
 */
void text_set_colour(text_t* text, colour_t colour)
{
    text->colour = colour;
}

/**
 * sets the intended background colour, does not redraw the textbox.
 */
void text_set_background_colour(text_t* text, colour_t colour)
{
    text->shape.fill_colour = colour;
}

/**
 * sets the intended border colour, does not redraw the textbox.
 */
void text_set_border_colour(text_t* text, colour_t colour)
{
    text->shape.border_colour = colour;
}

/**
 * sets the intended filled state, does not redraw the textbox.
 */
void text_set_filled(text_t* text, bool filled)
{
    text->shape.fill = filled;
}

/**
 * sets the intended radius of the text background corners, does not redraw the textbox.
 */
void text_set_radius(text_t* text, int16_t radius)
{
    text->shape.radius = radius;
}

/**
 * sets the text buffer, does not redraw the textbox.
 */
void text_set_buffer(text_t* text, const char* str)
{
    text->buffer = str;
}

void text_blank_text(text_t* text, point_t location)
{
    point_t text_location = text_justify(text, location);
    text_draw_raw(text, text_location, true);
}

/**
 * sets the text buffer, blanks the old text then redraws the new text.
 */
void text_update_text(text_t* text, const char* str, point_t location)
{
    point_t text_location = text_justify(text, location);
    text_draw_raw(text, text_location, true);
    text->buffer = str;
    text_draw_raw(text, text_location, false);
}

/**
 * redraws the background, overwriting the text, then redraws the text.
 */
void text_draw(text_t* text, point_t location)
{
    point_t text_location = text_justify(text, location);
    draw_shape(&text->shape, location);
    text_draw_raw(text, text_location, false);
}

/**
 * redraws the old text only, without blanking anything.
 */
void text_redraw_text(text_t* text, point_t location)
{
    point_t text_location = text_justify(text, location);
    text_draw_raw(text, text_location, false);
}

/**
 * redraws the background, blanking the text.
 */
void text_redraw_background(text_t* text, point_t location)
{
    draw_shape(&text->shape, location);
}

/**
 * this is the fundamental text draw routine. writes the text buffer
 * to the display at location. if blank is true, draws the background colour
 * where the text should be. if false, draws the text like normal.
 *
 * does not draw the textbox background.
 */
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

