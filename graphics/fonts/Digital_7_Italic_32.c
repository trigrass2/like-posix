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
 * This file is part of the Appleseed project, <https://github.com/drmetal/app-l-seed>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

#include "Digital_7_Italic_32.h"

static const uint8_t Digital_7_Italic_32_char_53_data[202] = {
    2, 0, 1, 75, 1, 235, 8, 255, 1, 219, 1, 3, 3, 0, 1, 115, 1, 203, 8, 255, 1, 95, 4, 0, 1, 243, 1, 179, 1, 63, 7, 31, 1, 3, 3, 0, 1, 11, 2, 255, 1, 75, 11, 0, 1, 35, 2, 255, 1, 51, 11, 0, 1, 63, 2, 255, 1, 31, 11, 0, 1, 91, 2, 255, 12, 0, 1, 115, 1, 255, 1, 231, 12, 0, 1, 139, 1, 247, 1, 83, 12, 0, 1, 79, 1, 91, 1, 207, 7, 255, 1, 143, 5, 0, 1, 59, 1, 243, 7, 255, 1, 147, 1, 95, 5, 0, 1, 19, 7, 31, 1, 163, 1, 255, 1, 23, 11, 0, 1, 95, 2, 255, 12, 0, 1, 127, 1, 255, 1, 223, 12, 0, 1, 147, 1, 255, 1, 203, 12, 0, 1, 163, 1, 255, 1, 179, 12, 0, 1, 191, 1, 255, 1, 159, 12, 0, 1, 211, 1, 255, 1, 127, 2, 0, 1, 51, 1, 151, 1, 227, 6, 255, 1, 195, 1, 191, 1, 255, 1, 103, 2, 0, 1, 71, 1, 251, 8, 255, 1, 143, 1, 243, 1, 35, 12, 0, 1, 15, 1, 11, 3, 0, 
};
static const uint8_t Digital_7_Italic_32_char_52_data[242] = {
    11, 0, 1, 127, 1, 167, 3, 0, 1, 151, 1, 99, 1, 7, 6, 0, 1, 15, 1, 239, 1, 255, 1, 27, 2, 0, 1, 227, 1, 255, 1, 115, 6, 0, 1, 91, 2, 255, 1, 3, 2, 0, 2, 255, 1, 91, 6, 0, 1, 127, 1, 255, 1, 235, 2, 0, 1, 27, 2, 255, 1, 63, 6, 0, 1, 143, 1, 255, 1, 215, 2, 0, 1, 55, 2, 255, 1, 35, 6, 0, 1, 159, 1, 255, 1, 191, 2, 0, 1, 83, 2, 255, 1, 11, 6, 0, 1, 191, 1, 255, 1, 159, 2, 0, 1, 107, 1, 255, 1, 243, 7, 0, 1, 207, 1, 255, 1, 139, 2, 0, 1, 135, 1, 255, 1, 139, 7, 0, 1, 151, 1, 255, 1, 115, 2, 0, 1, 111, 1, 147, 1, 207, 7, 255, 1, 147, 1, 175, 1, 47, 3, 0, 1, 59, 1, 243, 7, 255, 2, 159, 1, 7, 4, 0, 1, 19, 6, 31, 1, 51, 1, 215, 1, 255, 1, 15, 11, 0, 1, 107, 1, 255, 1, 247, 12, 0, 1, 127, 1, 255, 1, 223, 12, 0, 1, 155, 1, 255, 1, 195, 12, 0, 1, 171, 1, 255, 1, 171, 12, 0, 1, 191, 1, 255, 1, 151, 12, 0, 1, 219, 1, 255, 1, 127, 12, 0, 1, 171, 1, 255, 1, 91, 12, 0, 1, 95, 1, 211, 1, 15, 3, 0, 
};
static const uint8_t Digital_7_Italic_32_char_42_data[152] = {
    4, 0, 1, 35, 1, 191, 1, 27, 8, 0, 1, 195, 1, 255, 1, 147, 4, 0, 1, 23, 1, 111, 1, 107, 1, 3, 1, 223, 1, 255, 1, 127, 1, 35, 1, 131, 1, 87, 1, 0, 1, 71, 2, 255, 1, 211, 2, 255, 1, 211, 1, 251, 1, 255, 1, 187, 1, 0, 1, 15, 1, 215, 6, 255, 1, 231, 1, 63, 2, 0, 1, 11, 1, 199, 4, 255, 1, 203, 1, 15, 2, 0, 1, 47, 1, 207, 6, 255, 1, 199, 1, 27, 1, 0, 1, 175, 2, 255, 1, 231, 2, 255, 1, 231, 2, 255, 1, 95, 1, 0, 1, 123, 1, 167, 1, 63, 1, 123, 1, 255, 1, 227, 1, 15, 1, 139, 1, 151, 1, 51, 4, 0, 1, 147, 1, 255, 1, 203, 8, 0, 1, 43, 1, 219, 1, 59, 9, 0, 1, 3, 6, 0, 
};
static const uint8_t Digital_7_Italic_32_char_43_data[102] = {
    4, 0, 1, 15, 1, 191, 1, 27, 8, 0, 1, 147, 1, 255, 1, 179, 8, 0, 1, 191, 1, 255, 1, 159, 8, 0, 1, 211, 1, 255, 1, 139, 4, 0, 1, 47, 1, 223, 7, 255, 1, 111, 1, 0, 1, 83, 1, 251, 6, 255, 1, 251, 1, 123, 2, 0, 1, 23, 1, 31, 1, 67, 2, 255, 1, 79, 1, 31, 1, 23, 5, 0, 1, 63, 2, 255, 1, 31, 7, 0, 1, 87, 1, 255, 1, 251, 1, 3, 7, 0, 1, 7, 1, 199, 1, 83, 9, 0, 1, 3, 6, 0, 
};
static const uint8_t Digital_7_Italic_32_char_44_data[40] = {
    1, 143, 1, 255, 1, 207, 1, 0, 1, 175, 1, 255, 1, 175, 1, 0, 1, 207, 1, 255, 1, 143, 1, 0, 1, 239, 1, 255, 1, 111, 1, 0, 1, 99, 1, 175, 1, 11, 1, 0, 
};
static const uint8_t Digital_7_Italic_32_char_45_data[30] = {
    1, 47, 1, 223, 7, 255, 1, 111, 1, 0, 1, 83, 1, 251, 6, 255, 1, 251, 1, 123, 2, 0, 1, 23, 6, 31, 1, 23, 2, 0, 
};
static const uint8_t Digital_7_Italic_32_char_46_data[16] = {
    1, 143, 1, 255, 1, 207, 1, 0, 1, 175, 1, 255, 1, 175, 1, 0, 
};
static const uint8_t Digital_7_Italic_32_char_47_data[184] = {
    11, 0, 1, 95, 1, 247, 1, 155, 12, 0, 1, 15, 1, 231, 1, 255, 1, 227, 1, 7, 11, 0, 1, 135, 2, 255, 1, 91, 11, 0, 1, 35, 1, 251, 1, 255, 1, 195, 12, 0, 1, 183, 1, 255, 1, 251, 1, 51, 11, 0, 1, 71, 2, 255, 1, 151, 11, 0, 1, 3, 1, 219, 1, 255, 1, 239, 1, 19, 11, 0, 1, 51, 2, 255, 1, 107, 12, 0, 1, 83, 1, 255, 1, 211, 1, 3, 12, 0, 1, 115, 1, 255, 1, 67, 11, 0, 1, 135, 1, 211, 13, 0, 1, 43, 1, 251, 1, 211, 13, 0, 1, 195, 1, 255, 1, 179, 12, 0, 1, 91, 2, 255, 1, 107, 11, 0, 1, 15, 1, 231, 1, 255, 1, 211, 1, 3, 11, 0, 1, 143, 2, 255, 1, 67, 11, 0, 1, 43, 1, 251, 1, 255, 1, 167, 12, 0, 1, 195, 1, 255, 1, 243, 1, 27, 11, 0, 1, 91, 2, 255, 1, 123, 12, 0, 1, 75, 1, 239, 1, 219, 1, 7, 12, 0, 
};
static const uint8_t Digital_7_Italic_32_char_48_data[318] = {
    12, 0, 1, 19, 1, 15, 4, 0, 1, 75, 1, 235, 7, 255, 1, 239, 1, 175, 1, 223, 1, 3, 3, 0, 1, 115, 1, 203, 7, 255, 1, 163, 1, 251, 1, 255, 1, 27, 3, 0, 1, 243, 1, 179, 1, 63, 6, 31, 1, 111, 2, 255, 3, 0, 1, 11, 2, 255, 1, 75, 6, 0, 1, 127, 1, 255, 1, 227, 3, 0, 1, 35, 2, 255, 1, 51, 6, 0, 1, 151, 1, 255, 1, 207, 3, 0, 1, 63, 2, 255, 1, 31, 6, 0, 1, 167, 1, 255, 1, 183, 3, 0, 1, 91, 2, 255, 7, 0, 1, 191, 1, 255, 1, 159, 3, 0, 1, 115, 1, 255, 1, 231, 7, 0, 1, 215, 1, 255, 1, 131, 3, 0, 1, 139, 1, 247, 1, 83, 7, 0, 1, 99, 1, 255, 1, 107, 3, 0, 1, 79, 1, 59, 9, 0, 1, 111, 1, 23, 3, 0, 1, 71, 1, 39, 9, 0, 1, 91, 4, 0, 1, 207, 1, 223, 1, 23, 6, 0, 1, 3, 1, 163, 1, 255, 1, 23, 3, 0, 1, 231, 1, 255, 1, 119, 6, 0, 1, 95, 2, 255, 4, 0, 2, 255, 1, 95, 6, 0, 1, 127, 1, 255, 1, 223, 3, 0, 1, 31, 2, 255, 1, 67, 6, 0, 1, 147, 1, 255, 1, 203, 3, 0, 1, 55, 2, 255, 1, 43, 6, 0, 1, 163, 1, 255, 1, 179, 3, 0, 1, 79, 2, 255, 1, 23, 6, 0, 1, 191, 1, 255, 1, 159, 3, 0, 1, 103, 1, 247, 1, 167, 7, 0, 1, 211, 1, 255, 1, 127, 3, 0, 1, 91, 1, 167, 1, 227, 6, 255, 1, 195, 1, 191, 1, 255, 1, 103, 3, 0, 1, 71, 1, 251, 8, 255, 1, 143, 1, 243, 1, 35, 13, 0, 1, 15, 1, 11, 4, 0, 
};
static const uint8_t Digital_7_Italic_32_char_49_data[152] = {
    2, 0, 1, 23, 1, 223, 1, 43, 3, 0, 1, 131, 1, 255, 1, 127, 3, 0, 1, 223, 1, 255, 1, 99, 3, 0, 2, 255, 1, 75, 2, 0, 1, 27, 2, 255, 1, 55, 2, 0, 1, 55, 2, 255, 1, 31, 2, 0, 1, 83, 2, 255, 3, 0, 1, 107, 1, 255, 1, 235, 3, 0, 1, 59, 1, 251, 1, 211, 4, 0, 1, 95, 1, 131, 4, 0, 1, 83, 1, 91, 3, 0, 1, 111, 1, 255, 1, 139, 3, 0, 1, 239, 1, 255, 1, 111, 2, 0, 1, 7, 2, 255, 1, 83, 2, 0, 1, 35, 2, 255, 1, 55, 2, 0, 1, 63, 2, 255, 1, 27, 2, 0, 1, 95, 2, 255, 3, 0, 1, 119, 1, 255, 1, 223, 3, 0, 1, 75, 1, 255, 1, 195, 3, 0, 1, 7, 1, 239, 1, 79, 3, 0, 
};
static const uint8_t Digital_7_Italic_32_char_54_data[254] = {
    2, 0, 1, 75, 1, 235, 8, 255, 1, 219, 1, 3, 3, 0, 1, 115, 1, 203, 8, 255, 1, 95, 4, 0, 1, 243, 1, 179, 1, 63, 7, 31, 1, 3, 3, 0, 1, 11, 2, 255, 1, 75, 11, 0, 1, 35, 2, 255, 1, 51, 11, 0, 1, 63, 2, 255, 1, 31, 11, 0, 1, 91, 2, 255, 12, 0, 1, 115, 1, 255, 1, 231, 12, 0, 1, 139, 1, 247, 1, 83, 12, 0, 1, 79, 1, 91, 1, 207, 7, 255, 1, 143, 4, 0, 1, 71, 1, 99, 1, 243, 7, 255, 1, 147, 1, 95, 3, 0, 1, 207, 1, 223, 1, 43, 7, 31, 1, 163, 1, 255, 1, 23, 2, 0, 1, 231, 1, 255, 1, 119, 6, 0, 1, 95, 2, 255, 3, 0, 2, 255, 1, 95, 6, 0, 1, 127, 1, 255, 1, 223, 2, 0, 1, 31, 2, 255, 1, 67, 6, 0, 1, 147, 1, 255, 1, 203, 2, 0, 1, 55, 2, 255, 1, 43, 6, 0, 1, 163, 1, 255, 1, 179, 2, 0, 1, 79, 2, 255, 1, 23, 6, 0, 1, 191, 1, 255, 1, 159, 2, 0, 1, 103, 1, 247, 1, 167, 7, 0, 1, 211, 1, 255, 1, 127, 2, 0, 1, 91, 1, 167, 1, 227, 6, 255, 1, 195, 1, 191, 1, 255, 1, 103, 2, 0, 1, 71, 1, 251, 8, 255, 1, 143, 1, 243, 1, 35, 12, 0, 1, 15, 1, 11, 3, 0, 
};
static const uint8_t Digital_7_Italic_32_char_51_data[220] = {
    12, 0, 1, 19, 1, 15, 4, 0, 1, 75, 1, 235, 7, 255, 1, 239, 1, 175, 1, 223, 1, 3, 3, 0, 1, 91, 1, 203, 7, 255, 1, 163, 1, 251, 1, 255, 1, 27, 5, 0, 1, 23, 6, 31, 1, 111, 2, 255, 13, 0, 1, 127, 1, 255, 1, 227, 13, 0, 1, 151, 1, 255, 1, 207, 13, 0, 1, 167, 1, 255, 1, 183, 13, 0, 1, 191, 1, 255, 1, 159, 13, 0, 1, 215, 1, 255, 1, 131, 13, 0, 1, 99, 1, 255, 1, 107, 4, 0, 1, 31, 1, 207, 7, 255, 1, 143, 1, 111, 1, 23, 4, 0, 1, 59, 1, 243, 7, 255, 1, 147, 1, 95, 6, 0, 1, 19, 7, 31, 1, 163, 1, 255, 1, 23, 12, 0, 1, 95, 2, 255, 13, 0, 1, 127, 1, 255, 1, 223, 13, 0, 1, 147, 1, 255, 1, 203, 13, 0, 1, 163, 1, 255, 1, 179, 13, 0, 1, 191, 1, 255, 1, 159, 13, 0, 1, 211, 1, 255, 1, 127, 3, 0, 1, 51, 1, 151, 1, 227, 6, 255, 1, 195, 1, 191, 1, 255, 1, 103, 3, 0, 1, 71, 1, 251, 8, 255, 1, 143, 1, 243, 1, 35, 13, 0, 1, 15, 1, 11, 4, 0, 
};
static const uint8_t Digital_7_Italic_32_char_126_data[112] = {
    3, 0, 1, 83, 1, 251, 1, 195, 1, 11, 10, 0, 1, 175, 1, 255, 1, 239, 1, 51, 7, 0, 1, 19, 1, 187, 1, 255, 1, 123, 1, 95, 1, 127, 1, 243, 1, 215, 1, 23, 1, 19, 1, 187, 1, 255, 1, 111, 1, 0, 1, 39, 1, 231, 1, 251, 1, 123, 1, 0, 1, 143, 1, 255, 1, 211, 1, 79, 1, 83, 1, 231, 1, 251, 1, 123, 2, 0, 1, 15, 1, 23, 3, 0, 1, 31, 1, 111, 2, 251, 1, 79, 1, 23, 9, 0, 1, 95, 1, 255, 1, 219, 1, 39, 11, 0, 1, 27, 1, 11, 4, 0, 
};
static const uint8_t Digital_7_Italic_32_char_50_data[208] = {
    12, 0, 1, 19, 1, 15, 4, 0, 1, 75, 1, 235, 7, 255, 1, 239, 1, 175, 1, 223, 1, 3, 3, 0, 1, 91, 1, 203, 7, 255, 1, 163, 1, 251, 1, 255, 1, 27, 5, 0, 1, 23, 6, 31, 1, 111, 2, 255, 13, 0, 1, 127, 1, 255, 1, 227, 13, 0, 1, 151, 1, 255, 1, 207, 13, 0, 1, 167, 1, 255, 1, 183, 13, 0, 1, 191, 1, 255, 1, 159, 13, 0, 1, 215, 1, 255, 1, 131, 13, 0, 1, 99, 1, 255, 1, 107, 4, 0, 1, 31, 1, 207, 7, 255, 1, 143, 1, 111, 1, 23, 3, 0, 1, 71, 1, 99, 1, 243, 7, 255, 1, 147, 1, 3, 4, 0, 1, 207, 1, 223, 1, 43, 6, 31, 1, 27, 6, 0, 1, 231, 1, 255, 1, 119, 13, 0, 2, 255, 1, 95, 12, 0, 1, 31, 2, 255, 1, 67, 12, 0, 1, 55, 2, 255, 1, 43, 12, 0, 1, 79, 2, 255, 1, 23, 12, 0, 1, 103, 1, 247, 1, 167, 13, 0, 1, 91, 1, 167, 1, 227, 7, 255, 1, 163, 5, 0, 1, 71, 1, 251, 8, 255, 1, 243, 1, 7, 4, 0, 
};
static const uint8_t Digital_7_Italic_32_char_55_data[242] = {
    11, 0, 1, 19, 1, 15, 3, 0, 1, 75, 1, 235, 7, 255, 1, 239, 1, 175, 1, 223, 1, 3, 2, 0, 1, 115, 1, 203, 7, 255, 1, 163, 1, 251, 1, 255, 1, 27, 2, 0, 1, 243, 1, 179, 1, 63, 6, 31, 1, 111, 2, 255, 2, 0, 1, 11, 2, 255, 1, 75, 6, 0, 1, 127, 1, 255, 1, 227, 2, 0, 1, 35, 2, 255, 1, 51, 6, 0, 1, 151, 1, 255, 1, 207, 2, 0, 1, 63, 2, 255, 1, 31, 6, 0, 1, 167, 1, 255, 1, 183, 2, 0, 1, 91, 2, 255, 7, 0, 1, 191, 1, 255, 1, 159, 2, 0, 1, 115, 1, 255, 1, 231, 7, 0, 1, 215, 1, 255, 1, 131, 2, 0, 1, 139, 1, 247, 1, 83, 7, 0, 1, 99, 1, 255, 1, 107, 2, 0, 1, 79, 1, 59, 9, 0, 1, 111, 1, 23, 12, 0, 1, 11, 1, 155, 1, 7, 11, 0, 1, 23, 1, 215, 1, 255, 1, 15, 11, 0, 1, 107, 1, 255, 1, 247, 12, 0, 1, 127, 1, 255, 1, 223, 12, 0, 1, 155, 1, 255, 1, 195, 12, 0, 1, 171, 1, 255, 1, 171, 12, 0, 1, 191, 1, 255, 1, 151, 12, 0, 1, 219, 1, 255, 1, 127, 12, 0, 1, 171, 1, 255, 1, 91, 12, 0, 1, 95, 1, 211, 1, 15, 3, 0, 
};
static const uint8_t Digital_7_Italic_32_char_58_data[48] = {
    1, 0, 1, 47, 2, 255, 1, 47, 2, 0, 1, 79, 2, 255, 1, 15, 2, 0, 1, 11, 2, 31, 38, 0, 1, 31, 2, 255, 1, 63, 2, 0, 1, 63, 2, 255, 1, 31, 2, 0, 1, 7, 2, 31, 1, 3, 2, 0, 
};
static const uint8_t Digital_7_Italic_32_char_57_data[272] = {
    12, 0, 1, 19, 1, 15, 4, 0, 1, 75, 1, 235, 7, 255, 1, 239, 1, 175, 1, 223, 1, 3, 3, 0, 1, 115, 1, 203, 7, 255, 1, 163, 1, 251, 1, 255, 1, 27, 3, 0, 1, 243, 1, 179, 1, 63, 6, 31, 1, 111, 2, 255, 3, 0, 1, 11, 2, 255, 1, 75, 6, 0, 1, 127, 1, 255, 1, 227, 3, 0, 1, 35, 2, 255, 1, 51, 6, 0, 1, 151, 1, 255, 1, 207, 3, 0, 1, 63, 2, 255, 1, 31, 6, 0, 1, 167, 1, 255, 1, 183, 3, 0, 1, 91, 2, 255, 7, 0, 1, 191, 1, 255, 1, 159, 3, 0, 1, 115, 1, 255, 1, 231, 7, 0, 1, 215, 1, 255, 1, 131, 3, 0, 1, 139, 1, 247, 1, 83, 7, 0, 1, 99, 1, 255, 1, 107, 3, 0, 1, 79, 1, 91, 1, 207, 7, 255, 1, 143, 1, 111, 1, 23, 4, 0, 1, 59, 1, 243, 7, 255, 1, 147, 1, 95, 6, 0, 1, 19, 7, 31, 1, 163, 1, 255, 1, 23, 12, 0, 1, 95, 2, 255, 13, 0, 1, 127, 1, 255, 1, 223, 13, 0, 1, 147, 1, 255, 1, 203, 13, 0, 1, 163, 1, 255, 1, 179, 13, 0, 1, 191, 1, 255, 1, 159, 13, 0, 1, 211, 1, 255, 1, 127, 3, 0, 1, 51, 1, 151, 1, 227, 6, 255, 1, 195, 1, 191, 1, 255, 1, 103, 3, 0, 1, 71, 1, 251, 8, 255, 1, 143, 1, 243, 1, 35, 13, 0, 1, 15, 1, 11, 4, 0, 
};
static const uint8_t Digital_7_Italic_32_char_56_data[324] = {
    12, 0, 1, 19, 1, 15, 4, 0, 1, 75, 1, 235, 7, 255, 1, 239, 1, 175, 1, 223, 1, 3, 3, 0, 1, 115, 1, 203, 7, 255, 1, 163, 1, 251, 1, 255, 1, 27, 3, 0, 1, 243, 1, 179, 1, 63, 6, 31, 1, 111, 2, 255, 3, 0, 1, 11, 2, 255, 1, 75, 6, 0, 1, 127, 1, 255, 1, 227, 3, 0, 1, 35, 2, 255, 1, 51, 6, 0, 1, 151, 1, 255, 1, 207, 3, 0, 1, 63, 2, 255, 1, 31, 6, 0, 1, 167, 1, 255, 1, 183, 3, 0, 1, 91, 2, 255, 7, 0, 1, 191, 1, 255, 1, 159, 3, 0, 1, 115, 1, 255, 1, 231, 7, 0, 1, 215, 1, 255, 1, 131, 3, 0, 1, 139, 1, 247, 1, 83, 7, 0, 1, 99, 1, 255, 1, 107, 3, 0, 1, 79, 1, 91, 1, 207, 7, 255, 1, 143, 1, 111, 1, 23, 3, 0, 1, 71, 1, 99, 1, 243, 7, 255, 1, 147, 1, 95, 4, 0, 1, 207, 1, 223, 1, 43, 7, 31, 1, 163, 1, 255, 1, 23, 3, 0, 1, 231, 1, 255, 1, 119, 6, 0, 1, 95, 2, 255, 4, 0, 2, 255, 1, 95, 6, 0, 1, 127, 1, 255, 1, 223, 3, 0, 1, 31, 2, 255, 1, 67, 6, 0, 1, 147, 1, 255, 1, 203, 3, 0, 1, 55, 2, 255, 1, 43, 6, 0, 1, 163, 1, 255, 1, 179, 3, 0, 1, 79, 2, 255, 1, 23, 6, 0, 1, 191, 1, 255, 1, 159, 3, 0, 1, 103, 1, 247, 1, 167, 7, 0, 1, 211, 1, 255, 1, 127, 3, 0, 1, 91, 1, 167, 1, 227, 6, 255, 1, 195, 1, 191, 1, 255, 1, 103, 3, 0, 1, 71, 1, 251, 8, 255, 1, 143, 1, 243, 1, 35, 13, 0, 1, 15, 1, 11, 4, 0, 
};


