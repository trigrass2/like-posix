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
 * This file is part of the cutensils project, <https://github.com/drmetal/cutensils>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

/**
* @addtogroup confparse
* @{
*
* Examples:

@code

//
// example of iterating over all config entries
//

// get some memory for a working area
uint8_t* buf[64];
config_parser_t cfg;
if(open_config_file(&cfg, buf, sizeof(buf), (const uint8_t*)"file.conf"))
{
    while(get_next_config(&cfg))
    {
        // get all entries as they appear
        printf("key: %s\n", get_config_key(&cfg));
        printf("value: %s\n", get_config_value(&cfg));

        // or test for a specific key
        if(config_key_match(&cfg, (const uint8_t*)"keyofinterest"))
        {
            printf("keyofinterest = %s", get_config_value(&cfg));
        }

        // or test for a specific value
        if(config_value_match(&cfg, (const uint8_t*)"valueofinterest"))
        {
            printf("%s = valueofinterest", get_config_key(&cfg));
        }
    }

    close_config_file(&cfg);
}

@endcode

*/

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>

#ifndef CONFPARSE_H_
#define CONFPARSE_H_

#ifdef __cplusplus
 extern "C" {
#endif

typedef struct {
	FILE* file;
    uint8_t* buffer;
    uint16_t buffer_length;
    const uint8_t* key;
    const uint8_t* value;
    const uint8_t* comment;
    bool retain_comments_newlines;
} config_parser_t;

bool config_file_exists(const uint8_t* filepath);
bool open_config_file(config_parser_t* cfg, uint8_t* buffer, uint16_t buffer_length, const uint8_t* filepath);
void close_config_file(config_parser_t* cfg);
bool get_next_config(config_parser_t* cfg);
bool config_key_match(config_parser_t* cfg, const uint8_t* key);
bool config_value_match(config_parser_t* cfg, const uint8_t* value);
const uint8_t* get_config_key(config_parser_t* cfg);
const uint8_t* get_config_value(config_parser_t* cfg);
const uint8_t* get_config_comment(config_parser_t* cfg);

const uint8_t* get_config_value_by_key(uint8_t* buffer, uint16_t buffer_length, const uint8_t* filepath, const uint8_t* key);
bool edit_config_entry(uint8_t* buffer, uint16_t buffer_length, const uint8_t* filepath, const uint8_t* key, const uint8_t* value);
bool add_config_entry(uint8_t* buffer, uint16_t buffer_length, const uint8_t* filepath, const uint8_t* key, const uint8_t* value);

bool config_key_in_list(const uint8_t* key, uint16_t key_length, const uint8_t** list);
bool string_match(const char* ref, const uint8_t* to_compare);

#ifdef __cplusplus
 }
#endif

#endif // CONFPARSE_H_

 /**
  * @}
  */
