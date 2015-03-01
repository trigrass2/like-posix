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

#include <unistd.h>
#include "touch_panel.h"
#include "cutensils.h"

const char* key_press_type[] = {
    "SWIPE_LEFT",
    "SWIPE_RIGHT",
    "SWIPE_UP",
    "SWIPE_DOWN",
    "KEY_DOWN",
    "KEY_HOLD",
    "KEY_UP",
    "KEY_LONG_PRESS",
    "KEY_TAP",
};

typedef struct {
    uint16_t inactivity_timeout;                       ///< user settable, inactivity timeout in poll periods
    uint16_t long_press_duration;                       ///< user settable, define the number of poll periods required to register a long press
    uint16_t tap_min_duration;                          ///< user settable, define min timing in key poll periods, of a tap
    uint16_t tap_max_duration;                          ///< user settable, define max timing in key poll periods, of a tap
    uint16_t swipe_length;                              ///< user settable, the length of an unbroken movement required to register a swipe
    point_t _touch_point;                               ///< not user settable,
    touch_handler_t* _handlers[TOUCH_MAX_HANDLERS];     ///< not user settable,
    uint16_t _long_press_count;                         ///< not user settable
    uint16_t _tap_count;                                ///< not user settable
    point_t  _swipe_start;                              ///< not user settable
    uint16_t  _inactivity_count;                         ///< not user settable
    void(*inactivity_callback)(void);                   ///< user settable, a function that is called after a period of inactivity.
    void(*activity_callback)(void);                     ///< user settable, a function that is called once on activity.
}touch_task_t;

touch_task_t touch_task_data;

static void touch_panel_interpreter_task(void *pvParameters);

/**
 * initializes the touch panel driver, and starts the touch panel interpreter task.
 */
bool touch_panel_init()
{
    panel_init();
    memset(&touch_task_data, 0, sizeof(touch_task_t));

    touch_panel_set_long_press_duration(TOUCH_DEFAULT_LONG_PRESS_MS);
    touch_panel_set_tap_duration(TOUCH_DEFAULT_TAP_MIN_MS, TOUCH_DEFAULT_TAP_MAX_MS);
    touch_panel_set_swipe_length(TOUCH_DEFAULT_SWIPE_LENGTH);
    touch_panel_set_inactivity_timeout(TOUCH_DEFAULT_INACTIVITY_TIMEOUT);

    return xTaskCreate(touch_panel_interpreter_task,
                        "touch",
                        configMINIMAL_STACK_SIZE + TOUCH_TASK_STACK_SIZE,
                        &touch_task_data,
                        tskIDLE_PRIORITY + TOUCH_TASK_PRIORITY,
                        NULL) == pdPASS;
}

const char* touch_panel_get_press_type_string(touch_handler_t* handler)
{
    return key_press_type[handler->press_type];
}
/**
 * @retval the last touched point - best read from a callback
 * registered with a touch handler.
 */
point_t touch_panel_xy()
{
	return touch_task_data._touch_point;
}

/**
 * initializes the touch handler.
 */
void touch_panel_handler_init(touch_handler_t* handler, point_t* location, point_t* size, void* parent)
{
    handler->location = location;
    handler->size = size;
    handler->parent = parent;
}

/**
 * adds a touch handler, which is typically associated
 * with a key or other graphical object on the
 * LCD under the touch panel.
 *
 * Note: be sure to call touch_panel_handler_init() before calling this function.
 *
 * @retval returns true if the handler was added.
 */
bool touch_panel_add_handler(touch_handler_t* handler)
{
	assert_param(handler->backend_key_callback != NULL);

	for(uint8_t i = 0; i < TOUCH_MAX_HANDLERS; i++)
	{
		if(touch_task_data._handlers[i] == NULL)
		{
			touch_task_data._handlers[i] = handler;
			return true;
		}
	}
	return false;
}

/**
 * removes a touch handler.
 */
void touch_panel_remove_handler(touch_handler_t* handler)
{
	for(uint8_t i = 0; i < TOUCH_MAX_HANDLERS; i++)
	{
		if(touch_task_data._handlers[i] == handler)
		{
			touch_task_data._handlers[i] = NULL;
			break;
		}
	}
}

/**
 * removes all touch handlers.
 */
void touch_panel_clear_handlers()
{
    for(uint8_t i = 0; i < TOUCH_MAX_HANDLERS; i++)
        touch_task_data._handlers[i] = NULL;
}

/**
 * set the duration, counted in touch poll cycles, of the inactivity timer.
 */
void touch_panel_set_inactivity_timeout(uint16_t timeout)
{
    touch_task_data.inactivity_timeout = timeout;
}
/**
 * set the duration, counted in touch poll cycles, of a long press.
 */
void touch_panel_set_long_press_duration(uint16_t duration)
{
    touch_task_data.long_press_duration = duration;
}

/**
 * set the window, counted in touch poll cycles, of a tap.
 */
void touch_panel_set_tap_duration(uint16_t min_duration, uint16_t max_duration)
{
    touch_task_data.tap_min_duration = min_duration;
    touch_task_data.tap_max_duration = max_duration;
}

/**
 * set the length in pixels, of a swipe. to activate a swipe callback,
 * the swipe must traverse this many pixels on a single handler.
 */
