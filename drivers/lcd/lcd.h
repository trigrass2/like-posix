
/**
 * Compatible list:
 * ILI9320_DEVICE_ID (ili9320 LGDP4532)
 * ILI9320_DEVICE_ID (ili9325)
 *
 * Maybe supported but not tested: ili9328 LGDP4531
 */

#include <stdio.h>
#include <stdint.h>

#include "stm32f10x.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"

#include "lcd_config.h"
#include "ili932x.h"
#include "system.h"


#ifndef LCD_H_
#define LCD_H_

/**
 * display rotation
 */
#define ROTATE_0_DEGREES                    1
#define ROTATE_90_DEGREES                   2
#define ROTATE_180_DEGREES                  3
#define ROTATE_270_DEGREES                  4


#ifndef LCD_DRIVER_ID
// defined in lcd_config.h
// #define LCD_DRIVER_ID ILI9320_DEVICE_ID
#endif
#ifndef LCD_ROTATION
// defined in lcd_config.h
// #define LCD_ROTATION  ROTATE_0_DEGREES
#endif

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

#define LCD_RED_MASK	0xF800
#define LCD_GREEN_MASK	0x07E0
#define LCD_BLUE_MASK	0x001F

#define LCD_RED_MAX		32
#define LCD_GREEN_MAX	64
#define LCD_BLUE_MAX	32

#define LCD_RED_SHIFT	11
#define LCD_GREEN_SHIFT	5
#define LCD_BLUE_SHIFT	0


/**
 * returns the value of the individual colour words given a combined colour word.
 */
#define LCD_RED_BYTE(colour)                ((colour & LCD_RED_MASK) >> LCD_RED_SHIFT)
#define LCD_GREEN_BYTE(colour)              ((colour & LCD_GREEN_MASK) >> LCD_GREEN_SHIFT)
#define LCD_BLUE_BYTE(colour)               (colour & LCD_BLUE_MASK)

 /**
  * inserts colour data back into a colour word.
  */
#define LCD_INSERT_RED_BYTE(colour, red)    (colour |= red << LCD_RED_SHIFT)
#define LCD_INSERT_GREEN_BYTE(colour, green)    (colour |= green << LCD_GREEN_SHIFT)
#define LCD_INSERT_BLUE_BYTE(colour, blue)  (colour |= blue)


#if LCD_DRIVER_ID == LGDP4532_DEVICE_ID

