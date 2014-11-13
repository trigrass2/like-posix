
#include <string.h>
#include "graphics.h"
#include "text.h"

#ifndef PANEL_METER_H_
#define PANEL_METER_H_

typedef struct {
    int16_t length;
    const char* prescision;
    const char* units;
    const font_t* units_font;
    point_t location;
    shape_t background;
    text_t textbox;
} panel_meter_t;

void init_panel_meter(panel_meter_t* meter, char* buffer, int16_t length, bool rounded);
void draw_panel_meter(panel_meter_t* meter);
void update_panel_meter(panel_meter_t* meter, float value);
void panel_meter_set_units(panel_meter_t* meter, const char* units);
const char* panel_meter_get_units(panel_meter_t* meter);

#endif // PANEL_METER_H_
