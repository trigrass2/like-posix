
#include <stdint.h>

#ifndef IMAGES_H_
#define IMAGES_H_

typedef struct  {
    uint16_t width;
    uint16_t height;
    const uint16_t* data;
} image_t;

#define image_width(image)      (image.width)
#define image_height(image)     (image.height)

extern const image_t micro_sd_gray_aa;
extern const image_t micro_sd_blue_aa;

extern const image_t up_bright_aa;
extern const image_t up_dim_aa;
extern const image_t up_gray_aa;
extern const image_t down_bright_aa;
extern const image_t down_dim_aa;
extern const image_t down_gray_aa;

#endif // IMAGES_H_