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

/**
* @addtogroup sock_utils
*
* @{
* @file sock_utils.h
*/

#ifndef SOCKET_SOCK_UTILS_H_
#define SOCKET_SOCK_UTILS_H_

#include <sys/types.h>
#include <sys/socket.h>
#include "cutensils.h"

typedef struct _sock_server_t sock_server_t;
typedef struct _sock_conn_t sock_conn_t;

typedef int(*sock_handle_incoming_fptr_t)(sock_server_t*,sock_conn_t*);
typedef void(*sock_service_fptr_t)(sock_conn_t*);

typedef struct _sock_conn_t {
	struct sockaddr_in cliaddr;
	socklen_t clilen;
	int connfd;
	void* ctx;
	sock_service_fptr_t service;
}sock_conn_t;

typedef struct _sock_server_t {
	int listenfd;
	logger_t log;
	const char* name;
	void* ctx;
	sock_handle_incoming_fptr_t handle_incoming;
	sock_service_fptr_t service;
	int stacksize;
	int prio;
}sock_server_t;

int sock_connect(const char *host, int port, int type, struct sockaddr* servaddr);
int sock_server(int port, int type, int conns, sock_server_t* servinfo,
				sock_handle_incoming_fptr_t handle_incoming, sock_service_fptr_t service,
				void* ctx, const char* name, int stacksize, int prio);
void sock_server_thread(void* parameters);
void sock_server_kill(sock_server_t* servinfo);

#endif /* SOCKET_SOCK_UTILS_H_ */

/**
 * @}
 */
