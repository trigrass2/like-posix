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
 * This file is part of the Appleseed project, <https://github.com/drmetal/app-l-seed>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "builtins.h"
#include "system.h"
#include "cutensils.h"
#include "netconf.h"

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
    (void)args;
    (void)nargs;
    int length;
    char* buffer = malloc(STRING_BUFFER_SIZE);
    char* end;
    if(buffer)
    {
		struct timeval tv;
		if(gettimeofday(&tv, NULL) == 0)
		{
		    struct tm* lt = localtime(&tv.tv_sec);
		    length = strftime(buffer, STRING_BUFFER_SIZE, "%Y-%m-%d %H:%M:%S", lt);
		    end = buffer + length;
		    length += sprintf(end, ".%03d", tv.tv_usec/1000);
		    if(length > 0)
		        write(fdes, buffer, length);
		}
		free(buffer);
    }

    return SHELL_CMD_EXIT;
}

int sh_uname(int fdes, const char** args, unsigned char nargs)
{
    uint8_t* buffer = malloc(STRING_BUFFER_SIZE);
    char* confstr;

    if(buffer)
    {
        bool all = has_switch("-a", args, nargs);

        if(all || has_switch("-n", args, nargs))
        {
            confstr = (char*)get_config_value_by_key(buffer, STRING_BUFFER_SIZE, (const uint8_t*)DEFAULT_RESOLV_CONF_PATH, (const uint8_t*)"hostname");
            if(confstr)
            {
                write(fdes, confstr, strlen(confstr));
                write(fdes, " ", sizeof(" ")-1);
            }
            else
                write(fdes, "unknown ", sizeof("unknown ")-1);
        }
        if(all || has_switch("-o", args, nargs))
        {
            write(fdes, OPERATING_SYSTEM, sizeof(OPERATING_SYSTEM)-1);
            write(fdes, " ", sizeof(" ")-1);
        }
        if(all || has_switch("-k", args, nargs))
        {
            write(fdes, KERNEL_VERSION, sizeof(KERNEL_VERSION)-1);
            write(fdes, " ", sizeof(" ")-1);
        }
        if(all || has_switch("-i", args, nargs))
        {
            write(fdes, BOARD, sizeof(BOARD)-1);
            write(fdes, " ", sizeof(" ")-1);
        }
        if(all || has_switch("-p", args, nargs))
        {
            write(fdes, DEVICE, sizeof(DEVICE)-1);
            write(fdes, " ", sizeof(" ")-1);
        }
        if(all || has_switch("-v", args, nargs))
        {
            write(fdes, PROJECT_VERSION, sizeof(PROJECT_VERSION)-1);
            write(fdes, " ", sizeof(" ")-1);
        }
        write(fdes, SHELL_NEWLINE, sizeof(SHELL_NEWLINE)-1);

        free(buffer);
    }

    return SHELL_CMD_EXIT;
}

int sh_reboot(int fdes, const char** args, unsigned char nargs)
{
    (void)args;
    (void)nargs;

    write(fdes, "rebooting"SHELL_NEWLINE, sizeof("rebooting"SHELL_NEWLINE)-1);
    sleep(1);
    soft_reset();

    return SHELL_CMD_EXIT;
}

int sh_echo(int fdes, const char** args, unsigned char nargs)
{
    const char* echo = arg_by_index(0, args, nargs);

    if(echo)
    	write(fdes, echo, strlen(echo));

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

shell_cmd_t sh_echo_cmd = {
    .name = "echo",
    .usage = "echoes input text",
    .cmdfunc = sh_echo
};

shell_cmd_t sh_uname_cmd = {
    .name = "uname",
    .usage = "Print certain system information" SHELL_NEWLINE
"\t-a\tprint all information"SHELL_NEWLINE
"\t-n\tprint the network node hostname" SHELL_NEWLINE
"\t-o\tprint the operating system" SHELL_NEWLINE
"\t-k\tprint the kernel version" SHELL_NEWLINE
"\t-i\tprint the hardware platform or unknown" SHELL_NEWLINE
"\t-p\tprint the processor type or unknown" SHELL_NEWLINE
"\t-v\tprint the version" SHELL_NEWLINE,
    .cmdfunc = sh_uname
};

shell_cmd_t sh_reboot_cmd = {
    .name = "reboot",
    .usage = "reboots the device",
    .cmdfunc = sh_reboot
};
