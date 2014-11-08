/*
 * shell_command.h
 *
 *  Created on: 29/10/2014
 *      Author: stuartm
 */

#ifndef SHELL_COMMAND_H_
#define SHELL_COMMAND_H_

#include <sys/socket.h>
#include <stdint.h>
#include <stdbool.h>

#include "shell_defs.h"

typedef enum {
	SHELL_CMD_EXIT = 0,             ///< exit cmd and return to prompt
	SHELL_CMD_CONTINUE = -1,            ///< kill shell
    SHELL_CMD_KILL = -2,            ///< kill shell
	SHELL_CMD_PRINT_CMDS = -3,      ///< dump shell cmd list
	SHELL_CMD_PRINT_USAGE = -4,     ///< print usage string
	SHELL_CMD_IO_ERROR = -5,        ///< IO error
    SHELL_CMD_MEM_ERROR = -6,       ///< memory error
}shell_cmd_special_returns_t;

typedef struct _shell_cmd_t shell_cmd_t;

typedef int(*shell_cmd_func_t)(int fdes, const char** args, unsigned char nargs);

typedef struct _shell_cmd_t {
    const char* name;
    shell_cmd_t* next;
    const char* usage;
    shell_cmd_func_t cmdfunc;
}shell_cmd_t;

void shell_cmd_init(shell_cmd_t* cmd, shell_cmd_func_t cmdfunc, const char* name, const char* usage);
int shell_cmd_exec(shell_cmd_t* cmf, int fdes, const char** args, unsigned char nargs);
const char* arg_by_switch(const char* sw, const char** args, unsigned char nargs);
const char* arg_by_index(unsigned char index, const char** args, unsigned char nargs);
const char* final_arg(const char** args, unsigned char nargs);
bool has_switch(const char* sw, const char** args, unsigned char nargs);
void cmd_usage(shell_cmd_t* cmd, int fdes);

#endif /* SHELL_COMMAND_H_ */
