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

#include "startup_script.h"

#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include "shell.h"

/**
 * runs all shell scripts in the "location" directory.
 * intended for use in a similar context to upstart or systemd, for automated configuration
 * of the system or starting threads just after boot time.
 *
 * requires either:
 *
 * - a shellserver_t structure that has already had commands registered.
 * - or a blank shellserver_t structure, and a pointer to the command set of an existing shell.
 *
 * Example usage:
 *
@code

#include "shell.h"
#include "startup_script.h"
#include "fs_cmds.h"
#include "os_cmds.h"
#include "net_cmds.h"

shellserver_t shell;

install_fs_cmds(&shell);
install_net_cmds(&shell);
install_os_cmds(&shell);
// application specific commands here too ...

startup_script_runner(&shell, NULL, "/etc/init", ".conf");

@endcode
 */
void startup_script_runner(shellserver_t* shell, shell_cmd_t* commandset, const char* location, const char* extension)
{
	const char* dot;
	int rdfd;
	int wrfd;
    DIR *dir;
    struct dirent *ent;
    char log_file[128];

    mkdir(DEFAULT_STARTUP_SCRIPT_LOG_PATH, 0777);

	dir = opendir(location);
	if(dir)
	{
		for(ent=readdir(dir); ent; ent=readdir(dir))
		{
			dot = strrchr(ent->d_name, extension[0]);

			if(dot && strcmp(dot, extension) == 0)
			{
				strcpy(log_file, location);
				strncat(log_file, "/", sizeof(log_file)-1);
				strncat(log_file, ent->d_name, sizeof(log_file)-1);
				rdfd = open(log_file, O_RDONLY);

				strcpy(log_file, DEFAULT_STARTUP_SCRIPT_LOG_PATH);
				strncat(log_file, "/", sizeof(log_file)-1);
				strncat(log_file, ent->d_name, sizeof(log_file)-1);
				strncat(log_file, DEFAULT_STARTUP_LOG_EXTENSION, sizeof(log_file)-1);
				wrfd = open(log_file, O_WRONLY|O_CREAT|O_TRUNC);


				if(rdfd != -1)
				{
					start_shell(shell, commandset, NULL, false, true, rdfd, wrfd);
					close(rdfd);
				}

				if(wrfd != -1)
					close(wrfd);
			}
		}

		closedir(dir);
	}
}
