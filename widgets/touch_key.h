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

#include <stdint.h>
#include <stdbool.h>

#include "touch_panel.h"
#include "graphics.h"
#include "text.h"

#ifndef TOUCH_KEY_H_
#define TOUCH_KEY_H_

// define to enable debug
// #define DEBUG_TOUCH_KEY_PRINTF

bool touch_add_key(touch_handler_t* key);
void touch_redraw_key(touch_handler_t* handler);
void touch_redraw_text(touch_handler_t* handler);
void touch_enable_key(touch_handler_t* handler, bool enable);

text_t* touch_get_text(touch_handler_t* handler);
keypress_type_t touch_get_press_type(touch_handler_t* handler);
bool touch_press_is(touch_handler_t* handler, keypress_type_t type);
bool touch_key_is(touch_handler_t* handler, touch_handler_t* key);
const char* touch_get_press_type_string(touch_handler_t* handler);
void* touch_get_appdata(touch_handler_t* key);

void touch_set_callback(touch_handler_t* key, touch_callback_t callback);
void touch_set_appdata(touch_handler_t* key, void* appdata);

#endif // TOUCH_KEY_H_
