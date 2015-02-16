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

#include "slider.h"

static void slider_touch_callback(touch_key_t* handler);

void slider_init(slider_t* slider, point_t location, point_t size, int16_t min, int16_t max)
{
    slider->slide_position.x = location.x + SLIDER_SLIDE_MARGINS;
    slider->slide_position.y = location.y + SLIDER_SLIDE_MARGINS;
    slider->touch_key.location = location;
    slider->min = min;
    slider->max = max;
    slider->value = min;

    slider->slide.border_colour = DARK_GREY;
    slider->slide.fill = true;
    slider->slide.fill_colour = DARK_GREY;
    slider->slide.radius = 2;
    slider->slide.size.x = size.x-(2 * SLIDER_SLIDE_MARGINS);
    slider->slide.size.y = size.y/SLIDER_SLIDE_RATIO;
    slider->slide.type = SQUARE;

    slider->touch_key.text.shape.border_colour = DARK_GREY;
    slider->touch_key.text.shape.fill = true;
    slider->touch_key.text.shape.fill_colour = MID_GREY;
    slider->touch_key.text.shape.radius = 2;
    slider->touch_key.text.shape.size = size;
    slider->touch_key.text.shape.type = SQUARE;

    slider->touch_key.text.buffer = NULL;
    slider->touch_key.text.colour = BLACK;
    slider->touch_key.text.font = &Ubuntu_16;
    slider->touch_key.text.justify = JUSTIFY_LEFT;

    slider->touch_key.alt_colour = DARK_GREY;

    touch_key_set_callback(&slider->touch_key, (touch_callback_t)slider_touch_callback);
    touch_key_set_appdata(&slider->touch_key, slider);
    touch_key_add(&slider->touch_key);
    touch_key_enable(&slider->touch_key, true);

    draw_shape(&slider->slide, slider->slide_position);
}

void slider_touch_callback(touch_key_t* key)
{
    slider_t* slider = (slider_t*)touch_key_get_appdata(key);
    colour_t fill;
    colour_t brdr;
    int16_t value = slider->value;
    point_t pt;

    if(key->handler.press_type == KEY_HOLD)
    {
        pt = touch_panel_xy();

        // move the handle to the new position
        if(pt.y < slider->touch_key.location.y + SLIDER_SLIDE_MARGINS)
            pt.y = slider->touch_key.location.y + SLIDER_SLIDE_MARGINS;
        else if(pt.y > slider->touch_key.location.y + slider->touch_key.text.shape.size.y - (slider->slide.size.y + SLIDER_SLIDE_MARGINS))
            pt.y = slider->touch_key.location.y + slider->touch_key.text.shape.size.y - (slider->slide.size.y + SLIDER_SLIDE_MARGINS);

        // calculate application freindly slider value
        // min + ((((max - min ) * curr pixel) / (pixel range))
        value = slider->min + (((slider->max - slider->min) *
           (pt.y - (slider->touch_key.location.y + SLIDER_SLIDE_MARGINS))) /
           ((slider->touch_key.location.y + slider->touch_key.text.shape.size.y - (SLIDER_SLIDE_MARGINS + slider->slide.size.y))  -
           (slider->touch_key.location.y + SLIDER_SLIDE_MARGINS)));

        if(slider->value != value)
        {
            // save colours
            fill = slider->slide.fill_colour;
            brdr = slider->slide.border_colour;
            // undraw the slide handle
            slider->slide.border_colour = slider->touch_key.text.shape.fill_colour;
            slider->slide.fill_colour = slider->touch_key.text.shape.fill_colour;
            draw_shape(&slider->slide, slider->slide_position);
            // restore colours, update position
            slider->slide.fill_colour = fill;
            slider->slide.border_colour = brdr;
            slider->slide_position.y = pt.y;
        }
    }
    // redraw slide handle
    if((slider->value != value) || (key->handler.press_type == KEY_UP))
    {
        // save new value
        slider->value = value;
        draw_shape(&slider->slide, slider->slide_position);
    }
}

void slider_set_colours(slider_t* slider, colour_t border, colour_t background, colour_t alt_background,
                        colour_t slide_border, colour_t slide_background)
{
    slider->touch_key.text.shape.border_colour = border;
    slider->touch_key.text.shape.fill_colour = background;
    slider->slide.border_colour = slide_border;
    slider->slide.fill_colour = slide_background;
    slider->touch_key.alt_colour = alt_background;
}

void slider_redraw(slider_t* slider)
{
    touch_key_redraw(&slider->touch_key);
    draw_shape(&slider->slide, slider->slide_position);
}
