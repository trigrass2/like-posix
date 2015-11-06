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


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "strutils.h"
#include "confparse.h"
#include "fs_cmds.h"
#include "shell.h"
#include "sdfs.h"
#include "fs_cmds.h"
#if USE_CONFPARSE
#include "confparse.h"
#pragma message "building shell::fs_cmds with config parser support"
#else
#pragma message "building shell::fs_cmds without config parser support"
#endif

#define IS_NOT_A_DIRECTORY          " is not a directory"
#define FORMATTING_SDCARD           "formatting sdcard"
#define ARGUMENT_NOT_SPECIFIED      "argument not specified"
#define ERROR_OPENING_SOURCE_FILE      "couldnt open source file"
#define ERROR_OPENING_DEST_FILE      "couldnt open destination file"
#define ERROR_MOVING_FILE            "error moving file"
// 16 characters per column
#define DF_CMD_HEADING            "      Filesystem         1K-blocks            Used       Available            Use%      Mounted on"
#define DF_CMD_ROW            	  "%13s:%s%18u%16u%16u%15u%%%16s"

#define CONFIG_CMD_BUFFER_SIZE 		512
#define LS_CMD_BUFFER_SIZE 		256
#define DF_CMD_BUFFER_SIZE 		256

const char* units[] = {
       "b", "kb", "Mb", "Gb"
};

#define PAD_TO_FILESIZE     40
#define PAD_TO_NEXT_FILE    16

shell_cmd_t* install_fs_cmds(shellserver_t* sh)
{
    shell_cmd_t* head;
    register_command(sh, &sh_ls_cmd, NULL, NULL, NULL);
    register_command(sh, &sh_cd_cmd, NULL, NULL, NULL);
    register_command(sh, &sh_rm_cmd, NULL, NULL, NULL);
    register_command(sh, &sh_mkdir_cmd, NULL, NULL, NULL);
    register_command(sh, &sh_cat_cmd, NULL, NULL, NULL);
    register_command(sh, &sh_mv_cmd, NULL, NULL, NULL);
    register_command(sh, &sh_cp_cmd, NULL, NULL, NULL);
    head = register_command(sh, &sh_df_cmd, NULL, NULL, NULL);
    #if USE_CONFPARSE
    head = register_command(sh, &sh_config_cmd, NULL, NULL, NULL);
#endif
    return head;
}

int sh_ls(int fdes, const char** args, unsigned char nargs)
{
    unsigned int i;
    char size[8];
    struct stat st;
    DIR *dir;
    struct dirent *ent;
    bool ll = has_switch((const char*)"-l", args, nargs);
    const char* rel = final_arg(args, nargs);
    char* buffer = malloc(LS_CMD_BUFFER_SIZE);

    if(buffer)
    {
        if(getcwd(buffer, SHELL_CWD_LENGTH_MAX))
        {
            if((nargs == 1 && !ll) || (nargs == 2 && ll))
            {
                strcat(buffer, rel);
            }

            dir = opendir(buffer);

            if(dir)
            {
                while(1)
                {
                    ent = readdir(dir);
                    if(ent == NULL)
                        break;

                    if(ent->d_type == DT_DIR)
                        write(fdes, DIR_TEXT_START, sizeof(DIR_TEXT_START)-1);

                    i = strlen(ent->d_name);
                    write(fdes, ent->d_name, i);

                    if(ent->d_type == DT_DIR)
                        write(fdes, DIR_TEXT_STOP, sizeof(DIR_TEXT_STOP)-1);

                    if(ll)
                    {
                        // pad spaces
                        while(i++ < PAD_TO_FILESIZE)
                            write(fdes, " ", 1);

                        if(ent->d_type == DT_REG)
                        {

                            // if its a regular file
                            if(stat(ent->d_name, &st) == 0)
                            {
                                i = 0;
                                while(st.st_size > 1000 && (i < sizeof(units)/sizeof(units[0])))
                                {
                                    i++;
                                    st.st_size /= 1000;
                                }
                                sprintf(size, "%d%s", st.st_size, units[i]);
                                write(fdes, size, strlen(size));
                            }
                        }
                        else
                            write(fdes, "-", 1);

                        write(fdes, SHELL_NEWLINE, sizeof(SHELL_NEWLINE)-1);
                    }
                    else
                    {
                        // pad spaces
                        while(i++ < PAD_TO_NEXT_FILE)
                            write(fdes, " ", 1);
                    }
                }
                closedir(dir);
            }
        }

        free(buffer);
    }
    return SHELL_CMD_EXIT;
}

