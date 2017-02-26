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


#include "panel_meter.h"

/**
 * initialises and draws an LED panel meter style numeric display.
 *
\code

uint8_t buffer[16];
panel_meter_t panelmeter;
// plain panel meter with default colour
panel_meter_init(&panelmeter, buffer, sizeof(buffer), (point_t){100, 40}, (point_t){0, 0}, true,
                     "%02f, "units", &Ubintu_32, &Ubuntu_16);
// enable touch handling
// panel_meter_enable_touch(&panelmeter, callback, appdata);
// set colours
// touch_key_set_colour(&panelmeter->touch_key, border, background, alt, text);
\endcode
 */
void panel_meter_init(panel_meter_t* meter, char* buffer,
                        point_t position, point_t size, bool rounded,
                        char* precision, char* units, const font_t* font, const font_t* units_font)
{
    // draw background with units
    meter->precision = precision;
    meter->units = units;
    meter->units_font = units_font;

    touch_key_init(&meter->touch_key, position, size, buffer, rounded ? 8 : 0);
    touch_key_set_font(&meter->touch_key, font);

    panel_meter_draw(meter);
}

/**
 * enables touch event handling on the panel meter.
 */
void panel_meter_enable_touch(panel_meter_t* meter, touch_callback_t callback, void* appdata)
{
    touch_key_add(&meter->touch_key, callback, appdata);
    touch_key_enable(&meter->touch_key, callback != NULL);
}

/**
 * redraws the whole panel meter. the panel meter structure
 * must be initialized with init_panel_meter() before this function is safe to use.
 */
void panel_meter_draw(panel_meter_t* meter)
{
    const char* buffer = meter->touch_key.text.buffer;
    const font_t* font = meter->touch_key.text.font;

    text_set_justification(&meter->touch_key.text, JUSTIFY_BOTTOM|JUSTIFY_RIGHT);
    text_set_buffer(&meter->touch_key.text, meter->units);
    text_set_font(&meter->touch_key.text, meter->units_font);

    text_draw(&meter->touch_key.text, meter->touch_key.location);

    // reset font / buffer for data text
    text_set_buffer(&meter->touch_key.text, buffer);
    text_set_font(&meter->touch_key.text, font);
    text_set_justification(&meter->touch_key.text, JUSTIFY_LEFT);
}

/**
 * updates the the text part of the panel meter. the panel meter structure
 * must be initialized with init_panel_meter() before this function is safe to use.
 */
void panel_meter_update(panel_meter_t* meter, float value)
{
    touch_key_blank_text(&meter->touch_key);
    sprintf((char*)touch_key_get_buffer(&meter->touch_key), meter->precision, (double)value);
    touch_key_redraw_text(&meter->touch_key);
}

void panel_meter_set_units(panel_meter_t* meter, const char* units)
{
    const char* buffer = touch_key_get_buffer(&meter->touch_key);
    const font_t* font = meter->touch_key.text.font;

    touch_key_set_justification(&meter->touch_key, JUSTIFY_BOTTOM|JUSTIFY_RIGHT);
    touch_key_set_buffer(&meter->touch_key, meter->units);
    touch_key_set_font(&meter->touch_key, meter->units_font);
    touch_key_blank_text(&meter->touch_key);
    meter->units = units;
    touch_key_set_buffer(&meter->touch_key, meter->units);
    touch_key_redraw_text(&meter->touch_key);
    // reset font / buffer for data text
    touch_key_set_buffer(&meter->touch_key, buffer);
    touch_key_set_font(&meter->touch_key, font);
    touch_key_set_justification(&meter->touch_key, JUSTIFY_LEFT);
}

const char* panel_meter_get_units(panel_meter_t* meter)
{
    return meter->units;
}
