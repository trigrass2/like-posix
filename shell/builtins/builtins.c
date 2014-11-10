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

#include <stdlib.h>
#include <sys/time.h>
#include "builtins.h"

int sh_help(int fdes, const char** args, unsigned char nargs)
{
    (void)fdes;
    (void)args;
    (void)nargs;
    return SHELL_CMD_PRINT_CMDS;
}

int sh_exit(int fdes, const char** args, unsigned char nargs)
{
    (void)fdes;
    (void)args;
    (void)nargs;
    return SHELL_CMD_KILL;
}

int sh_date(int fdes, const char** args, unsigned char nargs)
{
    (void)fdes;
    (void)args;
    (void)nargs;
    int length;
    char* buffer = malloc(32);
    if(buffer)
    {
		struct timeval tv;
		if(gettimeofday(&tv, NULL) == 0)
		{
			length = sprintf(buffer, "%u.%u", tv.tv_sec, tv.tv_usec);
			send(fdes, buffer, length, 0);
		}
		free(buffer);
    }

    return SHELL_CMD_EXIT;
}

shell_cmd_t sh_help_cmd = {
     .name = "help",
     .usage = "prints a list of available commands",
     .cmdfunc = sh_help
};

shell_cmd_t sh_exit_cmd = {
    .name = "exit",
    .usage = "causes the shell session to terminate",
    .cmdfunc = sh_exit
};

shell_cmd_t sh_date_cmd = {
    .name = "date",
    .usage = "prints current date/time",
    .cmdfunc = sh_date
};

