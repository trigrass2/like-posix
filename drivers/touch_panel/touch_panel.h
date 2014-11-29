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

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "tsc2046.h"
#include "text.h"
#include "graphics.h"

#ifndef TOUCH_PANEL_H_
#define TOUCH_PANEL_H_

/**
 * setup touch task
 */
#define TOUCH_TASK_STACK_SIZE		100
#define TOUCH_TASK_POLL_RATE		25
#define TOUCH_DEFAULT_LONG_PRESS_MS 1500
#define TOUCH_DEFAULT_TAP_MIN_MS 	50
#define TOUCH_DEFAULT_TAP_MAX_MS 	100
#define TOUCH_DEFAULT_SWIPE_LENGTH 	70

/**
 * setup panel interface
 */
#define panel_x() 					tsc2046_x()
#define panel_y() 					tsc2046_y()
#define panel_init() 				tsc2046_init()
#define panel_deinit() 				tsc2046_deinit()

#define TOUCH_MAX_HANDLERS 			10

extern const char* key_press_type[];

typedef enum {
	SWIPE_LEFT = 0,
	SWIPE_RIGHT,
	SWIPE_UP,
	SWIPE_DOWN,
	KEY_DOWN,
	KEY_HOLD,
	KEY_UP,
	KEY_LONG_PRESS,
	KEY_TAP,
} keypress_type_t;

typedef struct {
	text_t* text;
	colour_t alt_colour;				///< text string colour
}touch_key_t;

typedef struct _touch_handler_t touch_handler_t;

typedef void(*touch_callback_t)(touch_handler_t*);

struct _touch_handler_t {
    bool enabled;							///< enables / disables key callbacks
	point_t location;						///< the location of the key on the display
	touch_key_t* keydata;					///< the graphical key data
	void* appdata;							///< set by the application, is a pointer to application data.
	touch_callback_t key_callback;			///< set by application to be run on key press
	touch_callback_t backend_key_callback;	///< not to be set by application.
	bool pressed;							///< read only by application. the current state of the key (true = presed, false = not pressed)
	bool long_pressed;						///< read only by application. the current state of the key (true = long presed, false = not long pressed)
	keypress_type_t press_type;				///< read only by application. the direction of a swipe on the key.
};

void touch_init();
void touch_deinit();
point_t touch_xy();

bool touch_add_handler(touch_handler_t* handler);
void touch_remove_handler(touch_handler_t* handler);
void touch_clear_handlers();
void touch_set_long_press_duration(uint16_t duration);
void touch_set_tap_duration(uint16_t min_duration, uint16_t max_duration);
void touch_set_swipe_length(uint16_t length);


#endif ///< TOUCH_PANEL_H_
