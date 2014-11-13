
#include <stdint.h>

#ifndef FONT_TYPE_H_
#define FONT_TYPE_H_

typedef struct {
    uint8_t yoffset;
    uint8_t width;
    uint8_t xadvance;
    uint8_t height;
    const uint8_t* data;
    uint8_t xoffset;
    
} character_t;
typedef struct {
    uint8_t aa;
    uint8_t count;
    uint8_t bold;
    uint8_t italic;
    const character_t** characters;
    uint8_t base_id;
    uint8_t outline;
    uint8_t smooth;
    uint8_t stretchH;
    uint8_t size;
    
} font_t;

#endif // FONT_TYPE_H_