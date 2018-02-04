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

#include <unistd.h>
#include "FontAwesome_32.h"
#include "touch_cal.h"
#include "touch_key.h"
#include "confparse.h"
#include "logger.h"

static const char plus_icon[2] = {23,0};

#define CAL_BUTTON_SIZE 80
#define CAL_CORRECT_X (CAL_BUTTON_SIZE/2)
#define CAL_CORRECT_Y (CAL_BUTTON_SIZE/2)

void touch_callback(touch_key_t* key)
{
	touch_cal_t* tc = (touch_cal_t*)touch_key_get_appdata(key);

    if(touch_key_press_is(key, KEY_UP)) {
    	if(key == &tc->touch_keyok) {
    		tc->done = true;
    	}
    	else if(!tc->done){
    		if(key == &tc->touch_key_a) {
				tc->point_a = touch_key_get_point(key);
				printf("point_a %d %d\n", tc->point_a.x, tc->point_a.y);
			}
			else if(key == &tc->touch_key_b) {
				tc->point_b = touch_key_get_point(key);
				printf("point_b %d %d\n", tc->point_b.x, tc->point_b.y);
			}
    	}
    }
}

void touch_cal_start(touch_cal_t* tc)
{
	point_t position;
	point_t size;

	tc->done = false;

	position.x = 110;
	position.y = 90;
	size.x = 100;
	size.y = 60;
    touch_key_init(&tc->touch_keyok, position, size, "Done", 8);
    touch_key_add(&tc->touch_keyok, (touch_callback_t)touch_callback, tc);
    touch_key_set_colour(&tc->touch_keyok, WHITE, BLACK, WHITE, YELLOW);
    touch_key_enable(&tc->touch_keyok, true);

	position.x = 0;
	position.y = 0;
	size.x = CAL_BUTTON_SIZE;
	size.y = CAL_BUTTON_SIZE;
    touch_key_init(&tc->touch_key_a, position, size, plus_icon, 8);
    touch_key_add(&tc->touch_key_a, (touch_callback_t)touch_callback, tc);
    touch_key_set_font(&tc->touch_key_a, &FontAwesome_32);
    touch_key_set_colour(&tc->touch_key_a, BLACK, BLACK, VERY_DARK_GREY, WHITE);
    touch_key_enable(&tc->touch_key_a, true);

	position.x = LCD_WIDTH-CAL_BUTTON_SIZE;
	position.y = LCD_HEIGHT-CAL_BUTTON_SIZE;
    touch_key_init(&tc->touch_key_b, position, size, plus_icon, 8);
    touch_key_add(&tc->touch_key_b, (touch_callback_t)touch_callback, tc);
    touch_key_set_font(&tc->touch_key_b, &FontAwesome_32);
    touch_key_set_colour(&tc->touch_key_b, BLACK, BLACK, VERY_DARK_GREY, WHITE);
    touch_key_enable(&tc->touch_key_b, true);

    while(!tc->done) {
    	usleep(100000);
    }

    /**
     * to translate tp value V to the corresponding tft pixel V':
 	 *
 	 *		V' = mV + c
     *
     * if
     * 		x is tp measurement A and X is tp measurment B
     * and
     * 		x' is the corresponding tft pixel A and X' is the corresponding tft pixel B
     *
     * m = (X' - x') / (X - x)
     * c = x' - mx
     */

    float mx = ((float)(LCD_WIDTH-CAL_CORRECT_X-CAL_CORRECT_X)) / ((float)tc->point_b.x - (float)tc->point_a.x);
    float cx = (float)CAL_CORRECT_X - (mx * (float)tc->point_a.x);
    float my = ((float)(LCD_HEIGHT-CAL_CORRECT_Y-CAL_CORRECT_Y)) / ((float)tc->point_b.y - (float)tc->point_a.y);
    float cy = CAL_CORRECT_Y - (my * tc->point_a.y);

    save_cal("/etc/display/touch/config", mx, cx, my, cy);
    touch_panel_cal(mx, cx, my, cy);

    touch_key_remove(&tc->touch_key_a);
    touch_key_remove(&tc->touch_key_b);
    touch_key_remove(&tc->touch_keyok);
}



void save_cal(const char* file, float mx, float cx, float my, float cy)
{
	uint8_t buffer[32];
	char value[32];

	sprintf(value, "%.3f", (double)mx);
	if(! add_config_entry(buffer, 32, (const uint8_t*)file, (const uint8_t*)"mx", (const uint8_t*)value)) {
		log_error(NULL, "failed to add mx config to %s", file);
	}
	sprintf(value, "%.3f", (double)cx);
	if(! add_config_entry(buffer, 32, (const uint8_t*)file, (const uint8_t*)"cx", (const uint8_t*)value)) {
		log_error(NULL, "failed to add cx config to %s", file);
	}
	sprintf(value, "%.3f", (double)my);
	if(! add_config_entry(buffer, 32, (const uint8_t*)file, (const uint8_t*)"my", (const uint8_t*)value)) {
		log_error(NULL, "failed to add my config to %s", file);
	}
	sprintf(value, "%.3f", (double)cy);
	if(! add_config_entry(buffer, 32, (const uint8_t*)file, (const uint8_t*)"cy", (const uint8_t*)value)) {
		log_error(NULL, "failed to add cy config to %s", file);
	}
}

void load_cal(const char* file)
{
	//config_parser_t cp;


}
