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

#include "touch_key.h"

#ifdef DEBUG_TOUCH_KEY_PRINTF
#define DEBUG_TOUCH_KEY(...) printf(__VA_ARGS__);
#else
#define DEBUG_TOUCH_KEY(...) ((void)0)
#endif

static void touch_on_key_stroke(touch_handler_t* handler);


bool touch_add_key(touch_handler_t* handler)
{
	handler->backend_key_callback = touch_on_key_stroke;
	handler->pressed = false;

	touch_redraw_key(handler);
	return touch_add_handler(handler);
}

void touch_enable_key(touch_handler_t* handler, bool enable)
{
    handler->enabled = enable;
}

void touch_redraw_key(touch_handler_t* handler)
{
	draw_textbox(handler->keydata->text, handler->location);
}

void touch_redraw_text(touch_handler_t* handler)
{
	redraw_textbox_text(handler->keydata->text, handler->location);
}

void touch_set_callback(touch_handler_t* key, touch_callback_t callback)
{
	key->key_callback = callback;
}

void touch_set_appdata(touch_handler_t* key, void* appdata)
{
	key->appdata = appdata;
}

void* touch_get_appdata(touch_handler_t* handler)
{
	return handler->appdata;
}

text_t* touch_get_text(touch_handler_t* handler)
{
	return handler->keydata->text;
}

bool touch_press_is(touch_handler_t* handler, keypress_type_t type)
{
	return handler->press_type == type;
}

bool touch_key_is(touch_handler_t* handler, touch_handler_t* key)
{
	return handler == key;
}

keypress_type_t touch_get_press_type(touch_handler_t* handler)
{
    return handler->press_type;
}

const char* touch_get_press_type_string(touch_handler_t* handler)
{
    return key_press_type[handler->press_type];
}

void touch_on_key_stroke(touch_handler_t* handler)
{
	DEBUG_TOUCH_KEY("%s %s\n", touch_get_press_type_string(handler), handler->keydata->text->buffer);

	switch(handler->press_type)
	{
		case KEY_DOWN:
		{
			bool redraw = handler->keydata->text->shape->fill_colour !=
				handler->keydata->alt_colour;
			if(redraw)
			{
				colour_t colour = handler->keydata->text->shape->fill_colour;
				handler->keydata->text->shape->fill_colour = handler->keydata->alt_colour;
				handler->keydata->alt_colour = colour;
			}
			if(handler->key_callback)
				handler->key_callback(handler);
			if(redraw)
				touch_redraw_key(handler);
		}
		break;

		case KEY_UP:
		{
			bool redraw = handler->keydata->text->shape->fill_colour !=
				handler->keydata->alt_colour;
			if(redraw)
			{
				colour_t colour = handler->keydata->text->shape->fill_colour;
				handler->keydata->text->shape->fill_colour = handler->keydata->alt_colour;
				handler->keydata->alt_colour = colour;
			}
			if(handler->key_callback)
				handler->key_callback(handler);
			if(redraw)
				touch_redraw_key(handler);
		}
		break;

		// KEY_TAP, KEY_LONG_PRESS,KEY_HOLD, SWIPE_LEFT, SWIPE_RIGHT, SWIPE_UP, SWIPE_DOWN
		default:
			if(handler->key_callback)
				handler->key_callback(handler);
		break;
	}
}
