/*
 * shell.h
 *
 *  Created on: 29/10/2014
 *      Author: stuartm
 */

#ifndef SHELL_DEFS_H_
#define SHELL_DEFS_H_

/**
 * SHELL_NEWLINE_PROMPT is the string that is printed a prompt.
 */
#define SHELL_PROMPT            "# "

#define SHELL_UPARROW			"\x1B\x5B\x41"
#define SHELL_DOWNARROW			"\x1B\x5B\x42"
#define SHELL_LEFTARROW			"\x1B\x5B\x44"
#define SHELL_RIGHTARROW		"\x1B\x5B\x43"
#define SHELL_PREVIOUS_LINE		"\x1B[1F"

#define SHELL_NO_SUCH_COMMAND       SHELL_NEWLINE "no such command "
#define SHELL_HELP_STR              "the following commands exist:"
#define SHELL_NO_HELP_STR           "no help for command "
#define SHELL_USAGE_STR             " usage"SHELL_NEWLINE

/**
 * SHELL_NEWLINE is the type of newline to use throughout the shell system.
 */
#define SHELL_NEWLINE           "\r\n"

#endif /* SHELL_DEFS_H_ */
