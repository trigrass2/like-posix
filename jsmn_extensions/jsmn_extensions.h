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

#ifndef JSMN_EXTENSIONS_H_
#define JSMN_EXTENSIONS_H_

#include <stdbool.h>
#include <stddef.h>

#include "jsmn.h"

typedef struct _json_t {
    char* buffer;
    jsmntok_t* tokens;
    jsmntok_t* current_item;
    jsmntok_t* current_iterable;
}json_t;

jsmnerr_t json_init(json_t* json, jsmntok_t* tokens, int ntokens, char* input, int length);

jsmntok_t* json_reset_iterator(json_t* json, jsmntok_t* container);
jsmntok_t* json_iterator(json_t* json);
jsmntok_t* json_iterator_get_object_value(json_t* json);
jsmntok_t* json_object_value_iterator(json_t* json);

jsmntok_t* json_get_value_by_key(json_t* json, jsmntok_t* object, char* key);
jsmntok_t* json_get_item_by_index(json_t* json, jsmntok_t* array, int index);
jsmntok_t* json_get_current_item(json_t* json);
jsmntok_t* json_get_current_iterable(json_t* json);

bool json_token_value_match(json_t* json, jsmntok_t* token, const char* value);

jsmntok_t* json_token_is_iterable(jsmntok_t* iterable);
jsmntok_t* json_token_in_iterable(jsmntok_t* token, jsmntok_t* iterable);
int json_token_integer_value(json_t* json, jsmntok_t* token);
double json_token_float_value(json_t* json, jsmntok_t* token);
char* json_token_string_value(json_t* json, jsmntok_t* token);

bool json_value_match(json_t* json, const char* value);
float json_float_value(json_t* json);
int json_integer_value(json_t* json);
char* json_string_value(json_t* json);
jsmntype_t json_type(json_t* json);
int json_size(json_t* json);
int json_raw_length(json_t* json);
char* json_raw_data(json_t* json);

#endif /* JSMN_EXTENSIONS_H_ */
