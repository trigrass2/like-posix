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

#include "touch_key.h"

#ifdef DEBUG_TOUCH_KEY_PRINTF
#define DEBUG_TOUCH_KEY(...) printf(__VA_ARGS__);
#else
#define DEBUG_TOUCH_KEY(...) ((void)0)
#endif

static void touch_key_on_key_stroke(touch_handler_t* handler);

/**
 * initializes a touch key with default settings.
 * does not implicitly add the touch key to the touch panel handlers group.
 *
 * basic touch key setup:
 *
\code

char* buffer = "default";
touch_key_t key;
char* somedata = "my appdata";

void touch_callback(void* _key)
{
    touch_key_t* key = (touch_key_t*)_key;

    printf("key appdata: %s\n", touch_key_get_appdata(key));

    printf("got touch event: %d\n", touch_key_get_press_type(key));

//    check for press of type:
//    SWIPE_LEFT
//    SWIPE_RIGHT
//    SWIPE_UP
//    SWIPE_DOWN
//    KEY_DOWN
//    KEY_HOLD
//    KEY_UP
//    KEY_LONG_PRESS
//    KEY_TAP
    if(touch_key_press_is(key, KEY_DOWN))
    {
        printf("key down!\n");
    }
    else if(touch_key_press_is(key, KEY_LONG_PRESS))
    {
        printf("long press!\n");
    }
}

touch_key_init(&key, (point_t){100, 20}, (point_t){75, 40}, buffer, 0);
//touch_key_set_colour(&key, border, background, alt, text);
touch_key_add(&key, touch_callback, somedata);
touch_key_enable(&key, true);


\endcode
 */
void touch_key_init(touch_key_t* key, point_t location, point_t size, char* buffer, uint16_t radius)
{
    text_init(&key->text, size, buffer, radius);
    text_set_filled(&key->text, true);
    text_set_justification(&key->text, JUSTIFY_CENTRE);
    key->location = location;
    key->alt_colour = DEFAULT_ALT_COLOUR;
    touch_key_redraw(key);
}

/**
 * adds the key to the touch panel handlers group.
 * does not implicitly enable touch events on the key.
 */
bool touch_key_add(touch_key_t* key, touch_callback_t callback, void* appdata)
{
    key->handler.appdata = appdata;
    key->handler.key_callback = callback;
    key->handler.backend_key_callback = touch_key_on_key_stroke;
    key->handler.pressed = false;
	touch_panel_handler_init(&key->handler, &key->location, &key->text.shape.size, key);
	return touch_panel_add_handler(&key->handler);
}

/**
 * sets the key colours.
 * implicitly redraws the key.
 */
void touch_key_set_colour(touch_key_t* key, colour_t border, colour_t background, colour_t alt, colour_t text)
{
    key->alt_colour = alt;
    key->text.colour = text;
    key->text.shape.border_colour = border;
    key->text.shape.fill_colour = background;
    touch_key_redraw(key);
}

/**
 * sets the key size.
 * does not implicitly redraw the key.
 */
void touch_key_set_size(touch_key_t* key, point_t size)
{
    text_set_size(&key->text, size);
}

void touch_key_set_buffer(touch_key_t* key, const char* buffer)
{
    text_set_buffer(&key->text, buffer);
}

const char* touch_key_get_buffer(touch_key_t* key)
{
    return key->text.buffer;
}

void touch_key_redraw(touch_key_t* key)
{
	text_draw(&key->text, key->location);
}

void touch_key_redraw_text(touch_key_t* key)
{
	text_redraw_text(&key->text, key->location);
}

void touch_key_blank_text(touch_key_t* key)
{
    text_blank_text(&key->text, key->location);
}

void touch_key_draw_background(touch_key_t* key)
{
    text_redraw_background(&key->text, key->location);
}

void touch_key_set_fill(touch_key_t* key, bool fill)
{
    text_set_filled(&key->text, fill);
}

void touch_key_set_radius(touch_key_t* key, uint16_t radius)
{
    text_set_radius(&key->text, radius);
}

void touch_key_set_justification(touch_key_t* key, justify_t justify)
{
    text_set_justification(&key->text, justify);
}

void touch_key_set_font(touch_key_t* key, const font_t* font)
{
    text_set_font(&key->text, font);
}

/**
 * enables/disables touch events on the key.
 */
void touch_key_enable(touch_key_t* key, bool enable)
{
    key->handler.enabled = enable;
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
