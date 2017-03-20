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
    SHELL_CMD_CHDIR = -7,           ///< change shell cwd
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
