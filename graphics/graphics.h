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
 * This file is part of the Appleseed project, <https://github.com/drmetal/app-l-seed>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "lcd.h"
#include "system.h"

#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#define MAX_ALPHA 255

typedef uint16_t colour_t;
typedef uint8_t alpha_t;

typedef enum {
    WHITE = 0xFFFF,
    BLACK = 0x0000,
    RED = 0xF98A,
    GREEN = 0x07E0,
    BLUE = 0x001F,
    MAGENTA = 0xF81F,
    CYAN = 0x7FFF,
    YELLOW = 0xFFE0,

    VERY_DARK_GREY = 0x18E3,
    DARK_GREY = 0x4A49,
    MID_GREY = 0x8410,
    LIGHT_GREY = 0xA534,
    VERY_LIGHT_GREY = 0xD69A,

    VERY_DARK_BLUE = 0x01B8,
    DARK_BLUE = 0x135D,
    MID_BLUE = 0x4C9F,
    LIGHT_BLUE = 0x861F,
    VERY_LIGHT_BLUE = 0xC6FF,

    VERY_DARK_GREEN = 0x0280,
    DARK_GREEN = 0x03C4,
    MID_GREEN = 0x0505,
    LIGHT_GREEN = 0x3DEA,
    VERY_LIGHT_GREEN = 0x8FB1,

    VERY_DARK_ORANGE = 0xFA40,
    DARK_ORANGE = 0xFB20,
    MID_ORANGE = 0xFCA3,
    LIGHT_ORANGE = 0xFD28,
    VERY_LIGHT_ORANGE = 0xFE31,

    DARK_RED = 0xF804,
    PINK = 0xFAF3,
    VIOLET = 0x841F,
    PURPLE = 0x619F,
    SLATE = 0x534F,

} preset_colour_t;

typedef struct {
    int16_t x;
    int16_t y;
}point_t;


void graphics_init();
void set_background_colour(colour_t colour);
colour_t get_background_colour();
colour_t blend_colour(colour_t fgc, alpha_t fga, colour_t bgc, alpha_t bga);

#endif // GRAPHICS_H_
