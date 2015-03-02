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
 * This file is part of the nutensils project, <https://github.com/drmetal/nutensils>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "shell_command.h"

static int check_resources(int fdes, const char** args, unsigned char nargs);

/**
 * @brief	shell_cmd_t constructor.
 */
void shell_cmd_init(shell_cmd_t* cmd, shell_cmd_func_t cmdfunc, const char* name, const char* usage)
{
	cmd->next = NULL;
	if(cmdfunc)
	    cmd->cmdfunc = cmdfunc;
    if(usage)
        cmd->usage = usage;
    if(name)
        cmd->name = name;
}

/**
 * @brief   called before the command is called by the shell, hooks up memory references for use by the command.
 */
int shell_cmd_exec(shell_cmd_t* cmd, int fdes, const char** args, unsigned char nargs)
{
    int ret = check_resources(fdes, args, nargs);
    if(ret == SHELL_CMD_CONTINUE)
        ret = cmd->cmdfunc(fdes, args, nargs);
    return ret;
}

/**
 * @brief	base shell_cmd_t object function call.
 * @brief	args is a pointer to a list of string pointers - each string contains the test for one argument.
 * @brief	nargs is the number of arguments in the args list.
 * @retval
 */
int check_resources(int fdes, const char** args, unsigned char nargs)
{
	if(fdes  == -1)
	    return SHELL_CMD_IO_ERROR;
	if(has_switch((const char*)"-h", args, nargs))
		return SHELL_CMD_PRINT_USAGE;
	return SHELL_CMD_CONTINUE;
}

/**
 * @brief	returns a pointer to the string that follows the switch specified.
 */
const char* arg_by_switch(const char* sw, const char** args, unsigned char nargs)
{
	const char* arg;

	while(*args && nargs--)
	{
		if(!strcmp((const char*)*args, (const char*)sw))
			break;
		args++;
	}
	if(*args && *(args+1) && nargs)
	{
		arg = *(args+1);

		// return the argument if it doesnt start with '-'
		if(arg[0] != '-')
			return arg;
		// if it starts with '-', return it only if it looks like a numeric value
		else if(arg[1] >= '0' && arg[1] <= '9')
			return arg;
	}

	return NULL;
}

/**
 * @brief	returns the arg at the specified index, or NULL if no arg exists at index.
 */
const char* arg_by_index(unsigned char index, const char** args, unsigned char nargs)
{
	if(nargs && (index < nargs))
		return *(args+index);
	else
		return NULL;
}

/**
 * @brief	returns the last arg in the list, or NULL if no arg exists.
 */
const char* final_arg(const char** args, unsigned char nargs)
{
	if(nargs)
		return *(args+nargs-1);
	else
		return NULL;
}

/**
 * @brief	returns true if the switch specified exists.
 */
bool has_switch(const char* sw, const char** args, unsigned char nargs)
{
	while(*args && nargs--)
	{
		if(!strcmp((const char*)*args, (const char*)sw))
			return true;
		args++;
	}
	return false;
}

/**
 * @brief	prints a string for the usage of the comnmand required.
 */
void cmd_usage(shell_cmd_t* cmd, int fdes)
{
	if(cmd->usage)
	{
	    write(fdes, cmd->name, strlen(cmd->name));
        write(fdes, SHELL_USAGE_STR, sizeof(SHELL_USAGE_STR)-1);
        write(fdes, cmd->usage, strlen(cmd->usage));
	}
	else
	{
        write(fdes, SHELL_NO_HELP_STR, sizeof(SHELL_NO_HELP_STR)-1);
        write(fdes, cmd->name, strlen(cmd->name));
	}
}