int sh_cd(int fdes, const char** args, unsigned char nargs)
{
    const char* path;

    if(!nargs)
        path = sdfs_mountpoint();
    else
        path = arg_by_index(0, args, nargs);

    if(chdir(path) == -1)
    {
      write(fdes, path, strlen(path));
      write(fdes, IS_NOT_A_DIRECTORY, sizeof(IS_NOT_A_DIRECTORY)-1);
    }

    return SHELL_CMD_CHDIR;
}

int sh_rm(int fdes, const char** args, unsigned char nargs)
{
    unsigned char arg = 0;

    if(!nargs)
    {
        write(fdes, ARGUMENT_NOT_SPECIFIED, sizeof(ARGUMENT_NOT_SPECIFIED)-1);
    }

    while(arg < nargs)
    {
        unlink(arg_by_index(arg, args, nargs));
        arg++;
    }

    return SHELL_CMD_EXIT;
}

int sh_mkdir(int fdes, const char** args, unsigned char nargs)
{
    const char* dir = final_arg(args, nargs);

    if(dir)
        mkdir(dir, 0777);
    else
        write(fdes, ARGUMENT_NOT_SPECIFIED, sizeof(ARGUMENT_NOT_SPECIFIED)-1);

    return SHELL_CMD_EXIT;
}

int sh_cat(int fdes, const char** args, unsigned char nargs)
{
	(void)nargs;
	char buffer[64];
	int len;
	int ffd = open(args[0], O_RDONLY);

	if(ffd != -1)
	{
		len = 1;
		while(len > 0)
		{
			len = read(ffd, buffer, sizeof(buffer));
			if(len > 0)
				write(fdes, buffer, len);
		}
		close(ffd);
	}
    return SHELL_CMD_EXIT;
}

int sh_mv(int fdes, const char** args, unsigned char nargs)
{
    const char* path = arg_by_index(0, args, nargs);
    const char* newpath = arg_by_index(1, args, nargs);

    if(path && newpath)
    {
        if(rename(path, newpath) == -1)
            write(fdes, ERROR_MOVING_FILE, sizeof(ERROR_MOVING_FILE)-1);
    }
    else
        write(fdes, ARGUMENT_NOT_SPECIFIED, sizeof(ARGUMENT_NOT_SPECIFIED)-1);

    return SHELL_CMD_EXIT;
}

int sh_cp(int fdes, const char** args, unsigned char nargs)
{
    FILE* f1;
    FILE* f2;
    const char* path = arg_by_index(0, args, nargs);
    const char* newpath = arg_by_index(1, args, nargs);
    char buffer[64];
    int length;

    if(path && newpath)
    {
        f1 = fopen(path, "r");
        if(f1)
        {
            f2 = fopen(newpath, "w");
            if(f2)
            {
                length = 1;
                while(length > 0)
                {
                    length = fread(buffer, 1, sizeof(buffer), f1);
                    fwrite(buffer, 1, length, f2);
                }
                fclose(f2);
            }
            else
                write(fdes, ERROR_OPENING_DEST_FILE, sizeof(ERROR_OPENING_DEST_FILE)-1);
            fclose(f1);
        }
        else
            write(fdes, ERROR_OPENING_SOURCE_FILE, sizeof(ERROR_OPENING_SOURCE_FILE)-1);
    }
    else
        write(fdes, ARGUMENT_NOT_SPECIFIED, sizeof(ARGUMENT_NOT_SPECIFIED)-1);

    return SHELL_CMD_EXIT;
}

