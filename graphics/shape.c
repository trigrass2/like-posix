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
 * This file is part of the lollyjar project, <https://github.com/drmetal/lollyjar>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

#include "shape.h"

#define CIRCLE_QUAD_1 2 	///< top right
#define CIRCLE_QUAD_2 1 	///< top left
#define CIRCLE_QUAD_3 8 	///< bottom left
#define CIRCLE_QUAD_4 4 	///< bottom right

#define swap(a, b) { uint16_t t = a; a = b; b = t; }
#define sign(x) (x > 0) ? 1 : (x < 0) ? -1 : 0


static void draw_pixel(int16_t x, int16_t y, colour_t colour);
static void draw_line_h(int16_t x, int16_t y, uint16_t length, colour_t colour);
static void draw_line_v(int16_t x, int16_t y, uint16_t length, colour_t colour);
static void draw_line(point_t location, point_t size,  colour_t colour);
static void draw_circle_quadrant(int16_t x, int16_t y, uint16_t radius, uint8_t quadrant, colour_t fill_colour, colour_t border_colour, uint16_t delta, bool fill);
static void draw_circle(point_t location, uint16_t radius, colour_t fill_colour, colour_t border_colour, bool fill);
static void draw_rectangle(point_t location, point_t size, uint16_t radius, colour_t fill_colour, colour_t border_colour, bool fill);

/**
 * draws the shape as specified in a shape_t structure, at location.
 *
 * filled shapes:
 *      set fill to true.
 *      to have no border, set the border colour to the fill colour.
 *      to have an border, set the border colour to something other than the fill colour.
 * unfilled shapes:
 *      set fill to false.
 *      to have no border, set the border colour to the fill colour.
 *      to have an border, set the border colour to something other than the fill colour.
 * shape member radius: sets the radius of circles or the radius of corner rounding on other shapes.
 * shape member size: sets the size of the shape's bounding box.
 */
void draw_shape(shape_t* shape, point_t location)
{
	LCD_LOCK();

	switch(shape->type)
	{
		case SQUARE:
			draw_rectangle(location, shape->size, shape->radius, shape->fill_colour, shape->border_colour, shape->fill);
		break;

		case CIRCLE:
			draw_circle(location, shape->radius, shape->fill_colour, shape->border_colour, shape->fill);
		break;

        case LINE:
            draw_line(location, shape->size, shape->fill_colour);
        break;

        case DOT:
            lcd_set_cursor(location.x, location.y);
            lcd_rw_gram();
            write_data(shape->fill_colour);
        break;
	}

    LCD_UNLOCK();
}

void set_shape_size(shape_t* shape, point_t size)
{
    shape->size = size;
}

/*************************************************
 *
 * Not Thread Safe API
 *
 **************************************************/

void draw_pixel(int16_t x, int16_t y, colour_t colour)
{
    lcd_set_cursor(x, y);
    lcd_rw_gram();
    write_data(colour);
}

void draw_line_h(int16_t x, int16_t y, uint16_t length, colour_t colour)
{
	lcd_set_cursor(x, y);
	lcd_rw_gram();
	while(length--)
		write_data(colour);
}

void draw_line_v(int16_t x, int16_t y, uint16_t length, colour_t colour)
{
	uint16_t yy = y;
	y += length;
	lcd_set_cursor_x(x);
	while(yy < y)
	{
		lcd_set_cursor_y(yy);
		lcd_rw_gram();
		write_data(colour);
		yy++;
	}
}

/**
 * Bresenhem Algorithm a-la wikipedia
 *
 *function line(x0, y0, x1, y1)
 *  dx := abs(x1-x0)
 *  dy := abs(y1-y0)
 *  if x0 < x1 then sx := 1 else sx := -1
 *  if y0 < y1 then sy := 1 else sy := -1
 *  err := dx-dy
 *  loop
 *   plot(x0,y0)
 *   if x0 = x1 and y0 = y1 exit loop
 *   e2 := 2*err
 *   if e2 > -dy then
 *     err := err - dy
 *     x0 := x0 + sx
 *   end if
 *   if x0 = x1 and y0 = y1 then
 *     plot(x0,y0)
 *     exit loop
 *   end if
 *   if e2 <  dx then
 *     err := err + dx
 *     y0 := y0 + sy
 *   end if
 *  end loop
 */
static void draw_line(point_t location, point_t size,  colour_t colour)
{
    int16_t x1 = location.x + size.x;
    int16_t y1 = location.y + size.y;
    int16_t dx = abs(x1-location.x);
    int16_t dy = -abs(y1-location.y);
    int16_t err = dx + dy;
    int8_t sx = location.x < x1 ? 1 : -1;
    int8_t sy = location.y < y1 ? 1 : -1;
    int16_t err2;

    while(1)
    {
        draw_pixel(location.x, location.y, colour);

        if(location.x == x1 && location.y == y1)
            break;

        err2 = err<<2;

        if(err2 > dy)
        {
            err += dy;
            location.x += sx;
        }

        if(location.x == x1 && location.y == y1)
        {
            draw_pixel(location.x, location.y, colour);
            break;
        }

        if(err2 < dx)
        {
            err += dx;
            location.y += sy;
        }
    }
}

