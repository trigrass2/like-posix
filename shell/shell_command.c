/*
 * shell_command.c
 *
 *  Created on: 29/10/2014
 *      Author: stuartm
 */

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
    cmd->cmdfunc = cmdfunc;
	cmd->next = NULL;
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
	    send(fdes, cmd->name, strlen(cmd->name), 0);
        send(fdes, SHELL_USAGE_STR, sizeof(SHELL_USAGE_STR)-1, 0);
        send(fdes, cmd->usage, strlen(cmd->usage), 0);
	}
	else
	{
        send(fdes, SHELL_NO_HELP_STR, sizeof(SHELL_NO_HELP_STR)-1, 0);
        send(fdes, cmd->name, strlen(cmd->name), 0);
	}
}
