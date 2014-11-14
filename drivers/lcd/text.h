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

#include  <stdint.h>
#include "lcd.h"
#include "graphics.h"
#include "font_type.h"

#ifndef TEXT_H_
#define TEXT_H_


typedef enum {
    JUSTIFY_CENTRE = 0x00,
    JUSTIFY_LEFT = 0x01,
    JUSTIFY_RIGHT = 0x02,
    JUSTIFY_TOP = 0x04,
    JUSTIFY_BOTTOM = 0x08
}justify_t;


#define TEXT_DEFAULT_MARGIN_LEFT 4
#define TEXT_DEFAULT_MARGIN_RIGHT 4
#define TEXT_DEFAULT_MARGIN_TOP 4
#define TEXT_DEFAULT_MARGIN_BOTTOM 4


typedef struct {
    const char* buffer;
    colour_t colour;
    const font_t* font;
    justify_t justify;
    shape_t* shape;
} text_t;


void draw_textbox(text_t* text, point_t location);
void redraw_textbox_text(text_t* text, point_t location);
void redraw_textbox_background(text_t* text, point_t location);


void text_set_background_shape(text_t* text, shape_t* shape);
void text_set_justification(text_t* text, justify_t justification);
void text_set_font(text_t* text, const font_t* font);
void text_set_colour(text_t* text, colour_t colour);
void text_set_background_colour(text_t* text, colour_t colour);
void text_set_border_colour(text_t* text, colour_t colour);
void text_set_buffer(text_t* text, const char* str);
void text_blank_text(text_t* text, point_t location);
void text_update_text(text_t* text, const char* str, point_t location);

int16_t get_text_height(text_t* text);

point_t text_justify(text_t* text, point_t location);
point_t text_bounds(const char* str, const font_t* font);
void draw_raw_text(text_t* text, point_t location, bool blank);

#endif // TEXT_H_