#if LCD_ROTATION == ROTATE_0_DEGREES
#define LCD_WIDTH                           240
#define LCD_HEIGHT                          320
#define LCD_Y_SCAN_CONTROL_REG_VALUE        0x00
#define LCD_X_SCAN_CONTROL_REG_VALUE        ILI932x_GATE_SCAN_CTRL1_GS
#define LCD_ENTRYMODE_AM_VALUE              0x00
#define LCD_SET_CURSOR_X                    ILI932x_H_GRAM_ADDR_SET
#define LCD_SET_CURSOR_Y                    ILI932x_V_GRAM_ADDR_SET
#define LCD_SET_WINDOW_X1                   ILI932x_HSA
#define LCD_SET_WINDOW_Y1                   ILI932x_VSA
#define LCD_SET_WINDOW_X2                   ILI932x_HEA
#define LCD_SET_WINDOW_Y2                   ILI932x_VEA
#elif LCD_ROTATION == ROTATE_90_DEGREES
#define LCD_WIDTH                           320
#define LCD_HEIGHT                          240
#define LCD_Y_SCAN_CONTROL_REG_VALUE        ILI932x_DRIVER_OUTPUT_CTRL_SS
#define LCD_X_SCAN_CONTROL_REG_VALUE        ILI932x_GATE_SCAN_CTRL1_GS
#define LCD_ENTRYMODE_AM_VALUE              ILI932x_EM_AM
#define LCD_SET_CURSOR_X                    ILI932x_V_GRAM_ADDR_SET
#define LCD_SET_CURSOR_Y                    ILI932x_H_GRAM_ADDR_SET
#define LCD_SET_WINDOW_X1                   ILI932x_VSA
#define LCD_SET_WINDOW_Y1                   ILI932x_HSA
#define LCD_SET_WINDOW_X2                   ILI932x_VEA
#define LCD_SET_WINDOW_Y2                   ILI932x_HEA
#elif LCD_ROTATION == ROTATE_180_DEGREES
#define LCD_WIDTH                           240
#define LCD_HEIGHT                          320
#define LCD_Y_SCAN_CONTROL_REG_VALUE        ILI932x_DRIVER_OUTPUT_CTRL_SS
#define LCD_X_SCAN_CONTROL_REG_VALUE        0x00
#define LCD_ENTRYMODE_AM_VALUE              0x00
#define LCD_SET_CURSOR_X                    ILI932x_H_GRAM_ADDR_SET
#define LCD_SET_CURSOR_Y                    ILI932x_V_GRAM_ADDR_SET
#define LCD_SET_WINDOW_X1                   ILI932x_HSA
#define LCD_SET_WINDOW_Y1                   ILI932x_VSA
#define LCD_SET_WINDOW_X2                   ILI932x_HEA
#define LCD_SET_WINDOW_Y2                   ILI932x_VEA
#elif LCD_ROTATION == ROTATE_270_DEGREES
#define LCD_WIDTH                           320
#define LCD_HEIGHT                          240
#define LCD_Y_SCAN_CONTROL_REG_VALUE        0x00
#define LCD_X_SCAN_CONTROL_REG_VALUE        0x00
#define LCD_ENTRYMODE_AM_VALUE              ILI932x_EM_AM
#define LCD_SET_CURSOR_X                    ILI932x_V_GRAM_ADDR_SET
#define LCD_SET_CURSOR_Y                    ILI932x_H_GRAM_ADDR_SET
#define LCD_SET_WINDOW_X1                   ILI932x_VSA
#define LCD_SET_WINDOW_Y1                   ILI932x_HSA
#define LCD_SET_WINDOW_X2                   ILI932x_VEA
#define LCD_SET_WINDOW_Y2                   ILI932x_HEA
#endif

#elif LCD_DRIVER_ID == ILI9325_DEVICE_ID

#if LCD_ROTATION == ROTATE_0_DEGREES
#define LCD_WIDTH                           240
#define LCD_HEIGHT                          320
#define LCD_Y_SCAN_CONTROL_REG_VALUE        0x00
#define LCD_X_SCAN_CONTROL_REG_VALUE        0x00
#define LCD_ENTRYMODE_AM_VALUE              0x00
#define LCD_SET_CURSOR_X                    ILI932x_H_GRAM_ADDR_SET
#define LCD_SET_CURSOR_Y                    ILI932x_V_GRAM_ADDR_SET
#define LCD_SET_WINDOW_X1                   ILI932x_HSA
#define LCD_SET_WINDOW_Y1                   ILI932x_VSA
#define LCD_SET_WINDOW_X2                   ILI932x_HEA
#define LCD_SET_WINDOW_Y2                   ILI932x_VEA
#elif LCD_ROTATION == ROTATE_90_DEGREES
#define LCD_WIDTH                           320
#define LCD_HEIGHT                          240
#define LCD_Y_SCAN_CONTROL_REG_VALUE        ILI932x_DRIVER_OUTPUT_CTRL_SS
#define LCD_X_SCAN_CONTROL_REG_VALUE        0x00
#define LCD_ENTRYMODE_AM_VALUE              ILI932x_EM_AM
#define LCD_SET_CURSOR_X                    ILI932x_V_GRAM_ADDR_SET
#define LCD_SET_CURSOR_Y                    ILI932x_H_GRAM_ADDR_SET
#define LCD_SET_WINDOW_X1                   ILI932x_VSA
#define LCD_SET_WINDOW_Y1                   ILI932x_HSA
#define LCD_SET_WINDOW_X2                   ILI932x_VEA
#define LCD_SET_WINDOW_Y2                   ILI932x_HEA
#elif LCD_ROTATION == ROTATE_180_DEGREES
#define LCD_WIDTH                           240
#define LCD_HEIGHT                          320
#define LCD_Y_SCAN_CONTROL_REG_VALUE        ILI932x_DRIVER_OUTPUT_CTRL_SS
#define LCD_X_SCAN_CONTROL_REG_VALUE        ILI932x_GATE_SCAN_CTRL1_GS
#define LCD_ENTRYMODE_AM_VALUE              0x00
#define LCD_SET_CURSOR_X                    ILI932x_H_GRAM_ADDR_SET
#define LCD_SET_CURSOR_Y                    ILI932x_V_GRAM_ADDR_SET
#define LCD_SET_WINDOW_X1                   ILI932x_HSA
#define LCD_SET_WINDOW_Y1                   ILI932x_VSA
#define LCD_SET_WINDOW_X2                   ILI932x_HEA
#define LCD_SET_WINDOW_Y2                   ILI932x_VEA
#elif LCD_ROTATION == ROTATE_270_DEGREES
#define LCD_WIDTH                           320
#define LCD_HEIGHT                          240
#define LCD_Y_SCAN_CONTROL_REG_VALUE        0x00
#define LCD_X_SCAN_CONTROL_REG_VALUE        ILI932x_GATE_SCAN_CTRL1_GS
#define LCD_ENTRYMODE_AM_VALUE              ILI932x_EM_AM
#define LCD_SET_CURSOR_X                    ILI932x_V_GRAM_ADDR_SET
#define LCD_SET_CURSOR_Y                    ILI932x_H_GRAM_ADDR_SET
#define LCD_SET_WINDOW_X1                   ILI932x_VSA
#define LCD_SET_WINDOW_Y1                   ILI932x_HSA
#define LCD_SET_WINDOW_X2                   ILI932x_VEA
#define LCD_SET_WINDOW_Y2                   ILI932x_HEA
#endif
#endif