#if USE_CONFPARSE
int sh_config(int fdes, const char** args, unsigned char nargs)
{
    const char* file = arg_by_index(0, args, nargs);
    const char* key = arg_by_index(1, args, nargs);
    const char* value = arg_by_index(2, args, nargs);
    uint8_t* buffer;
    bool success = true;
    int length;

    buffer = malloc(CONFIG_CMD_BUFFER_SIZE);

    if(buffer)
    {
	    if(file && key && value)
	    {
	    	success = add_config_entry(buffer, CONFIG_CMD_BUFFER_SIZE, (const uint8_t*)file, (const uint8_t*)key, (const uint8_t*)value);
	    	if(!success)
	    	{
	            length = sprintf((char*)buffer, "couldnt add %s=%s to %s" SHELL_NEWLINE, key, value, file);
	            write(fdes, (char*)buffer, length);
	    	}
	    }
	    // todo - i get a hardfault in the next if block (if(success && value)) when I enable this :(
//	    else if(file && key)
//	    {
//	    	value = (const char*)get_config_value_by_key(buffer, CONFIG_CMD_BUFFER_SIZE, (const uint8_t*)file, (const uint8_t*)key);
//	    	if(!value)
//	    	{
//	            length = sprintf((char*)buffer, "couldnt read %s from %s" SHELL_NEWLINE, key, file);
//	            write(fdes, (char*)buffer, length);
//	    	}
//	    }

		if(success && value)
		{
			length = sprintf((char*)buffer, "pair: %s=%s" SHELL_NEWLINE, key, value);
			write(fdes, (char*)buffer, length);
		}

		free(buffer);
    }

    return SHELL_CMD_EXIT;
}
#endif

int sh_df(int fdes, const char** args, unsigned char nargs)
{
	(void)nargs;
	(void)args;
    char* buffer = malloc(DF_CMD_BUFFER_SIZE);
    uint32_t sectors = sdfs_sector_count();
    uint32_t free_clusters = sdfs_clusters_free();
    uint32_t sectors_per_cluster = sdfs_cluster_size();
    uint32_t used = (sectors - (free_clusters*sectors_per_cluster))/2;
    uint32_t available = (free_clusters*sectors_per_cluster)/2;

    if(buffer)
    {
		write(fdes, DF_CMD_HEADING SHELL_NEWLINE, sizeof(DF_CMD_HEADING SHELL_NEWLINE)-1);
		int length = sprintf(buffer, DF_CMD_ROW SHELL_NEWLINE,
				sdfs_drive_name(), sdfs_drive_mapping(), sectors/2, used, available, used/available, sdfs_mountpoint());
		write(fdes, (char*)buffer, length);
		free(buffer);
    }

    return SHELL_CMD_EXIT;
}

shell_cmd_t sh_df_cmd = {
    .name = "df",
    .usage =
"prints disk usage information",
    .cmdfunc = sh_df
};

shell_cmd_t sh_ls_cmd = {
    .name = "ls",
    .usage =
"prints directory content, relative to the current directory" SHELL_NEWLINE \
"flags:" SHELL_NEWLINE \
"\t-l  print details" SHELL_NEWLINE \
"ls [-l] [relpath]",
    .cmdfunc = sh_ls
};

shell_cmd_t sh_cd_cmd = {
    .name = "cd",
    .usage = "changes the current working directory",
    .cmdfunc = sh_cd
};

shell_cmd_t sh_rm_cmd = {
    .name = "rm",
    .usage = "removes the specified file(s)" SHELL_NEWLINE \
"rm file [file file ...]",
    .cmdfunc = sh_rm
};

shell_cmd_t sh_mkdir_cmd = {
    .name = "mkdir",
    .usage = "creates the specified directory",
    .cmdfunc = sh_mkdir
};

shell_cmd_t sh_cat_cmd = {
        .name = "cat",
        .usage = "reads the entire content of a file to the screen",
        .cmdfunc = sh_cat
};

shell_cmd_t sh_mv_cmd = {
		.name = "mv",
		.usage = "moves/renames a file" SHELL_NEWLINE \
"mv oldname newname",
		.cmdfunc = sh_mv
};

shell_cmd_t sh_cp_cmd = {
        .name = "cp",
        .usage = "copies a file from one location to another" SHELL_NEWLINE \
"cp file newfile",
        .cmdfunc = sh_cp
};

shell_cmd_t sh_config_cmd = {
        .name = "config",
        .usage = "configs, adds, reads lines from config files" SHELL_NEWLINE \
"config file [key] [value]",
        .cmdfunc = sh_config
};

