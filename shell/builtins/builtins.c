/*
 * builtins.c
 *
 *  Created on: 29/10/2014
 *      Author: stuartm
 */

#include "builtins.h"

shell_cmd_t help_cmd = {
     .name = "help",
     .usage = "prints a list of available commands"
};

shell_cmd_t exit_cmd = {
    .name = "exit",
    .usage = "causes the shell session to terminate"
};

/**
 * @brief	help command - prints all valid commands.
 */
int help_sh(int fdes, const char** args, unsigned char nargs)
{
    (void)fdes;
    (void)args;
    (void)nargs;
	return SHELL_CMD_PRINT_CMDS;
}

/**
 * @brief	exit command - allows the shell to exit.
 */
int exit_sh(int fdes, const char** args, unsigned char nargs)
{
    (void)fdes;
    (void)args;
    (void)nargs;
	return SHELL_CMD_KILL;
}

