/*
 * jsmm_extensions.c
 *
 *  Created on: 19/02/2015
 *      Author: Mike Stuart
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "jsmn_extensions.h"

/**
 * initialise a json/jsmn object from an input string.
 *
 * @param   json will be fully initialized by this function.
 * @param   tokens, pointer to an array of tokens. if there are more tokens in the input string
 *          than are provided in 'tokens', a parse error will result.
 * @param   ntokens, indicates how many tokens in the array 'tokens'.
 * @param   input is a pointer to the string to parse, this must be a valid json formatted string
 *          or a parse error will result. it does not have to be 0 terminated.
 * @param   length is the length in bytes of the input string.
 * @retval  returns either a jsmn error code or the the number of tokens found in the input string.
 *
 */
jsmnerr_t json_init(json_t* json, jsmntok_t* tokens, int ntokens, char* input, int length)
{
    jsmn_parser parser;
    json->buffer = input;
    json->tokens = tokens;
    json->current_item = NULL;
    json->current_iterable = NULL;
    jsmn_init(&parser);
    jsmnerr_t e = jsmn_parse(&parser, input, length, tokens, ntokens);
    if(e > 0)
    {
        json->current_item = tokens;
    }
    return e;
}

/**
 * resets the json iterator.
 *
 * this function must be called prior to using json_iterator().
 *
 * @param   json - pointer to the initialized json structure to work with.
 * @param   container - pointer to the particular json array or object to iterate on with within the token array.
 * @retval  returns 'container'.
 *
 *          the return value may also be accessed by the function json_get_current_item().
 */
jsmntok_t* json_reset_iterator(json_t* json, jsmntok_t* container)
{
    json->current_item = json->current_iterable = container;
    return json->current_item;
}

/**
 * the json iterator, iterates over keys in an object, or items in an array.
 *
 * it is required that json_reset_iterator() is called once before the iteration cycle.
 * it is safe to break the iteration cycle midway through if needed.
 *
 * @param   json - pointer to the initialized json structure to work with.
 * @retval  returns the first object key or array item, subsequent calls return subsequent
 *          keys/items until the end of the container is reached. at that point NULL is returned.
 *          if further calls are made, the iteration process repeats.
 *
 *          the return value may also be accessed by the function json_get_current_item().
 */
jsmntok_t* json_iterator(json_t* json)
{
    if(json->current_iterable && json->current_iterable->type != JSMN_PRIMITIVE && json->current_iterable->type != JSMN_STRING)
    {
        if(json->current_item < json->current_iterable || json->current_item > (json->current_iterable + json->current_iterable->size))
           json->current_item = json->current_iterable;

        if(json->current_item != json->current_iterable)
        {
            json->current_item += json->current_item->size;
            if(json->current_iterable->type == JSMN_OBJECT)
                json->current_item++;
        }

        // advance to next item
        json->current_item++;
        // return NULL if we hit the end of the parent container
        if(&json->tokens[json->current_item->parent] != json->current_iterable)
            json->current_item = NULL;
    }
    else
        json->current_item = NULL;

    return json->current_item;
}

/**
 * special helper method for use inside iterator loop, when iterating over an object type.
 *
 * @retval  returns the value token associated with the current key token,
 *          or NULL if the current iterator is not an object.
 */
jsmntok_t* json_iterator_get_object_value(json_t* json)
{
    if(json->current_iterable && json->current_item &&
       json->current_iterable->type == JSMN_OBJECT &&
       json->current_iterable == (json->tokens + json->current_item->parent))
    {
        return json->current_item + 1;
    }
    return NULL;
}

/**
 * special json iterator for the value set of json object types only.
 *
 * it is required that json_reset_iterator() is called once before the iteration cycle.
 * it is safe to break the iteration cycle midway through if needed.
 *
 * @param   json - pointer to the initialized json structure to work with.
 * @retval  returns the first object value, subsequent calls return subsequent
 *          values until the end of the object is reached. at that point NULL is returned.
 *          if further calls are made, the iteration process repeats.
 *
 *          the return value may also be accessed by the function json_get_current_item().
 */
jsmntok_t* json_object_value_iterator(json_t* json)
{
    if(json->current_iterable && json->current_iterable->type == JSMN_OBJECT && json->current_iterable->size >= 2)
    {
        if(json->current_item < json->current_iterable || json->current_item > (json->current_iterable + json->current_iterable->size))
            json->current_item = json->current_iterable;
        else if(json->current_item != json->current_iterable)
            json->current_item += json->current_item->size;

        json->current_item += 2;

        if(&json->tokens[json->current_item->parent] != json->current_iterable)
            json->current_item = NULL;
    }

    return json->current_item;
}


/**
 * json object value access by key.
 *
 * @param   json - pointer to the initialized json structure to work with.
 * @param   object - pointer to the particular json object to work with within the token array.
 * @param   key is a pointer to the key string to find the value for.
 * @retval  returns the json item (the value correspoding tho the given key).
 *          returns NULL if:
 *          - 'key' string was not found in the object.
 *          - 'object' was not actually an object type or NULL.
 *          - 'object' was empty.
 *
 *          the return value may also be accessed by the function json_get_current_item().
 */
jsmntok_t* json_get_value_by_key(json_t* json, jsmntok_t* object, char* key)
{
    jsmntok_t* token = object+1;
    int count = 0;
    json->current_item = NULL;

    if(object && object->type == JSMN_OBJECT && object->size >= 2)
    {
        while(count < object->size)
        {
           if(json->tokens + token->parent == object)
           {
               if(memcmp(key, json->buffer + token->start, token->end - token->start) == 0)
               {
                   json->current_item = token + 1;
                   return json->current_item;
               }
               count++;
           }
           token++;
        }
    }

    return json->current_item;
}

