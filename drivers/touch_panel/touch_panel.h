
#include <stdint.h>
#include <string.h>
#include <math.h>
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
