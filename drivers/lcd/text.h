
#include  <stdint.h>
#include "lcd.h"
#include "graphics.h"
#include "font_type.h"

#ifndef TEXT_H_
#define TEXT_H_


typedef enum {
    JUSTIFY_CENTRE = 0x00,
    JUSTIFY_LEFT = 0x01,
    JUSTIFY_RIGHT = 0x02,
    JUSTIFY_TOP = 0x04,
    JUSTIFY_BOTTOM = 0x08
}justify_t;


#define TEXT_DEFAULT_MARGIN_LEFT 4
#define TEXT_DEFAULT_MARGIN_RIGHT 4
#define TEXT_DEFAULT_MARGIN_TOP 4
#define TEXT_DEFAULT_MARGIN_BOTTOM 4


typedef struct {
    const char* buffer;
    colour_t colour;
    const font_t* font;
    justify_t justify;
    shape_t* shape;
} text_t;


void draw_textbox(text_t* text, point_t location);
void redraw_textbox_text(text_t* text, point_t location);
void redraw_textbox_background(text_t* text, point_t location);


void text_set_background_shape(text_t* text, shape_t* shape);
void text_set_justification(text_t* text, justify_t justification);
void text_set_font(text_t* text, const font_t* font);
void text_set_colour(text_t* text, colour_t colour);
void text_set_background_colour(text_t* text, colour_t colour);
void text_set_border_colour(text_t* text, colour_t colour);
void text_set_buffer(text_t* text, const char* str);
void text_blank_text(text_t* text, point_t location);
void text_update_text(text_t* text, const char* str, point_t location);

int16_t get_text_height(text_t* text);

point_t text_justify(text_t* text, point_t location);
point_t text_bounds(const char* str, const font_t* font);
void draw_raw_text(text_t* text, point_t location, bool blank);

#endif // TEXT_H_
