
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