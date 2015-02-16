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

#include "touch_key.h"

#ifdef DEBUG_TOUCH_KEY_PRINTF
#define DEBUG_TOUCH_KEY(...) printf(__VA_ARGS__);
#else
#define DEBUG_TOUCH_KEY(...) ((void)0)
#endif

static void touch_key_on_key_stroke(touch_handler_t* handler);


bool touch_key_add(touch_key_t* key)
{
    key->handler.backend_key_callback = touch_key_on_key_stroke;
    key->handler.pressed = false;
	touch_key_redraw(key);
	touch_panel_handler_init(&key->handler, &key->location, &key->text.shape.size, key);
	return touch_panel_add_handler(&key->handler);
}

void touch_key_enable(touch_key_t* key, bool enable)
{
    key->handler.enabled = enable;
}

void touch_key_redraw(touch_key_t* key)
{
	text_draw(&key->text, key->location);
}

void touch_key_redraw_text(touch_key_t* key)
{
	text_redraw_text(&key->text, key->location);
}

void touch_key_set_callback(touch_key_t* key, touch_callback_t callback)
{
    key->handler.key_callback = callback;
}

void touch_key_set_appdata(touch_key_t* key, void* appdata)
{
    key->handler.appdata = appdata;
}

void* touch_key_get_appdata(touch_key_t* key)
{
	return key->handler.appdata;
}

text_t* touch_key_get_text(touch_key_t* key)
{
	return &key->text;
}

bool touch_key_press_is(touch_key_t* key, keypress_type_t type)
{
	return key->handler.press_type == type;
}

keypress_type_t touch_key_get_press_type(touch_key_t* key)
{
    return key->handler.press_type;
}

void touch_key_on_key_stroke(touch_handler_t* handler)
{
    touch_key_t* key = (touch_key_t*)(handler->parent);

	DEBUG_TOUCH_KEY("%s %s\n", touch_panel_get_press_type_string(handler), key->text.buffer);

	switch(handler->press_type)
	{
		case KEY_DOWN:
		{
			bool redraw = key->text.shape.fill_colour != key->alt_colour;
			if(redraw)
			{
				colour_t colour = key->text.shape.fill_colour;
				key->text.shape.fill_colour = key->alt_colour;
				key->alt_colour = colour;
			}
			if(redraw)
				touch_key_redraw(key);
			if(handler->key_callback)
			    handler->key_callback(key);
		}
		break;

		case KEY_UP:
		{
			bool redraw = key->text.shape.fill_colour != key->alt_colour;
			if(redraw)
			{
				colour_t colour = key->text.shape.fill_colour;
				key->text.shape.fill_colour = key->alt_colour;
				key->alt_colour = colour;
			}
			if(redraw)
			    touch_key_redraw(key);
			if(handler->key_callback)
			    handler->key_callback(key);
		}
		break;

		// KEY_TAP, KEY_LONG_PRESS,KEY_HOLD, SWIPE_LEFT, SWIPE_RIGHT, SWIPE_UP, SWIPE_DOWN
		default:
		    if(handler->key_callback)
		        handler->key_callback(key);
		break;
	}
}
