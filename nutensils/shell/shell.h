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
 * This file is part of the lollyjar project, <https://github.com/drmetal/lollyjar>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

#ifndef SHELL_H_
#define SHELL_H_

#include "shell_command.h"
#include "shell_defs.h"

#define SHELL_TASK_STACK_SIZE 	256
#define SHELL_TASK_PRIORITY 	1

#define INCLUDE_REMOTE_SHELL_SUPPORT    USE_SOCK_UTILS && USE_THREADED_SERVER

#define DEFAULT_SHELL_CONFIG_PATH   "/etc/shell/shelld_config"

#if INCLUDE_REMOTE_SHELL_SUPPORT
#include "threaded_server.h"
#endif

/**
 * CMD_BUFFER_SIZE is the size of the shell command input buffer in bytes.
 */
#define SHELL_CMD_BUFFER_SIZE       128
#define SHELL_HISTORY_LENGTH        1
#define SHELL_MAX_ARGS              16

#define SHELL_CWD_LENGTH_MAX        256

typedef struct {
    shell_cmd_t* head_cmd;
    int rdfd;
    int wrfd;
    bool exit_on_eof;
#if INCLUDE_REMOTE_SHELL_SUPPORT
    sock_server_t server;
#endif
}shellserver_t;

typedef struct _shell_t shell_t;


int start_shell(shellserver_t* shell, shell_cmd_t* commandset, const char* configfile, bool threaded, bool exit_on_eof, int rdfd, int wrfd);
shell_cmd_t* register_command(shellserver_t* shell, shell_cmd_t* cmd, shell_cmd_func_t cmdfunc, const char* name, const char* usage);
extern int _system(const char* inputstr);

#endif /* SHELL_H_ */
