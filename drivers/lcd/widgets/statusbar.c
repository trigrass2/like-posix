

#include "statusbar.h"

#define CLOCK_BUFFER_SIZE   24
#define STATUSBAR_SIZE          {STATUSBAR_SIZE_X, STATUSBAR_SIZE_Y}
#define STATUSBAR_ORIGIN        {STATUSBAR_ORIGIN_X, STATUSBAR_ORIGIN_Y}

#if STATUSBAR_INCLUDE_CLOCK
#define CLOCK_FORMAT_STR                    "%d:%02d:%02d %s"
#define CLOCK_BUFFER_SIZE                   13
#endif

typedef struct {
    text_t* text;
    TaskHandle_t statusbar_task_handle;
    point_t location;
    uint8_t clock_buffer[CLOCK_BUFFER_SIZE];
#if STATUSBAR_INCLUDE_CLOCK
    const uint8_t* clock_format;
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
    .clock_format=(const uint8_t*)CLOCK_FORMAT_STR,
#endif
};

void statusbar_init()
{
#if STATUSBAR_INCLUDE_CLOCK
    RTC_Configuration();
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
    uint32_t hours;
    uint32_t minutes;
    uint32_t seconds;
    const uint8_t* ampm;
    text_set_buffer(statusbar.text, statusbar.clock_buffer);
    text_set_justification(statusbar.text, CLOCK_JUSTIFICATION);

    RTC_GetTime(&hours, &minutes, &seconds);
    ampm = hours > 11 ? (const uint8_t*)"pm" : (const uint8_t*)"am";
    hours %= 12;

    text_blank_text(statusbar.text, statusbar.location);
    snprintf((char*)statusbar.clock_buffer, sizeof(statusbar.clock_buffer)-1, (const char*)statusbar.clock_format, hours, minutes, seconds, ampm);
    redraw_textbox_text(statusbar.text, statusbar.location);
}
#endif

void statusbar_task(void* pvParameters)
{
    (void)pvParameters;
    portTickType last_execution_time = xTaskGetTickCount();
#if STATUSBAR_INCLUDE_LINK_DATA_ICONS
    uip_stats_t sent = 0;
    uip_stats_t received = 0;
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
