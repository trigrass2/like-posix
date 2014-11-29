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


#include <time.h>
#include "statusbar.h"

#define STATUSBAR_SIZE          {STATUSBAR_SIZE_X, STATUSBAR_SIZE_Y}
#define STATUSBAR_ORIGIN        {STATUSBAR_ORIGIN_X, STATUSBAR_ORIGIN_Y}

#if STATUSBAR_INCLUDE_CLOCK
#define CLOCK_FORMAT_STR                    "%d:%02d:%02d %s"
#define CLOCK_BUFFER_SIZE                   24
#endif

typedef struct {
    text_t* text;
    TaskHandle_t statusbar_task_handle;
    point_t location;
#if STATUSBAR_INCLUDE_CLOCK
    char clock_buffer[CLOCK_BUFFER_SIZE];
    const char* clock_format;
#endif
}statusbar_t;

static void statusbar_task(void* pvParameters);

#if STATUSBAR_INCLUDE_CLOCK
void statusbar_update_clock();
#endif

static shape_t statusbar_shape = {
    .type=SQUARE,
    .fill_colour=STATUSBAR_BACKGROUND_COLOUR,
    .border_colour=STATUSBAR_BORDER_COLOUR,
    .fill=STATUSBAR_FILL,
    .size=STATUSBAR_SIZE,
    .radius=5,
};

static text_t statusbar_text = {
    .buffer=NULL,
    .font=&STATUSBAR_FONT,
    .colour=STATUSBAR_TEXT_COLOUR,
    .justify=JUSTIFY_CENTRE,
    .shape=&statusbar_shape
};

static statusbar_t statusbar = {
    .text=&statusbar_text,
    .statusbar_task_handle=NULL,
    .location=STATUSBAR_ORIGIN,
#if STATUSBAR_INCLUDE_CLOCK
    .clock_format=CLOCK_FORMAT_STR,
#endif
};

void statusbar_init()
{
#if STATUSBAR_INCLUDE_CLOCK
//    RTC_Configuration();
#endif
    statusbar.clock_buffer[0] = '\0';
    assert_true(xTaskCreate(statusbar_task,
                             "statusbar",
                             configMINIMAL_STACK_SIZE + STATUSBAR_TASK_STACK_SIZE,
                             &statusbar,
                             tskIDLE_PRIORITY + STATUSBAR_TASK_PRIORITY,
                             &statusbar.statusbar_task_handle));
}

#if STATUSBAR_INCLUDE_CLOCK
void statusbar_update_clock()
{
    const char* ampm;
    text_set_buffer(statusbar.text, statusbar.clock_buffer);
    text_set_justification(statusbar.text, CLOCK_JUSTIFICATION);

    time_t t = time(NULL);
    struct tm* lt = localtime(&t);

    ampm = lt->tm_hour > 11 ? "pm" : "am";

    text_blank_text(statusbar.text, statusbar.location);
    snprintf((char*)statusbar.clock_buffer, sizeof(statusbar.clock_buffer)-1, statusbar.clock_format, lt->tm_hour%12, lt->tm_min, lt->tm_sec, ampm);
    redraw_textbox_text(statusbar.text, statusbar.location);
}
#endif

void statusbar_task(void* pvParameters)
{
    (void)pvParameters;
    portTickType last_execution_time = xTaskGetTickCount();
#if STATUSBAR_INCLUDE_LINK_DATA_ICONS
    unsigned long sent = 0;
    unsigned long received = 0;
    bool up = true;
    bool down = true;
#endif
    uint16_t icon_offset;

    redraw_textbox_background(statusbar.text, statusbar.location);

    for(;;)
    {
        vTaskDelayUntil(&last_execution_time, STATUSBAR_UPDATE_RATE/portTICK_RATE_MS);

        icon_offset = statusbar.location.x + STATUSBAR_GAP;

#if STATUSBAR_INCLUDE_CLOCK
        statusbar_update_clock();
#endif

#if STATUSBAR_INCLUDE_SD_STATUS_ICON
        if(sdfs_ready())
        {
            draw_image(&micro_sd_blue_aa, (point_t){icon_offset, statusbar.location.y + SD_STATUS_Y_GAP});
            icon_offset += micro_sd_blue_aa.width;
        }
        else
        {
            draw_image(&micro_sd_gray_aa, (point_t){icon_offset, statusbar.location.y + SD_STATUS_Y_GAP});
            icon_offset += micro_sd_gray_aa.width;
        }
#endif
        icon_offset += STATUSBAR_GAP;

#if STATUSBAR_INCLUDE_LINK_DATA_ICONS
        if(net_ip_packets_sent() > sent)
        {
            sent = net_ip_packets_sent();
            if(!up)
            {
                up = true;
                draw_image(&up_bright_aa, (point_t){icon_offset, statusbar.location.y + LINK_DATA_Y_GAP});
            }
        }
        else if(up)
        {
            up = false;
            draw_image(&up_gray_aa, (point_t){icon_offset, statusbar.location.y + LINK_DATA_Y_GAP});
        }

        icon_offset += image_width(up_gray_aa);

        if(net_ip_packets_received() > received)
        {
            received = net_ip_packets_received();
            if(!down)
            {
                down = true;
                draw_image(&down_bright_aa, (point_t){icon_offset, (statusbar.location.y + STATUSBAR_SIZE_Y) - image_height(down_bright_aa) - LINK_DATA_Y_GAP});
            }
        }
        else if(down)
        {
            down = false;
            draw_image(&down_gray_aa, (point_t){icon_offset, (statusbar.location.y + STATUSBAR_SIZE_Y) - image_height(down_gray_aa) - LINK_DATA_Y_GAP});
        }

        icon_offset += image_width(down_gray_aa);
        icon_offset += STATUSBAR_GAP;

#endif

        taskYIELD();
    }
}