/**
 * lcd IO macros.
 */
#define LCD_REG                             (*((volatile unsigned short *) LCD_REG_IO_ADDRESS))
#define LCD_RAM                             (*((volatile unsigned short *) LCD_RAM_IO_ADDRESS))
#define write_cmd(cmd)                      LCD_REG = cmd
#define write_data(data)                    LCD_RAM = data
#define write_reg(reg, value)               write_cmd(reg);write_data(value)
#define read_data()                         LCD_RAM
#define read_reg(reg, result)               write_cmd(reg);result = read_data()


/**
 * lcd control macros.
 */
#define lcd_set_cursor_x(x)                 write_reg(LCD_SET_CURSOR_X, x)
#define lcd_set_cursor_y(y)                 write_reg(LCD_SET_CURSOR_Y, y)
#define lcd_set_cursor(x, y)                lcd_set_cursor_y(y);lcd_set_cursor_x(x)
#define lcd_set_window(x, y, w, h)          write_reg(LCD_SET_WINDOW_X1, x); \
                                            write_reg(LCD_SET_WINDOW_Y1, y); \
                                            write_reg(LCD_SET_WINDOW_X2, x+(w-1)); \
                                            write_reg(LCD_SET_WINDOW_Y2, y+(h-1))
#define lcd_rw_gram()                       write_cmd(ILI932x_CMD_WRITE_GRAM)
#define lcd_set_entry_mode_normal()         write_reg(ILI932x_ENTRY_MODE, LCD_ENTRYMODE_AM_VALUE|ILI932x_EM_65K|ILI932x_EM_VERTICAL_INC|ILI932x_EM_HORIZONTAL_INC|ILI932x_EM_BGR); \
                                            lcd_set_window(0, 0, LCD_WIDTH, LCD_HEIGHT);
#define lcd_set_entry_mode_window()         write_reg(ILI932x_ENTRY_MODE, LCD_ENTRYMODE_AM_VALUE|ILI932x_EM_65K|ILI932x_EM_VERTICAL_INC|ILI932x_EM_HORIZONTAL_INC|ILI932x_EM_BGR|ILI932x_EM_ORG);

/**
 * lock for non atomic display operations.
 */
extern SemaphoreHandle_t xLcdMutex;
#define LCD_LOCK() assert_param(xSemaphoreTake(xLcdMutex, 1000/portTICK_RATE_MS))
#define LCD_UNLOCK() xSemaphoreGive(xLcdMutex)


void lcd_init(void);
unsigned int lcd_getdeviceid(void);
void lcd_backlight(uint8_t enable);


#endif // LCD_H_
