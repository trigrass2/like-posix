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
#include <string.h>
#include "shell.h"
#include "fs_cmds.h"
#include "strutils.h"

#define TEXTED_PAGESIZE     2048

#define CTRL_X              0x18

#define BACK_A_LINE()       write(fdes, SHELL_PREVIOUS_LINE, sizeof(SHELL_PREVIOUS_LINE)-1, 0);
#define IS_NEWLINE()        (buffer[index] == '\n')
#define write_CURRENT_BUF()  write(fdes, &buffer[index], 1, 0)



int sh_texted(int fdes, const char** args, unsigned char nargs);

shell_cmd_t sh_texted_cmd = {
        .name = "texted",
        .usage = "tiny text editor",
        .cmdfunc = sh_texted
};

void install_texted(shellserver_t* sh)
{
    register_command(sh, &sh_texted_cmd, NULL, NULL, NULL);
}

int sh_texted(int fdes, const char** args, unsigned char nargs)
{
    (void)nargs;
    int file = -1;
    char* buffer = NULL;
//    int length;
    char input = 1;

    int index = 0;
    int character = 0;
    int line = 0;

    int i;

    if(args[0])
    {
        file = open(args[0], O_RDWR | O_CREAT);
        if(file != -1)
        {
            write(fdes, "\n********* ", sizeof("\n********* ")-1);
            write(fdes, "TextEd: ", sizeof("TextEd: ")-1);
            write(fdes, args[0], strlen(args[0]));
            write(fdes, " *********\n", sizeof(" *********\n")-1);
            write(fdes, "CTRL+X to exit\n", sizeof("CTRL+X to exit\n")-1);
            write(fdes, "Line:         Character:         \n\n", sizeof("Line:         Character:         \n\n")-1);

            buffer = calloc(TEXTED_PAGESIZE, 1);

            if(buffer)
            {
                read(file, buffer, TEXTED_PAGESIZE-1);

                // write buffer
                write(fdes, buffer, strlen(buffer));

                // rewind buffer
                for(i = 0; buffer[i]; i++)
                {
                    if(buffer[i] == '\n')
                        write(fdes, SHELL_PREVIOUS_LINE, sizeof(SHELL_PREVIOUS_LINE)-1);
                }

                while(input > 0)
                {

                    // position cursor
                    for(i = 0; i < line; i++)
                        write(fdes, "\n", sizeof("\n")-1);
                    for(i = 0; i < character; i++)
                        write(fdes, SHELL_RIGHTARROW, sizeof(SHELL_RIGHTARROW)-1);

                    if(read(fdes, &input, 1) < 1)
                        break;

                    switch(input)
                    {
                        case CTRL_X:
                            // exit
                            input = 0;
                        break;

                        case 0x1B:
                            read(fdes, &input, 1);
                            switch(input)
                            {
                                case 0x5B:
                                    read(fdes, &input, 1);
                                    switch(input)
                                    {
                                        case 0x44:
                                            // move left
                                            if(index > 0)
                                                index--;
                                            if(buffer[index] == '\n')
                                            {
                                                line--;

                                                character = 0;
                                                i = 0;
                                                while(1)
                                                {
                                                    // count lines
                                                    if(buffer[i] == '\n')
                                                        character++;
                                                    i++;

                                                    // till we hit the current line
                                                    if(character == line)
                                                    {
                                                        // count characters
                                                        for(character = 0; buffer[i] != '\n' && buffer[i]; character++, i++);
                                                        break;
                                                    }
                                                }
                                            }
                                            else
                                                character--;
                                        break;
                                        case 0x43:
                                            // move right
                                            if(index < TEXTED_PAGESIZE-2)
                                                index++;
                                            if(buffer[index] == '\n')
                                            {
                                                line++;
                                                character = 0;
                                            }
                                            else
                                                character++;
                                        break;
                                    }
                                 break;
                            }
                        break;

                        default:
                            // shift all characters forward

                            // insert character

                        break;
                    }

                    // write buffer
                    write(fdes, buffer, strlen(buffer));

                    // rewind buffer
                    for(i = 0; buffer[i]; i++)
                    {
                        if(buffer[i] == '\n')
                            write(fdes, SHELL_PREVIOUS_LINE, sizeof(SHELL_PREVIOUS_LINE)-1);
                    }
                }
            }
        }
    }

    if(file != -1 && buffer)
    {
        lseek(file, 0, SEEK_SET);
        write(file, buffer, strlen(buffer));
        close(file);
    }

    if(buffer)
        free(buffer);

    return SHELL_CMD_EXIT;
}


