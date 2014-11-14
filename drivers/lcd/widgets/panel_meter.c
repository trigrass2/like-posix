#include "panel_meter.h"

/**
 * initialises and draws an LED panel meter style numeric display.
 *
 * the panel meter structure needs to be partially populated first:
 *
 *  // example buffer and structure definition:
 *  // number of digits = size of buffer-2,
 *  // for decimal point and null terminator
 *  uint8_t meter_buffer[16];
 *  panel_meter_t temperature = {
 *      .prescision = (const uint8_t*)"%.2f",   // floating point display,to 2 decimal places
 *      .location = {0, 0},                     // meter top left located at 0, 0
 *      .units = (const uint8_t*)"degrees",     // display unit `degrees`
 *      .units_font = &Ubuntu_32,               // display unit font
 *      .background = {
 *          .fill_colour=FOREST_GREEN,          // colour of background
 *          .border_colour=FOREST_GREEN,        // colour of border
 *          .size={320, 96}                     // size of background, make x long enough to fit the digits
 *      },                                      // make y the same height as the display font
 *      .textbox = {
 *          .font=&Digital_7_Italic_96,         // display font
 *          .colour=FOREST_GREEN                // display text colour
 *      }
 *  };
 *  // example initialization, a temperature meter with rounded corners
 *  init_panel_meter(&temperature, buffer_buffer, sizeof(meter_buffer), true);
 *
 */
void init_panel_meter(panel_meter_t* meter, char* buffer, int16_t length, bool rounded)
{
    // draw background with units
    meter->length = length;
    meter->length = length;
    meter->background.type = SQUARE;
    meter->background.fill = true;
    if(rounded)
        meter->background.radius = 8;
    else
        meter->background.radius = 0;
    text_set_buffer(&meter->textbox, buffer);
    text_set_background_shape(&meter->textbox, &meter->background);
    memset(buffer, '\0', meter->length);
    draw_panel_meter(meter);
}

/**
 * redraws the whole panel meter. the panel meter structure
 * must be initialized with init_panel_meter() before this function is safe to use.
 */
void draw_panel_meter(panel_meter_t* meter)
{
    const char* buffer = meter->textbox.buffer;
    const font_t* font = meter->textbox.font;

    text_set_justification(&meter->textbox, JUSTIFY_BOTTOM|JUSTIFY_RIGHT);
    text_set_buffer(&meter->textbox, meter->units);
    text_set_font(&meter->textbox, meter->units_font);
    draw_textbox(&meter->textbox, meter->location);

    // reset font / buffer for data text
    text_set_buffer(&meter->textbox, buffer);
    text_set_font(&meter->textbox, font);
    text_set_justification(&meter->textbox, JUSTIFY_LEFT);
}

/**
 * updates the the text part of the panel meter. the panel meter structure
 * must be initialized with init_panel_meter() before this function is safe to use.
 */
void update_panel_meter(panel_meter_t* meter, float value)
{
    text_blank_text(&meter->textbox, meter->location);
    snprintf((char*)meter->textbox.buffer, meter->length-1, meter->prescision, (double)value);
    redraw_textbox_text(&meter->textbox, meter->location);
}

void panel_meter_set_units(panel_meter_t* meter, const char* units)
{
    const char* buffer = meter->textbox.buffer;
    const font_t* font = meter->textbox.font;
    text_set_justification(&meter->textbox, JUSTIFY_BOTTOM|JUSTIFY_RIGHT);
    text_set_buffer(&meter->textbox, meter->units);
    text_set_font(&meter->textbox, meter->units_font);
    text_blank_text(&meter->textbox, meter->location);
    meter->units = units;
    text_set_buffer(&meter->textbox, meter->units);
    redraw_textbox_text(&meter->textbox, meter->location);
    // reset font / buffer for data text
    text_set_buffer(&meter->textbox, buffer);
    text_set_font(&meter->textbox, font);
    text_set_justification(&meter->textbox, JUSTIFY_LEFT);
}

const char* panel_meter_get_units(panel_meter_t* meter)
{
    return meter->units;
}
