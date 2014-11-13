

#include "touch_panel.h"

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
    TaskHandle_t touch_task_handle;
    point_t pt;
    touch_handler_t* handlers[TOUCH_MAX_HANDLERS];
    uint16_t long_press_count;
    uint16_t long_press_duration;
    uint16_t tap_count;
    uint16_t tap_min_duration;
    uint16_t tap_max_duration;
    point_t  swipe_start;
    uint16_t swipe_length;
}touch_task_t;

touch_task_t touch_task_data;

static void touch_task(void *pvParameters);

void touch_init()
{
    panel_init();
    memset(&touch_task_data, 0, sizeof(touch_task_t));

    touch_set_long_press_duration(TOUCH_DEFAULT_LONG_PRESS_MS);
    touch_set_tap_duration(TOUCH_DEFAULT_TAP_MIN_MS, TOUCH_DEFAULT_TAP_MAX_MS);
    touch_set_swipe_length(TOUCH_DEFAULT_SWIPE_LENGTH);

    xTaskCreate(touch_task,
                "touch",
                configMINIMAL_STACK_SIZE + TOUCH_TASK_STACK_SIZE,
                &touch_task_data,
                tskIDLE_PRIORITY + TOUCH_TASK_PRIORITY,
                &touch_task_data.touch_task_handle);
}

void touch_deinit()
{
    vTaskDelete(&touch_task_data.touch_task_handle);
    panel_deinit();
}

point_t touch_xy()
{
	return touch_task_data.pt;
}

bool touch_add_handler(touch_handler_t* handler)
{
	assert_param(handler->backend_key_callback != NULL);

	for(uint8_t i = 0; i < TOUCH_MAX_HANDLERS; i++)
	{
		if(touch_task_data.handlers[i] == NULL)
		{
			touch_task_data.handlers[i] = handler;
			return true;
		}
	}
	return false;
}

void touch_remove_handler(touch_handler_t* handler)
{
	for(uint8_t i = 0; i < TOUCH_MAX_HANDLERS; i++)
	{
		if(touch_task_data.handlers[i] == handler)
		{
			touch_task_data.handlers[i] = NULL;
			break;
		}
	}
}

void touch_clear_handlers()
{
    for(uint8_t i = 0; i < TOUCH_MAX_HANDLERS; i++)
        touch_task_data.handlers[i] = NULL;
}

void touch_set_long_press_duration(uint16_t duration)
{
    touch_task_data.long_press_duration = duration;
}

void touch_set_tap_duration(uint16_t min_duration, uint16_t max_duration)
{
    touch_task_data.tap_min_duration = min_duration;
    touch_task_data.tap_max_duration = max_duration;
}

void touch_set_swipe_length(uint16_t length)
{
    touch_task_data.swipe_length = length;
}

/**
 * polls the touch pad.
 * on a touch the on_press handler is run, if there is one.
 * while the touch persists the on_hold handler is run every poll period, if there is one.
 * when the touch is lifted, the on_release handler is run, if there is one.
 */
void touch_task(void *pvParameters)
{
    portTickType xLastExecutionTime = xTaskGetTickCount();
	touch_task_t* tt_params = (touch_task_t*)pvParameters;
    point_t held_at;

    for(;;)
    {
		touch_handler_t* handler;
        // flush last reading
        panel_x();
        panel_y();
        // get new reading
        tt_params->pt.x = panel_x();
        tt_params->pt.y = panel_y();

        for(uint8_t i = 0; i < TOUCH_MAX_HANDLERS; i++)
        {
			handler = touch_task_data.handlers[i];
			if(handler && handler->enabled)
			{
                // check if press is on current handler area
	        	if(tt_params->pt.x > handler->location.x &&
                   tt_params->pt.y > handler->location.y &&
				   tt_params->pt.x < (handler->location.x + handler->keydata->text->shape->size.x) &&
				   tt_params->pt.y < (handler->location.y + handler->keydata->text->shape->size.y))
				{
					if(!handler->pressed)
					{
                        handler->press_type = KEY_DOWN;
						handler->backend_key_callback(handler);
						handler->pressed = true;
                        tt_params->long_press_count = 0;
                        tt_params->tap_count = 0;
                        tt_params->swipe_start = tt_params->pt;
						break;
					}
					else
                    {
                        // hold timing
                        held_at = tt_params->pt;
                        handler->press_type = KEY_HOLD;
						handler->backend_key_callback(handler);

                        // tap timing
                        tt_params->tap_count += TOUCH_TASK_POLL_RATE;

                        // long press timing
                        if(!handler->long_pressed)
                        {
                            tt_params->long_press_count += TOUCH_TASK_POLL_RATE;
                            if(tt_params->long_press_count >= tt_params->long_press_duration)
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
                    int16_t dx = tt_params->swipe_start.x - held_at.x;
                    int16_t dy = tt_params->swipe_start.y - held_at.y;
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
                    else if(tt_params->tap_count <= (tt_params->tap_max_duration) &&
                        tt_params->tap_count >= (tt_params->tap_min_duration))
                    {
                        handler->press_type = KEY_TAP;
                        handler->backend_key_callback(handler);
                    }
                    else
                    {
                        handler->press_type = KEY_UP;
    					handler->backend_key_callback(handler);
                    }
                    handler->long_pressed = false;
                    handler->pressed = false;
				}
			}
        }

        taskYIELD();
        vTaskDelayUntil(&xLastExecutionTime, TOUCH_TASK_POLL_RATE/portTICK_RATE_MS);
    }
}
