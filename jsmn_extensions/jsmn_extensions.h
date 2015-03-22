/*
 * jsmn_extensions.h
 *
 *  Created on: 19/02/2015
 *      Author: Mike Stuart
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

int json_token_integer_value(json_t* json, jsmntok_t* token);
float json_token_float_value(json_t* json, jsmntok_t* token);
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
