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

#ifndef SHELL_H_
#define SHELL_H_

#include "threaded_server.h"
#include "shell_command.h"
#include "shell_defs.h"

#define SHELL_TASK_STACK_SIZE 	64
#define SHELL_TASK_PRIORITY 	1

#define DEFAULT_SHELL_CONFIG_PATH   "/etc/shell/shelld_config"

/**
 * CMD_BUFFER_SIZE is the size of the shell command input buffer in bytes.
 */
#define SHELL_CMD_BUFFER_SIZE       128
#define SHELL_HISTORY_LENGTH        4
#define SHELL_MAX_ARGS              16

typedef struct {
    char* name;
    shell_cmd_t* head_cmd;
    sock_server_t server;
}shellserver_t;

typedef struct _shell_t shell_t;

int start_shell(shellserver_t* shell, char* configfile);
void register_command(shellserver_t* sh, shell_cmd_t* cmd, shell_cmd_func_t cmdfunc, const char* name, const char* usage);

#endif /* SHELL_H_ */
