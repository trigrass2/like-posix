
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