void touch_panel_set_swipe_length(uint16_t length)
{
    touch_task_data.swipe_length = length;
}

/**
 * set callback functions called when the panel is touched,
 * and when it has not been touched for the inactivity period.
 */
void touch_panel_set_activity_callbacks(void(*on_activity)(void), void(*on_inactivity)(void))
{
    touch_task_data.inactivity_callback = on_inactivity;
    touch_task_data.activity_callback = on_activity;
}

/**
 * polls the touch pad.
 * on a touch the on_press handler is run, if there is one.
 * while the touch persists the on_hold handler is run every poll period, if there is one.
 * when the touch is lifted, the on_release handler is run, if there is one.
 */
void touch_panel_interpreter_task(void *pvParameters)
{
    touch_handler_t* handler;
	touch_task_t* tt_params = (touch_task_t*)pvParameters;
    point_t held_at;
    uint16_t down_count = 0;

    for(;;)
    {
		while(!panel_ready() && !down_count)
		{
		    usleep(TOUCH_TASK_POLL_RATE * 1000);

		    if(tt_params->_inactivity_count < touch_task_data.inactivity_timeout)
		        tt_params->_inactivity_count++;
		    else if(tt_params->_inactivity_count == touch_task_data.inactivity_timeout)
		    {
		        if(tt_params->inactivity_callback)
		            tt_params->inactivity_callback();
                tt_params->_inactivity_count++;
		    }
		}

		if(tt_params->_inactivity_count == (touch_task_data.inactivity_timeout + 1))
		    tt_params->_inactivity_count = 0;
		else
		    tt_params->_inactivity_count = 3;

        // flush last reading
        panel_x();
        panel_y();

		while(panel_ready() || down_count)
		{
		    if(tt_params->_inactivity_count == 0 && tt_params->activity_callback)
		    {
	            tt_params->_inactivity_count = 1;
		        tt_params->activity_callback();
                tt_params->_inactivity_count = 2;
		    }
		    else if(tt_params->_inactivity_count == 2)
		    {

		    }
		    else
		    {
                // get new reading
                tt_params->_touch_point.x = panel_x();
                tt_params->_touch_point.y = panel_y();

    //            log_edebug(NULL, "%d,%d", tt_params->_touch_point.x, tt_params->_touch_point.y);

                for(uint8_t i = 0; i < TOUCH_MAX_HANDLERS; i++)
                {
                    handler = touch_task_data._handlers[i];
                    if(handler && handler->enabled)
                    {
                        // check if press is on current handler area
                        if(tt_params->_touch_point.x > handler->location->x &&
                           tt_params->_touch_point.y > handler->location->y &&
                           tt_params->_touch_point.x < (handler->location->x + handler->size->x) &&
                           tt_params->_touch_point.y < (handler->location->y + handler->size->y))
                        {
                            if(!handler->pressed)
                            {
                                handler->press_type = KEY_DOWN;
                                handler->backend_key_callback(handler);
                                handler->pressed = true;
                                down_count++;
                                tt_params->_long_press_count = 0;
                                tt_params->_tap_count = 0;
                                tt_params->_swipe_start = tt_params->_touch_point;
                                break;
                            }
                            else
                            {
                                // hold timing
                                held_at = tt_params->_touch_point;
                                handler->press_type = KEY_HOLD;
                                handler->backend_key_callback(handler);

                                // tap timing
                                tt_params->_tap_count += TOUCH_TASK_POLL_RATE;

                                // long press timing
                                if(!handler->long_pressed)
                                {
                                    tt_params->_long_press_count += TOUCH_TASK_POLL_RATE;
                                    if(tt_params->_long_press_count >= tt_params->long_press_duration)
                                    {
                                        handler->long_pressed = true;
                                        handler->press_type = KEY_LONG_PRESS;
                                        handler->backend_key_callback(handler);
                                    }
                                }
                            }
                        }
                        else if(handler->pressed)
                        {
                            // swipe condition
                            int16_t dx = tt_params->_swipe_start.x - held_at.x;
                            int16_t dy = tt_params->_swipe_start.y - held_at.y;
                            bool swiped = false;

                            if(abs(dx) >= tt_params->swipe_length)
                            {
                                swiped = true;
                                if(dx < 0)
                                    handler->press_type = SWIPE_RIGHT;
                                else
                                    handler->press_type = SWIPE_LEFT;
                            }
                            else if(abs(dy) >= tt_params->swipe_length)
                            {
                                swiped = true;
                                if(dy < 0)
                                    handler->press_type = SWIPE_UP;
                                else
                                    handler->press_type = SWIPE_DOWN;
                            }

                            if(swiped)
                                handler->backend_key_callback(handler);

                            // tap condition
                            else if(tt_params->_tap_count <= (tt_params->tap_max_duration) &&
                                tt_params->_tap_count >= (tt_params->tap_min_duration))
                            {
                                handler->press_type = KEY_TAP;
                                handler->backend_key_callback(handler);
                            }
    //                        else
    //                        {
                            handler->press_type = KEY_UP;
                            handler->backend_key_callback(handler);
    //                        }
                            handler->long_pressed = false;
                            handler->pressed = false;

                            down_count--;
                        }
                    }
                }
		    }

            usleep(TOUCH_TASK_POLL_RATE * 1000);
		}
    }
}