/**
 * json array item access by index.
 *
 * @param   json - pointer to the initialized json structure to work with.
 * @param   array - pointer to the particular json array to work with within the token array.
 * @param   index is the index number within the array to find the value for.
 * @retval  returns the json item (the value correspoding the the given index).
 *          returns NULL if:
 *          - 'index' value was out of bounds for the array (< 0, or >= length of the array).
 *          - 'array' was not actually an object type, or NULL.
 *          - 'array' was empty.
 *
 *          the return value may also be accessed by the function json_get_current_item().
 */
jsmntok_t* json_get_item_by_index(json_t* json, jsmntok_t* array, int index)
{
    jsmntok_t* token;
    json->current_item = NULL;
    if(array && array->type == JSMN_ARRAY && index >= 0 && index < array->size)
    {
        token = array + 1;
        while(index--)
        {
            token += token->size;
            token++;
        }
        json->current_item = token;
    }
    return json->current_item;
}

/**
 * returns the current selected json item. this is usually found by one of the
 * iterator or access functions first, eg:
 *
 * json_get_value_by_key()
 * json_get_item_by_index()
 * json_iterator()
 * json_object_value_iterator()
 *
 * @param   json - pointer to the initialized json structure to work with.
 * @retval  returns the current json item, or NULL if an iteration has ended or an error
 *          occurred in the previous json function call.
 */
jsmntok_t* json_get_current_item(json_t* json)
{
    return json->current_item;
}

/**
 * returns the current iterable json item.
 *
 * this is set first by a function call to json_reset_iterator()
 *
 * @param   json - pointer to the initialized json structure to work with.
 * @retval  returns the current iterable json item.
 */
jsmntok_t* json_get_current_iterable(json_t* json)
{
    return json->current_iterable;
}

/**
 * function for testing json primitive values.
 *
 * @param   json - pointer to the initialized json structure to work with.
 * @param   token - pointer to the token to get the value of.
 *          the token should have been set with one of the valid json primitive values:
 *          - true
 *          - false
 *          - null
 * @retval  returns true if the token value matches that specified in 'value'.
 */
bool json_token_value_match(json_t* json, jsmntok_t* token, const char* value)
{
    if(token && ((token->type == JSMN_PRIMITIVE) || (token->type == JSMN_STRING)))
        return memcmp(value, json->buffer + token->start, token->end - token->start) == 0;
    return false;
}

/**
 * @param   json - pointer to the initialized json structure to work with.
 * @param   token - pointer to the token to get the value of.
 *          the token should have been set with an integer type primitive.
 * @retval  returns value of 'token' as an integer.
 *
 *          warning - this function is a danger as it will always return 0 when an error occurs, such
 *                      as when the token is not an integer type.
 *          warning - this function modifies the input buffer, zero terminating the integer representation.
 */
int json_token_integer_value(json_t* json, jsmntok_t* token)
{
    if(token && token->type == JSMN_PRIMITIVE)
    {
        json->buffer[token->end] = '\0';
        return atoi(json->buffer + token->start);
    }
    return 0;
}

/**
 * @param   json - pointer to the initialized json structure to work with.
 * @param   token - pointer to the token to get the value of.
 *          the token should have been set with an float type primitive.
 * @retval  returns value of 'token' as a float.
 *
 *          warning - this function modifies the input buffer, zero terminating the float representation.
 */
float json_token_float_value(json_t* json, jsmntok_t* token)
{
    if(token && token->type == JSMN_PRIMITIVE)
    {
        json->buffer[token->end] = '\0';
        return atof(json->buffer + token->start);
    }
    return NAN;
}

/**
 * @param   json - pointer to the initialized json structure to work with.
 * @param   token - pointer to the token to get the value of.
 *          the token should have been set with an string type.
 * @retval  returns value of 'token' as a string.
 *
 *          warning - this function modifies the input buffer, zero terminating the string.
 */
char* json_token_string_value(json_t* json, jsmntok_t* token)
{
    if(token && token->type == JSMN_STRING)
    {
        json->buffer[token->end] = '\0';
        return json->buffer + token->start;
    }
    return NULL;
}

/**
 * wraps json_token_value_match(), acting on the current item.
 */
bool json_value_match(json_t* json, const char* value)
{
    return json_token_value_match(json, json->current_item, value);
}

/**
 * wraps json_token_float_value(), acting on the current item.
 */
float json_float_value(json_t* json)
{
    return json_token_float_value(json, json->current_item);
}

/**
 * wraps json_token_integer_value(), acting on the current item.
 */
int json_integer_value(json_t* json)
{
    return json_token_integer_value(json, json->current_item);
}

/**
 * wraps json_token_string_value(), acting on the current item.
 */
char* json_string_value(json_t* json)
{
    return json_token_string_value(json, json->current_item);
}

/**
 * @retval  the type of the current item.
 */
jsmntype_t json_type(json_t* json)
{
    return json->current_item->type;
}

/**
 * @retval  the size of the current item (0 for primitives and strings, 0 or larger for array and object)
 */
int json_size(json_t* json)
{
    return json->current_item->size;
}

/**
 * @retval  the length in bytes of the current item, in the buffer.
 */
int json_raw_length(json_t* json)
{
    return json->current_item->end - json->current_item->start;
}

/**
 * @retval  the address in the buffer of the current item.
 */
char* json_raw_data(json_t* json)
{
    return json->buffer + json->current_item->start;
}
