
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "lcd.h"
#include "system.h"
#include "Ubuntu_16.h"
#include "Ubuntu_20.h"
#include "Ubuntu_24.h"
#include "Ubuntu_32.h"
#include "Ubuntu_38.h"
#include "Ubuntu_48.h"
#include "Ubuntu_64.h"
#include "Ubuntu_48_bold.h"
#include "Ubuntu_64_bold.h"
#include "Digital_7_Italic_32.h"
#include "Digital_7_Italic_64.h"
#include "Digital_7_Italic_96.h"
#include "Digital_7_Italic_128.h"
#include "images.h"


#ifndef GRAPHICS_H_
#define GRAPHICS_H_

typedef enum {
	CIRCLE,
	SQUARE,
	LINE,
    DOT,
} shape_name_t;

typedef struct {
	shape_name_t type;			///< defines the type of shape
	colour_t fill_colour;		///< the colour of the shape
	colour_t border_colour;		///< the colour of the shape border
	bool fill;					///< if set, fills the shape, if false just draws an border.
	point_t size;				///< the size of the shape (bounding rectangle of non rectangles)
	uint16_t radius;			///< the radius of the shape (radius of corners where applicable)
}shape_t;


void graphics_init();
void set_background_colour(colour_t colour);
colour_t get_background_colour();
void draw_shape(shape_t* shape, point_t location);
void set_shape_size(shape_t* shape, point_t size);
void draw_image(const image_t* image, point_t location);
colour_t blend_colour(colour_t fgc, alpha_t fga, colour_t bgc, alpha_t bga);

#endif // GRAPHICS_H_