static const character_t Digital_7_Italic_32_char_42 = {
    .yoffset=10, .width=11, .xadvance=12, .height=12, .data=Digital_7_Italic_32_char_42_data, .xoffset=1, 
};
static const character_t Digital_7_Italic_32_char_43 = {
    .yoffset=11, .width=11, .xadvance=12, .height=11, .data=Digital_7_Italic_32_char_43_data, .xoffset=1, 
};
static const character_t Digital_7_Italic_32_char_44 = {
    .yoffset=24, .width=4, .xadvance=5, .height=5, .data=Digital_7_Italic_32_char_44_data, .xoffset=0, 
};
static const character_t Digital_7_Italic_32_char_45 = {
    .yoffset=15, .width=11, .xadvance=12, .height=3, .data=Digital_7_Italic_32_char_45_data, .xoffset=1, 
};
static const character_t Digital_7_Italic_32_char_46 = {
    .yoffset=24, .width=4, .xadvance=5, .height=2, .data=Digital_7_Italic_32_char_46_data, .xoffset=0, 
};
static const character_t Digital_7_Italic_32_char_47 = {
    .yoffset=6, .width=16, .xadvance=15, .height=20, .data=Digital_7_Italic_32_char_47_data, .xoffset=0, 
};
static const character_t Digital_7_Italic_32_char_48 = {
    .yoffset=5, .width=16, .xadvance=15, .height=22, .data=Digital_7_Italic_32_char_48_data, .xoffset=0, 
};
static const character_t Digital_7_Italic_32_char_49 = {
    .yoffset=6, .width=6, .xadvance=5, .height=20, .data=Digital_7_Italic_32_char_49_data, .xoffset=0, 
};
static const character_t Digital_7_Italic_32_char_50 = {
    .yoffset=5, .width=16, .xadvance=15, .height=21, .data=Digital_7_Italic_32_char_50_data, .xoffset=0, 
};
static const character_t Digital_7_Italic_32_char_51 = {
    .yoffset=5, .width=16, .xadvance=15, .height=22, .data=Digital_7_Italic_32_char_51_data, .xoffset=0, 
};
static const character_t Digital_7_Italic_32_char_52 = {
    .yoffset=6, .width=15, .xadvance=15, .height=20, .data=Digital_7_Italic_32_char_52_data, .xoffset=1, 
};
static const character_t Digital_7_Italic_32_char_53 = {
    .yoffset=6, .width=15, .xadvance=15, .height=21, .data=Digital_7_Italic_32_char_53_data, .xoffset=0, 
};
static const character_t Digital_7_Italic_32_char_54 = {
    .yoffset=6, .width=15, .xadvance=15, .height=21, .data=Digital_7_Italic_32_char_54_data, .xoffset=0, 
};
static const character_t Digital_7_Italic_32_char_55 = {
    .yoffset=5, .width=15, .xadvance=15, .height=21, .data=Digital_7_Italic_32_char_55_data, .xoffset=1, 
};
static const character_t Digital_7_Italic_32_char_56 = {
    .yoffset=5, .width=16, .xadvance=15, .height=22, .data=Digital_7_Italic_32_char_56_data, .xoffset=0, 
};
static const character_t Digital_7_Italic_32_char_57 = {
    .yoffset=5, .width=16, .xadvance=15, .height=22, .data=Digital_7_Italic_32_char_57_data, .xoffset=0, 
};
static const character_t Digital_7_Italic_32_char_58 = {
    .yoffset=10, .width=6, .xadvance=5, .height=12, .data=Digital_7_Italic_32_char_58_data, .xoffset=0, 
};
static const character_t Digital_7_Italic_32_char_126 = {
    .yoffset=11, .width=14, .xadvance=15, .height=7, .data=Digital_7_Italic_32_char_126_data, .xoffset=1, 
};


static const character_t* Digital_7_Italic_32_characters[] = {
&Digital_7_Italic_32_char_42, &Digital_7_Italic_32_char_43, &Digital_7_Italic_32_char_44, &Digital_7_Italic_32_char_45, &Digital_7_Italic_32_char_46, &Digital_7_Italic_32_char_47, &Digital_7_Italic_32_char_48, &Digital_7_Italic_32_char_49, &Digital_7_Italic_32_char_50, &Digital_7_Italic_32_char_51, &Digital_7_Italic_32_char_52, &Digital_7_Italic_32_char_53, &Digital_7_Italic_32_char_54, &Digital_7_Italic_32_char_55, &Digital_7_Italic_32_char_56, &Digital_7_Italic_32_char_57, &Digital_7_Italic_32_char_58, &Digital_7_Italic_32_char_126, 
};

const font_t Digital_7_Italic_32 = {
    .aa=1, .count=18, .bold=0, .smooth=1, .stretchH=100, .italic=0, .characters=Digital_7_Italic_32_characters, .base_id=42, .size=32, .outline=0, 
};
