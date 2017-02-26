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

#include <stdint.h>
#include <stdbool.h>

#include "touch_panel.h"
#include "graphics.h"
#include "text.h"

#ifndef TOUCH_KEY_H_
#define TOUCH_KEY_H_

typedef struct {
    text_t text;
    colour_t alt_colour;
    point_t location;
    touch_handler_t handler;
}touch_key_t;

#define DEFAULT_ALT_COLOUR   0x4E99
// define to enable debug
// #define DEBUG_TOUCH_KEY_PRINTF

void touch_key_init(touch_key_t* key, point_t location, point_t size, char* buffer, uint16_t radius);
bool touch_key_add(touch_key_t* key, touch_callback_t callback, void* appdata);
void touch_key_enable(touch_key_t* key, bool enable);

void touch_key_redraw(touch_key_t* key);
void touch_key_redraw_text(touch_key_t* key);
void touch_key_blank_text(touch_key_t* key);
void touch_key_draw_background(touch_key_t* key);

text_t* touch_key_get_text(touch_key_t* key);
keypress_type_t touch_key_get_press_type(touch_key_t* key);
bool touch_key_press_is(touch_key_t* key, keypress_type_t type);

void touch_key_set_buffer(touch_key_t* key, const char* buffer);
const char* touch_key_get_buffer(touch_key_t* key);
void touch_key_set_fill(touch_key_t* key, bool fill);
void touch_key_set_radius(touch_key_t* key, uint16_t radius);
void touch_key_set_justification(touch_key_t* key, justify_t justify);
void touch_key_set_font(touch_key_t* key, const font_t* font);
void touch_key_set_size(touch_key_t* key, point_t size);
void touch_key_set_colour(touch_key_t* key, colour_t border, colour_t background, colour_t alt_background, colour_t text);
void touch_key_set_callback(touch_key_t* key, touch_callback_t callback);
void touch_key_set_appdata(touch_key_t* key, void* appdata);
void* touch_key_get_appdata(touch_key_t* key);

#endif // TOUCH_KEY_H_
