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
 * This file is part of the like-posix project, <https://github.com/drmetal/like-posix>
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
    uint8_t* keystart;
    uint8_t* keyend;
    uint8_t* valstart;
    uint8_t* valend;
    uint8_t* comstart;
    uint8_t* ptr;

    cfg->value = NULL;
    cfg->key = NULL;
	cfg->comment = NULL;

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
    	comstart = NULL;
    	cfg->comment = NULL;
		cfg->key = NULL;
		cfg->value = NULL;

		ptr = cfg->buffer;

		// loop while whitespace
		while(*ptr &&  ((*ptr == ' ')||(*ptr == '\t')))
			ptr++;

		// check for end of line, or start of comment
		if((*ptr == '\r')||(*ptr == '\n')||(*ptr == '#'))
		{
			// line is a comment or blank
			if(cfg->retain_comments_newlines)
				return true;
			else
				continue;
		}

		// store key
		keystart = ptr;

		// loop while not whitespace, or end of line is reached
		while(*ptr && (*ptr != ' ') && (*ptr != '\t') && (*ptr != '\r') && (*ptr != '\n') && (*ptr != '#'))
			ptr++;

		// check for end of line only
		if((*ptr == '\r')||(*ptr == '\n')||(*ptr == '#'))
		{
			// we only got the key - the line is corrupt
			if(cfg->retain_comments_newlines)
				return true;
			else
				continue;
		}

		// terminate the key
		keyend = ptr;

		// loop while whitespace
		while(*ptr &&  ((*ptr == ' ')||(*ptr == '\t')))
			ptr++;

		// check for end of line or comment
		if((*ptr == '\r')||(*ptr == '\n')||(*ptr == '#'))
		{
			// we only got the key - the line is corrupt
			if(cfg->retain_comments_newlines)
				return true;
			else
				continue;
		}

		// store value
		valstart = ptr;

		// loop while not whitespace, or end of line is reached
		while(*ptr && (*ptr != ' ') && (*ptr != '\t') && (*ptr != '\r') && (*ptr != '\n') && (*ptr != '#'))
			ptr++;

		// terminate the value
		valend = ptr;

		// loop while whitespace
		while(*ptr &&  ((*ptr == ' ')||(*ptr == '\t')))
			ptr++;

		// check for comment
		if(*ptr == '#')
		{
			comstart = ptr+1;
		}

		cfg->key = keystart;
		*keyend = '\0';
		cfg->value = valstart;
		*valend = '\0';
		cfg->comment = comstart;
		return true;
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
    cfg.retain_comments_newlines = true;
    const char* k;
	const char* v;
    const char* c;

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
					k = (const char*)get_config_key(&cfg);
					v = (const char*)get_config_value(&cfg);
					c = (const char*)get_config_comment(&cfg);

					if(!c && !k && !v)
					{
						// the line contained only a comment - no key/value
						fputs((const char*)cfg.buffer, newconf);
					}
					else if(config_key_match(&cfg, key))
					{
						// if we matched the desired key, add it with the new value
						if(c)
							fprintf(newconf, "%s %s #%s", (const char*)key, (const char*)value, c);
						else
							fprintf(newconf, "%s %s\n", (const char*)key, (const char*)value);
						set = true;
					}
					else
					{
						if(k && v && c)
							fprintf(newconf, "%s %s #%s", k, v, c);
						else if(k && v)
							fprintf(newconf, "%s %s\n", k, v);
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

    cfg.retain_comments_newlines = false;

    return set;
}

/**
 * @}
 */
