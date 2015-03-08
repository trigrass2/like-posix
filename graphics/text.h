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

#include  <stdint.h>
#include "lcd.h"
#include "shape.h"
#include "Digital_7_Italic_128.h"
#include "Digital_7_Italic_96.h"
#include "Digital_7_Italic_64.h"
#include "Digital_7_Italic_32.h"
#include "Ubuntu_64.h"
#include "Ubuntu_64_bold.h"
#include "Ubuntu_32.h"
#include "Ubuntu_48.h"
#include "Ubuntu_48_bold.h"
#include "Ubuntu_38.h"
#include "Ubuntu_24.h"
#include "Ubuntu_20.h"
#include "Ubuntu_16.h"

#ifndef TEXT_H_
#define TEXT_H_

#define TEXT_DEFAULT_MARGIN_LEFT 4
#define TEXT_DEFAULT_MARGIN_RIGHT 4
#define TEXT_DEFAULT_MARGIN_TOP 4
#define TEXT_DEFAULT_MARGIN_BOTTOM 4

typedef enum {
    JUSTIFY_CENTRE = 0x00,
    JUSTIFY_LEFT = 0x01,
    JUSTIFY_RIGHT = 0x02,
    JUSTIFY_TOP = 0x04,
    JUSTIFY_BOTTOM = 0x08
}justify_t;

typedef struct {
    const char* buffer;
    colour_t colour;
    const font_t* font;
    justify_t justify;
    shape_t shape;
} text_t;

void text_init(text_t* text, point_t size, const char* buffer, uint16_t radius);

void text_draw(text_t* text, point_t location);
void text_redraw_text(text_t* text, point_t location);
void text_redraw_background(text_t* text, point_t location);
void text_draw_raw(text_t* text, point_t location, bool blank);

void text_set_buffer(text_t* text, const char* str);
void text_blank_text(text_t* text, point_t location);
void text_update_text(text_t* text, const char* str, point_t location);

point_t text_justify(text_t* text, point_t location);
point_t text_bounds(const char* str, const font_t* font);

void text_set_justification(text_t* text, justify_t justification);
void text_set_filled(text_t* text, bool filled);
void text_set_radius(text_t* text, int16_t radius);
void text_set_font(text_t* text, const font_t* font);
void text_set_colour(text_t* text, colour_t colour);
void text_set_size(text_t* text, point_t size);
void text_set_background_colour(text_t* text, colour_t colour);
void text_set_border_colour(text_t* text, colour_t colour);

int16_t text_get_height(text_t* text);



#endif // TEXT_H_
