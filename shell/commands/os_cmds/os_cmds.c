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


#include "os_cmds.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "shell.h"
#include "FreeRTOS.h"
#include "task.h"

#define TOP_LINE_BUFFER_SIZE            512
#define TOP_HEADER                      "       PID      State       HWM   RUNTIME\tNAME"SHELL_NEWLINE
#define BLANK_EOL                       ANSII_CLEAR_LINE_FROM SHELL_NEWLINE
#define REMOVE_PREV_LINE                SHELL_PREVIOUS_LINE ANSII_CLEAR_LINE

void install_os_cmds(shellserver_t* sh)
{
    register_command(sh, &sh_top_cmd, NULL, NULL, NULL);
}

int sh_top(int fdes, const char** args, unsigned char nargs)
{
    char dec = 0;
    int n = 1;
    int rate = 2;
    char code;
    int ret;
    int length;
    unsigned long uptime;
    int i;
    int ntasks = 0;
    TaskStatus_t* state_table;
    time_t t;
    struct tm* tm;
    char* buffer;

    buffer = (char*)arg_by_switch("-n", args, nargs);
    if(buffer)
    {
        dec = 1;
        n = atoi(buffer);
        if(n < 1)
            n = 1;
    }
    buffer = (char*)arg_by_switch("-d", args, nargs);
    if(buffer)
    {
        rate = atoi(buffer);
        if(rate < 1)
            rate = 2;
    }

    buffer = malloc(TOP_LINE_BUFFER_SIZE);

    if(buffer)
    {
        for(;;)
        {
            ntasks = uxTaskGetNumberOfTasks();
            state_table = malloc(ntasks * sizeof(TaskStatus_t));
            if(state_table)
            {
                t = time(NULL);
                tm = localtime(&t);

                uxTaskGetSystemState(state_table, ntasks, &uptime);

                length = snprintf(buffer, TOP_LINE_BUFFER_SIZE-1, "top - %02d:%02d:%02d up %d days n %d"BLANK_EOL, tm->tm_hour, tm->tm_min, tm->tm_sec, uptime, n);
                send(fdes, buffer, length, 0);

                length = snprintf(buffer, TOP_LINE_BUFFER_SIZE-1, "Tasks: %d\tMem: %d/%db"BLANK_EOL, ntasks, configTOTAL_HEAP_SIZE-xPortGetFreeHeapSize(), configTOTAL_HEAP_SIZE);
                send(fdes, buffer, length, 0);

                send(fdes, TOP_HEADER, sizeof(TOP_HEADER)-1, 0);

                for(i = 0; i < ntasks; i++)
                {
                    length = snprintf(buffer, TOP_LINE_BUFFER_SIZE-1, "% 10d % 10d% 10d% 10d\t%s"BLANK_EOL, state_table[i].xHandle, state_table[i].eCurrentState, state_table[i].usStackHighWaterMark, state_table[i].ulRunTimeCounter, state_table[i].pcTaskName);
                    send(fdes, buffer, length, 0);
                }

                send(fdes, ANSII_CLEAR_LINE, sizeof(ANSII_CLEAR_LINE)-1, 0);

                free(state_table);
                sleep(rate);
            }

            code = 0;
            length = 0;
            ret = ioctlsocket(fdes, FIONREAD, &length);
            if(length > 0)
            {
                ret = recv(fdes, &code, 1, 0);
                if(ret > 0)
                    ret = 0;
            }

            if(dec)
                n--;

            if((ret != 0) || (code == 'q') || (n == 0))
                break;
            else if (code == '\n')
                send(fdes, SHELL_PREVIOUS_LINE, sizeof(SHELL_PREVIOUS_LINE)-1, 0);

            for(i = 0; i < ntasks; i++)
                send(fdes, REMOVE_PREV_LINE, sizeof(REMOVE_PREV_LINE)-1, 0);
            for(; i < ntasks + 3; i++)
                send(fdes, SHELL_PREVIOUS_LINE, sizeof(SHELL_PREVIOUS_LINE)-1, 0);
        }

        free(buffer);
    }

    return SHELL_CMD_EXIT;
}

shell_cmd_t sh_top_cmd = {
    .name = "top",
    .usage = "prints task info" SHELL_NEWLINE
"flags:" SHELL_NEWLINE
"\t-d\t update speed in seconds, defaults to 2."SHELL_NEWLINE
"\t-n\t the number of times to cycle. runs continuously if not specified.",
    .cmdfunc = sh_top
};

