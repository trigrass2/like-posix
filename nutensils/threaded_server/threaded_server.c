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

/**
* @defgroup threaded_server Threaded Server
*
* multithreaded socket server - relies upon freertos, cutensils, like-posix,
* a network stack that supports posix style network calls - lwip for example.
*
* creates a listener and starts a new thread to handle
*
* @{
* @file threaded_server.c
*/

#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "cutensils.h"
#include "threaded_server.h"
#include "cutensils.h"
#include "FreeRTOS.h"
#include "task.h"

void run_spawned(sock_conn_t* conn);
int spawn_connection(sock_server_t* server, sock_conn_t* conn);

int start_threaded_server(sock_server_t* servinfo, const char* config, sock_service_fptr_t threadfunc, void* data, int stacksize, int prio)
{
	uint8_t buffer[32];
	int fd = -1;
	const uint8_t* confstr;
	int port = 0;
	int conns = 0;
	const char* name = NULL;

	if(stat(config, (struct stat*)&buffer) == 0)
	{
	    confstr = get_config_value_by_key(buffer, sizeof(buffer), (const uint8_t*)config, (const uint8_t*)"port");
        if(confstr)
            port = atoi((const char*)confstr);
        confstr = get_config_value_by_key(buffer, sizeof(buffer), (const uint8_t*)config, (const uint8_t*)"conns");
        if(confstr)
            conns = atoi((const char*)confstr);
        confstr = get_config_value_by_key(buffer, sizeof(buffer), (const uint8_t*)config, (const uint8_t*)"name");
        if(confstr)
        {
            name = malloc(strlen((const char*)confstr) + 1);
            strcpy((char*)name, (const char*)confstr);
        }

        if(port && conns)
        {
            // create the socket server structures
            fd = sock_server(port, SOCK_STREAM, conns, servinfo, spawn_connection, threadfunc, data, name, stacksize, prio);
            // start a new thread that runs the listener
            if(fd != -1)
            {
                if(xTaskCreate(sock_server_thread, name,
                                configMINIMAL_STACK_SIZE + THREADED_SERVER_STACK_SIZE,
                                servinfo, tskIDLE_PRIORITY + THREADED_SERVER_PRIORITY,
                                NULL) != pdPASS) {
                    log_error(&servinfo->log, "error staring server task");
                    stop_threaded_server(servinfo);
                    fd = -1;
                }
            }
        }
        else
            log_error(&servinfo->log, "port and/or conns settings invalid, %d and %d", port, conns);
	}
	else
	    log_error(&servinfo->log, "couldnt stat config file %s", config);

	return fd;
}

void stop_threaded_server(sock_server_t* servinfo)
{
    sock_server_kill(servinfo);

    if(servinfo->name)
        free((char*)servinfo->name);
}

int spawn_connection(sock_server_t* server, sock_conn_t* conn)
{
	int ret = xTaskCreate((void(*)(void*))run_spawned,
						server->name,
						configMINIMAL_STACK_SIZE + server->stacksize,
						conn,
						tskIDLE_PRIORITY + server->prio, NULL);
	if(ret == pdPASS)
	    return 0;

	log_error(&server->log, "error spawning connection task (%d)", ret);
	return -1;
}

void run_spawned(sock_conn_t* conn)
{
	if(conn)
	{
		conn->service(conn);
	    log_debug(NULL, "closing connection with %s", inet_ntoa(conn->cliaddr.sin_addr));
		closesocket(conn->connfd);
		free(conn);
	}
	else
	    log_error(NULL, "spawned with no connection data");

	vTaskDelete(NULL);
}

/**
 * @}
 */
