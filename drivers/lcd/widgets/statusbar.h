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

#include <stdio.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "graphics.h"
#include "text.h"

#include "statusbar-conf.h"

#if STATUSBAR_INCLUDE_CLOCK
#include "drtc.h"
#endif

#if STATUSBAR_INCLUDE_LINK_DATA_ICONS
#include "net.h"
#endif

#if STATUSBAR_INCLUDE_SD_STATUS_ICON
#include "sdfs.h"
#endif

#ifndef STATUSBAR_H_
#define STATUSBAR_H_

#define STATUSBAR_TASK_STACK_SIZE           200
#define STATUSBAR_TASK_PRIORITY             2
#define STATUSBAR_UPDATE_RATE               1000
#define STATUSBAR_GAP                       8

#define STATUSBAR_SIZE_X                    LCD_WIDTH
#define STATUSBAR_SIZE_Y                    32

#define STATUSBAR_ORIGIN_X                  LCD_WIDTH - STATUSBAR_SIZE_X
#define STATUSBAR_ORIGIN_Y                  0

#define STATUSBAR_FONT                      Ubuntu_20
#define STATUSBAR_TEXT_COLOUR               VERY_LIGHT_GREY
#define STATUSBAR_BORDER_COLOUR             LIGHT_GREY
#define STATUSBAR_BACKGROUND_COLOUR         BLACK
#define STATUSBAR_FILL                      false

#if STATUSBAR_INCLUDE_CLOCK
#define CLOCK_JUSTIFICATION                 JUSTIFY_RIGHT
#endif

#if STATUSBAR_INCLUDE_LINK_DATA_ICONS
#define LINK_DATA_Y_GAP                     3
#endif

#if STATUSBAR_INCLUDE_SD_STATUS_ICON
#define SD_STATUS_Y_GAP                     2
#endif

void statusbar_init();

#endif // STATUSBAR_H_
