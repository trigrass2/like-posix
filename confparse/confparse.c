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
* @defgroup confparse Config File Parser
*
* This module provides a method of reading key value pairs from a config file.
*
* @{
* @file confparse.c
*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "confparse.h"

/**
 * @brief 	opens a config file for parsing.
 *
 * @param cfg pointer to uninitialised config parser structure.
 * @param buffer is a pointer to some memory to use. must be long enough to hold one line from the config file.
 * @param buffer_length is the length of the buffer memory in  bytes.
 * @param filepath is the path to the config file.
 * @retval returns false if the buffer was null or 0 length, or if the file was not found. true otherwise.
 */
bool open_config_file(config_parser_t* cfg, uint8_t* buffer, uint16_t buffer_length, const uint8_t* filepath)
{
	cfg->file = NULL;
	if(buffer && buffer_length)
	{
		cfg->buffer = buffer;
		cfg->buffer_length = buffer_length;
		cfg->key = NULL;
		cfg->value = NULL;
		cfg->retain_comments_newlines = false;
		cfg->file = fopen((const char*)filepath, "r");
	}
    return cfg->file != NULL;
}

/**
 * @brief 	checks a config file exists.
 *
 * @param filepath is the path to the config file.
 * @retval returns false if the buffer was null or 0 length, or if the file was not found. true otherwise.
 */
bool config_file_exists(const uint8_t* filepath)
{
	struct stat st;
	int fd = stat((const char*)filepath, &st);
	return (fd != EOF) && (st.st_size > 0);
}

/**
 * @brief 	closes a config file.
 *
 * @param cfg pointer to an initialised config parser structure.
 */
void close_config_file(config_parser_t* cfg)
{
	if(cfg->file != NULL)
		fclose(cfg->file);
}

/**
 * @brief 	reads the current line from the config file.
 * 			this function is used to iterate over all config entries.
 * @param   cfg pointer to an initialised config parser structure.
 * @retval  returns true if a line was read successfully. does not indicate that the line held a key:value pair.
 *          returns false if the end of the file is reached.
 */
bool get_next_config(config_parser_t* cfg)
{
	long int fptr, size;
    uint8_t* start;
    uint8_t* ptr;
    uint8_t* end;

    cfg->value = NULL;
    cfg->key = NULL;

    if(!cfg->buffer || !cfg->buffer_length)
        return false;

    // where are we now?
    fptr = ftell(cfg->file);
    // go to end of file
    fseek(cfg->file, 0, SEEK_END);
    // where is that?
    size = ftell(cfg->file);
    // return to where we were a moment ago
    fseek(cfg->file, fptr, SEEK_SET);

	if(fptr >= size)
		return false;

    while(fgets((char*)cfg->buffer, cfg->buffer_length, cfg->file))
    {
    	cfg->comment = NULL;
		cfg->key = NULL;
		cfg->value = NULL;

    	// trim leading spaces
		start = cfg->buffer;
		while(*start &&  ((*start == ' ')||(*start == '\t')))
			start++;

		// fint the last non whitespace character
		end = strlen(cfg->buffer) + cfg->buffer;
		while((end > start) &&  ((*end == ' ')||(*end == '\t')||(*end == '\n'))||(*end == '\r'))
			end--;

		// if we didnt hit the start, there must be some text content
		if(end > start)
			end++;
		*end = '\0';


		cfg->comment = (uint8_t*)strchr((const char*)start, '#');
		if(cfg->comment)
		{
			*cfg->comment = '\0';
			cfg->comment++;
		}


		if((*ptr == '\0' ||*ptr == '#') && cfg->retain_comments_newlines)
		{
			// the line is blank or only contains a comment - just pass on the whole line
			cfg->comment = cfg->buffer;
			return true;
		}
		else if(*ptr != '\0' && *ptr != '#')
		{
			// we have a key, minus whitespace
			cfg->key = ptr;

			// null terminate line at the first trailing '#' character, if there is one
			ptr = (uint8_t*)strchr((const char*)cfg->key, '#');
			if(ptr)
			{
				cfg->comment = ptr + 1;
				*ptr = '\0';
			}

			// trim leading spaces.
			ptr = cfg->buffer;
			while(*ptr && *ptr == ' ')
			{
				ptr++;
			}


			// find the space - there must be one
			ptr = (uint8_t*)strchr((const char*)cfg->buffer, ' ');
			if(ptr)
			{
				// null terminate at the space, separates key from value
				*ptr = '\0';
				ptr++;
				// the value of the entry starts at the next non space character
				while(*ptr == ' ')
					ptr++;
				cfg->value = ptr;
				// count forward to the null terminator
				while(*ptr)
					ptr++;
				// count back to the last non space, newline or carriage return character
				ptr--;
				while(((*ptr == ' ')||(*ptr == '\n')||(*ptr == '\r')) && ptr >= cfg->buffer)
					ptr--;
				// if we didnt hit the start of the buffer, there must be some content
				if(ptr >= cfg->buffer)
					*(ptr+1) = '\0';
				else
					*(ptr) = '\0';
				cfg->key = cfg->buffer;
				return true;
			}
    	}
    }
	return false;
}

/**
 * @brief	use this function in conjunction with get_next_config().
 * @param   cfg pointer to an initialised config parser structure.
 * @param 	key is the key string to test.
 * @retval	returns true if the specified key matches the current key.
 */
bool config_key_match(config_parser_t* cfg, const uint8_t* key)
{
    return !cfg->key ? false : (bool)!strncmp((const char*)cfg->key, (const char*)key, cfg->buffer_length);
}

/**
 * @brief	use this function in conjunction with get_next_config().
 * @param   cfg pointer to an initialised config parser structure.
 * @param 	value is the key string to test.
 * @retval	returns true if the specified value matches the current value.
 */
bool config_value_match(config_parser_t* cfg, const uint8_t* value)
{
    return !cfg->value ? false : (bool)!strncmp((const char*)cfg->value, (const char*)value, cfg->buffer_length);
}

/**
 * @brief	use this function in conjunction with get_next_config().
 * @param   cfg pointer to an initialised config parser structure.
 * @retval	returns a pointer to the current key string.
 */
const uint8_t* get_config_key(config_parser_t* cfg)
{
    return cfg->key;
}

/**
 * @brief	use this function in conjunction with get_next_config().
 * @param   cfg pointer to an initialised config parser structure.
 * @retval	returns a pointer to the current value string.
 */
const uint8_t* get_config_value(config_parser_t* cfg)
{
    return cfg->value;
}

/**
 * @brief	use this function in conjunction with get_next_config().
 * @param   cfg pointer to an initialised config parser structure.
 * @retval	returns a pointer to the comment, if any, on the current config line.
 */
const uint8_t* get_config_comment(config_parser_t* cfg)
{
	return cfg->comment;
}

/**
 * @brief	checks if a key exists in the file, and returns its value.
 *
 * @param buffer is a pointer to some memory to use. must be long enough to hold one line from the config file.
 * @param buffer_length is the length of the buffer memory in  bytes.
 * @param filepath is the path to the config file.
 * @param key is the key string to write.
 * @retval returns a pointer to the config value corresponding to key, stored in buffer.
 * 			returns NULL if the key wasnt found.
 */
const uint8_t* get_config_value_by_key(uint8_t* buffer, uint16_t buffer_length, const uint8_t* filepath, const uint8_t* key)
{
	config_parser_t cfg;


	if(open_config_file(&cfg, buffer, buffer_length, filepath))
	{
		while(get_next_config(&cfg))
		{
			if(config_key_match(&cfg, key))
			{
				close_config_file(&cfg);
				return get_config_value(&cfg);
			}
		}
		close_config_file(&cfg);
	}
	return NULL;
}

/**
 * @brief	checks if a key exists in a list of strings (array of string pointers).
 * 			the list should be null terminated, eg:
@code
	const uint8_t* list[] = {"abc", "def", NULL};
@endcode
 *
 * @param key is the key string to check.
 * @param key_length is the maximum length of the key string.
 * @param list is an array of string pointers to check against.
 * @retval returns true if the key was found in the list, false otherwise.
 */
bool config_key_in_list(const uint8_t* key, uint16_t key_length, const uint8_t** list)
{
	while(*list != NULL)
	{
		if(!strncmp((const char*)(*list), (const char*)key, key_length))
			return true;
		list++;
	}
	return false;
}

/**
 * compare a string to a well trusted refrence string.
 *
 * @param ref a well trusted string -
 * 			must not be able to cause carnage by being an unterminated bunch of binary :)
 * @param to_compare - some memory to compare to ref. should be a null terminated string.
 * @retval returns true if both ref and to_compare are not NULL and they match to the length of ref.
 */
bool string_match(const char* ref, const uint8_t* to_compare)
{
	return to_compare && ref && strcmp(ref, (const char*)to_compare) == 0;
}

/**
 * @brief writes a key/value entry to a config file.
 *
 * exactly the same as edit_config_entry except that the file is created
 * if it isnt already in existence.
 *
 * @param buffer is a pointer to some memory to use. must be long enough to hold one line from the config file.
 * @param buffer_length is the length of the buffer memory in  bytes.
 * @param filepath is the path to the config file.
 * @param key is the key string to write.
 * @param value is the value string to write.
 */
bool add_config_entry(uint8_t* buffer, uint16_t buffer_length, const uint8_t* filepath, const uint8_t* key, const uint8_t* value)
{
    FILE* newconf;
    struct stat st;

    // check if file exists
    if(stat((const char*)filepath, &st) == EOF)
    {
    	// if not just create a new one and add the entry
    	newconf = fopen((const char*)filepath, "w");
    	if(newconf != NULL)
    	{
			fprintf(newconf, "%s %s\n", (const char*)key, (const char*)value);
			fclose(newconf);
			return true;
    	}
    }
    else
    {
    	// edit the existing file
    	return edit_config_entry(buffer, buffer_length, filepath, key, value);
    }

    return false;
}

/**
 * @brief edits a key/value entry to a config file.
 *
 * the file must already exist. a backup is created and left behind after modification.
 *
 * - if the file does not exist, exits immediately.
 * - if the key exists in the file, its value is modified.
 * - if the key does not already exist, it is added.
 * - whitespace is not guaranteed to be retained
 * - comments will be retained
 *
 * @param buffer is a pointer to some memory to use. must be long enough to hold one line from the config file.
 * @param buffer_length is the length of the buffer memory in  bytes.
 * @param filepath is the path to the config file.
 * @param key is the key string to write.
 * @param value is the value string to write.
 */
bool edit_config_entry(uint8_t* buffer, uint16_t buffer_length, const uint8_t* filepath, const uint8_t* key, const uint8_t* value)
{
    bool set = false;
    FILE* newconf;
    config_parser_t cfg;
    cfg.buffer = buffer;
    cfg.buffer_length = buffer_length;
    cfg.key = NULL;
    cfg.value = NULL;

    // use buffer temporarily to create backup config file name
    snprintf((char*)buffer, buffer_length-1, "%s.bak", filepath);

    // remove old backup if needed
    unlink((const char*)buffer);

    if(rename((const char*)filepath, (const char*)buffer) != -1)
    {
    	// open backup for reading
    	cfg.file = fopen((const char*)buffer, "r");
        if(cfg.file != NULL)
        {
        	// create new version of the config file
        	newconf = fopen((const char*)filepath, "w");
            if(newconf != NULL)
			{
            	// iterate over entries in backup
				while(get_next_config(&cfg))
				{
					if(config_key_match(&cfg, key))
					{
						// if we matched the desired key, add it with the new value
						fprintf(newconf, "%s %s\n", (const char*)key, (const char*)value);
						set = true;
					}
					else
					{
						// otherwise add the whole line from the backup back in
						// this preserves comments
						fprintf(newconf, "%s %s\n", (const char*)get_config_key(&cfg), (const char*)get_config_value(&cfg));
//						fprintf(newconf, "%s\n", cfg.buffer);
					}
				}
				// if we get here and the key wasnt modified, add it
				if(!set)
				{
					// if we matched the desired key, add it with the new value
					fprintf(newconf, "%s %s", (const char*)key, (const char*)value);
					set = true;
				}
				// close new config
				fclose(newconf);
			}
            // close backup file
			close_config_file(&cfg);
        }
    }
    return set;
}

/**
 * @}
 */
