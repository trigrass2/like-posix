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
#include "logger.h"
#include "threaded_server.h"
#include "logger.h"
#include "FreeRTOS.h"
#include "task.h"
#include "confparse.h"

void run_spawned(sock_conn_t* conn);
int spawn_connection(sock_server_t* server, sock_conn_t* conn);

/**
 * used to extract the name, stacksize and task priority from the config file.
 *
 * ensure defaults are set manually before calling or the app may not cope with
 * items that are not found in the config file or a missing config file.
 *
 * Example:
 * servinfo.conns = 0;
 * servinfo.port = 0;
 * servinfo.stacksize = APP_SERVER_STACK_SIZE;
 * servinfo.prio = APP_SERVER_TASK_PRIO;
 * servinfo.name = "serverapp";
 */
void get_server_configuration(const char* configfile, sock_server_t* servinfo)
{
	uint8_t buffer[32];
	const uint8_t* confstr;

	log_init(&servinfo->log, servinfo->name);

	if(stat(configfile, (struct stat*)&buffer) == 0)
	{
	    confstr = get_config_value_by_key(buffer, sizeof(buffer), (const uint8_t*)configfile, (const uint8_t*)"stacksize");
        if(confstr) {
        	servinfo->stacksize = atoi((const char*)confstr);
        	log_info(&servinfo->log, "%s set stacksize: %d", configfile, servinfo->stacksize);
        }

        confstr = get_config_value_by_key(buffer, sizeof(buffer), (const uint8_t*)configfile, (const uint8_t*)"taskprio");
        if(confstr) {
        	servinfo->prio = atoi((const char*)confstr);
        	log_info(&servinfo->log, "%s set threadprio: %d", configfile, servinfo->prio);
        }

		confstr = get_config_value_by_key(buffer, sizeof(buffer), (const uint8_t*)configfile, (const uint8_t*)"name");
		if(confstr)
		{
			servinfo->name = malloc(strlen((const char*)confstr) + 1);
			strcpy((char*)servinfo->name, (const char*)confstr);
			log_info(&servinfo->log, "%s set name: %s", configfile, servinfo->name);
		}

	    confstr = get_config_value_by_key(buffer, sizeof(buffer), (const uint8_t*)configfile, (const uint8_t*)"port");
        if(confstr) {
        	servinfo->port = atoi((const char*)confstr);
        	log_info(&servinfo->log, "%s set port: %d", configfile, servinfo->port);
        }

        confstr = get_config_value_by_key(buffer, sizeof(buffer), (const uint8_t*)configfile, (const uint8_t*)"conns");
        if(confstr) {
        	servinfo->conns = atoi((const char*)confstr);
        	log_info(&servinfo->log, "%s set connections: %d", configfile, servinfo->conns);
        }
	}
	else {
	    log_error(&servinfo->log, "couldnt stat config file %s", configfile);
	}

	log_init(&servinfo->log, servinfo->name);
}

/**
 * starts a threaded server daemon.
 *
 * ensure that the servinfo structure is configured manually or via a call to get_server_configuration().
 */
int start_threaded_server(sock_server_t* servinfo, sock_service_fptr_t threadfunc, void* appdata)
{
	int fd = -1;

    if(!servinfo->conns || !servinfo->port) {
        log_error(&servinfo->log, "port and/or conns settings invalid, %d and %d", servinfo->port, servinfo->conns);
        return fd;
    }

	// create the socket server structures
	fd = sock_server(servinfo->port, SOCK_STREAM, servinfo->conns, servinfo, spawn_connection, threadfunc, appdata, servinfo->name, servinfo->stacksize, servinfo->prio);
	// start a new thread that runs the listener
	if(fd != -1)
	{
		if(xTaskCreate(sock_server_thread, servinfo->name, THREADED_SERVER_STACK_SIZE, servinfo, THREADED_SERVER_PRIORITY, NULL) != pdPASS) {
			log_error(&servinfo->log, "error staring server task");
			stop_threaded_server(servinfo);
			fd = -1;
		}
	}

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
	int ret = xTaskCreate((TaskFunction_t)run_spawned, server->name, server->stacksize, conn, server->prio, NULL);
	if(ret == pdPASS) {
	    return 0;
	}

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