void draw_rectangle(point_t location, point_t size, uint16_t radius, colour_t fill_colour, colour_t border_colour, bool fill)
{
	uint16_t y;
	if(fill)
	{
        lcd_set_cursor(0, 0);
        lcd_set_window(location.x + radius, location.y, size.x - (2 * radius), size.y);
        lcd_set_entry_mode_window();
        lcd_rw_gram();
		for(y = 0; y < size.y * (size.x - (2 * radius)); y++)
	    	write_data(fill_colour);
        lcd_set_entry_mode_normal();

	    if(radius)
	    {
		    draw_circle_quadrant(location.x + size.x - (radius * 2) - 1, location.y, radius, 1, fill_colour, border_colour, size.y - 2 * radius - 1, true);
		    draw_circle_quadrant(location.x, location.y, radius, 2, fill_colour, border_colour, size.y - 2 * radius - 1, true);
		}
	}

	if(border_colour != fill_colour)
	{
		draw_line_h(location.x + radius, location.y, size.x - (2 * radius), border_colour);
		draw_line_h(location.x + radius, location.y + size.y - 1, size.x - (2 * radius), border_colour);
		draw_line_v(location.x, location.y + radius, size.y - (2 * radius), border_colour);
		draw_line_v(location.x + size.x - 1, location.y + radius, size.y - (2 * radius), border_colour);

	    draw_circle_quadrant(location.x, location.y, radius, CIRCLE_QUAD_2, fill_colour, border_colour, 0, false);
	    draw_circle_quadrant(location.x+size.x-(2 * radius)-1, location.y, radius, CIRCLE_QUAD_1, fill_colour, border_colour, 0, false);
	    draw_circle_quadrant(location.x+size.x-(2 * radius)-1, location.y+size.y-(2 * radius)-1, radius, CIRCLE_QUAD_4, fill_colour, border_colour, 0, false);
	    draw_circle_quadrant(location.x, location.y+size.y-(2 * radius)-1, radius, CIRCLE_QUAD_3, fill_colour, border_colour, 0, false);
	}
}

void draw_circle_quadrant(int16_t x, int16_t y, uint16_t radius, uint8_t quadrant, colour_t fill_colour, colour_t border_colour, uint16_t delta, bool fill)
{
    int16_t f = 1 - radius;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * radius;
    int16_t x_offset = 0;
    int16_t y_offset = radius;

    // this puts the origin of the circle at the top left corner of the bounding square
    x += radius;
    y += radius;

    while (x_offset<y_offset)
    {
        if(f >= 0)
        {
            y_offset--;
            ddF_y += 2;
            f += ddF_y;
        }

        x_offset++;
        ddF_x += 2;
        f += ddF_x;

        if(fill)
        {
	        if (quadrant & 0x1)
	        {
	            draw_line_v(x + x_offset, y - y_offset, 2 * y_offset + 1 + delta, fill_colour);
	            draw_line_v(x + y_offset, y - x_offset, 2 * x_offset + 1 + delta, fill_colour);
	        }
	        if (quadrant & 0x2)
	        {
	            draw_line_v(x - x_offset, y - y_offset, 2 * y_offset + 1 + delta, fill_colour);
	            draw_line_v(x - y_offset, y - x_offset, 2 * x_offset + 1 + delta, fill_colour);
	        }
	    }

        if(border_colour != fill_colour)
	    {
	        if (quadrant & 0x4) {
	            draw_pixel(x + x_offset, y + y_offset, border_colour);
	            draw_pixel(x + y_offset, y + x_offset, border_colour);
	        }
	        if (quadrant & 0x2) {
	            draw_pixel(x + x_offset, y - y_offset, border_colour);
	            draw_pixel(x + y_offset, y - x_offset, border_colour);
	        }
	        if (quadrant & 0x8) {
	            draw_pixel(x - y_offset, y + x_offset, border_colour);
	            draw_pixel(x - x_offset, y + y_offset, border_colour);
	        }
	        if (quadrant & 0x1) {
	            draw_pixel(x - y_offset, y - x_offset, border_colour);
	            draw_pixel(x - x_offset, y - y_offset, border_colour);
	        }
	    }
    }
}

void draw_circle(point_t location, uint16_t radius, colour_t fill_colour, colour_t border_colour, bool fill)
{
	int16_t f = 1 - radius;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * radius;
    int16_t x = 0;
    int16_t y = radius;

    // this puts the origin of the circle at the top left corner of the bounding square
    location.x += radius;
    location.y += radius;

    if(fill)
		draw_line_v(location.x, location.y - radius, 2 * radius + 1, fill_colour);

	if(border_colour != fill_colour)
	{
		draw_pixel(location.x, location.y + radius, border_colour);
	    draw_pixel(location.x, location.y - radius, border_colour);
	    draw_pixel(location.x + radius, location.y, border_colour);
	    draw_pixel(location.x - radius, location.y, border_colour);
	}

    while (x<y)
    {
        if(f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }

        x++;
        ddF_x += 2;
        f += ddF_x;

        if(fill)
		{
            draw_line_v(location.x+x, location.y-y, 2*y+1, fill_colour);
            draw_line_v(location.x+y, location.y-x, 2*x+1, fill_colour);

            draw_line_v(location.x-x, location.y-y, 2*y+1, fill_colour);
            draw_line_v(location.x-y, location.y-x, 2*x+1, fill_colour);
        }

        if(border_colour != fill_colour)
        {
			draw_pixel(location.x + x, location.y + y, border_colour);
	        draw_pixel(location.x - x, location.y + y, border_colour);
	        draw_pixel(location.x + x, location.y - y, border_colour);
	        draw_pixel(location.x - x, location.y - y, border_colour);

	        draw_pixel(location.x + y, location.y + x, border_colour);
	        draw_pixel(location.x - y, location.y + x, border_colour);
	        draw_pixel(location.x + y, location.y - x, border_colour);
	        draw_pixel(location.x - y, location.y - x, border_colour);
        }
    }
}
