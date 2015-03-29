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

/**
* @defgroup sock_utils Socket Utilities
*
* @{
* @file sock_utils.c
*/

#include <netdb.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "sock_utils.h"

#include "FreeRTOS.h"
#include "task.h"

/**
 * open a socket.
 *
 * @param   host may be an IP address or hostname.
 * @param   port is the port to open.
 * @param   type is the type of socket to open - generally SOCK_STREAM or SOCK_DGRAM.
 * @param   when type is SOCK_DGRAM, servaddr is required. servaddr will be populated
 *          by sock_connect, it need not be zeroed beforehand. for SOCK_STREAM connections,
 *          set to NULL.
 * @retval  returns the socket file descriptor, or -1 on error.
 *
 *
 \code

 char buf[32];

 int fd = sock_connect("google.com", 80, SOCK_STREAM, NULL) != -1);

 if(fd != -1)
 {
     send(fd, "hello", strlen("hello"), 0);
     recv(fd, buf, sizeof(buf), 0);

     socketclose(fd);
 }

 \endcode

 \code

 char buf[32];
 struct sockaddr_in servaddr;

 int fd = sock_connect("google.com", 80, SOCK_DGRAM, &servaddr) != -1);

 if(fd != -1)
 {
     sendto(fd, "hello", strlen("hello"), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
     recvfrom(fd, buf, sizeof(buf), 0, NULL, NULL);
 }

 \endcode
 */

int sock_connect(const char *host, int port, int type, struct sockaddr* servaddr)
{
    logger_t log;
    char portbuf[16];
    struct addrinfo *addr_list, *addr_ptr;
    struct addrinfo hints;
    int fd;
    int res;

    log_init(&log, "sock_connect");

    if(type == SOCK_DGRAM && !servaddr)
    {
        log_error(&log, "SOCK_DGRAM requires servaddr");
        return -1;
    }

    if(servaddr)
        bzero(servaddr, sizeof(struct sockaddr));

    // setup hints to help resolve our hostname
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = type;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    // convert integer port to string for getaddrinfo
    snprintf(portbuf, sizeof(portbuf)-1, "%d", port);

    // obtain a list of address info's
    res = getaddrinfo(host, portbuf, &hints, &addr_list);
    if (res != 0) {
        log_error(&log, "getaddrinfo error");
        return -1;
    }

    // attempt to connect to remote host
    for (addr_ptr = addr_list; addr_ptr != NULL; addr_ptr = addr_ptr->ai_next)
    {
        fd = socket(addr_ptr->ai_family, addr_ptr->ai_socktype, addr_ptr->ai_protocol);
        if(fd < 0)
            continue;

        if(addr_ptr->ai_socktype == SOCK_STREAM)
        {
            if(connect(fd, addr_ptr->ai_addr, addr_ptr->ai_addrlen) >= 0)
                break;
        }
        else if(addr_ptr->ai_socktype == SOCK_DGRAM)
        {
            *servaddr = *addr_ptr->ai_addr;
            break;
        }

        closesocket(fd);
    }

    // we are done with addr_list, we can free it, and check one more time that we got a connection
    freeaddrinfo(addr_list);

    return addr_ptr != NULL ? fd : -1;
}

/**
 * socket server.
 *
 * @param   port is the port to open.
 * @param   type is the type of socket to serve - generally SOCK_STREAM or SOCK_DGRAM.
 * @param   conns is the length of the queue for pending connections
 * @param   servinfo is a pointer to a fresh sock_server_t structure, it will be populated by the function.
 * @retval  returns the socket listener file descriptor, or -1 on error.
 *
 */
int sock_server(int port, int type, int conns, sock_server_t* servinfo, sock_handle_incoming_fptr_t handle_incoming, sock_service_fptr_t service, void* ctx, const char* name, int stacksize, int prio)
{
    struct sockaddr_in servaddr;
    servinfo->service = service;
    servinfo->handle_incoming = handle_incoming;
    servinfo->ctx = ctx;
    servinfo->name = name;
    servinfo->stacksize = stacksize;
    servinfo->prio = prio;

    log_init(&servinfo->log, servinfo->name);

    servinfo->listenfd = socket(AF_INET, type, 0);

    if(servinfo->listenfd != -1)
    {
        log_debug(&servinfo->log, "listening on port %d", port);

        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_port = htons(port);
        bind(servinfo->listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
        listen(servinfo->listenfd, conns);
    }
    else
        log_error(&servinfo->log, "couldnt listen on port %d, error=%d", port, servinfo->listenfd);

    return servinfo->listenfd;
}

void sock_server_kill(sock_server_t* servinfo)
{
    shutdown(servinfo->listenfd, SHUT_RDWR);
    closesocket(servinfo->listenfd);
}

/**
 * this is a thread function - when run, is the the listener.
 */
void sock_server_thread(void* parameters)
{
    sock_server_t* servinfo = (sock_server_t*)parameters;
    bool handled;

    sock_conn_t newconn;
    sock_conn_t* conn;

    newconn.ctx = servinfo->ctx;
    newconn.service = servinfo->service;
    newconn.service = servinfo->service;
    newconn.connfd = 0;

    while(newconn.connfd != -1)
    {
        newconn.clilen = sizeof(newconn.cliaddr);
        newconn.connfd = accept(servinfo->listenfd, (struct sockaddr *)&newconn.cliaddr, &newconn.clilen);

        if(newconn.connfd != -1)
        {
            log_debug(&servinfo->log, "%s accepted conn with %s",
                                    servinfo->name, inet_ntoa(newconn.cliaddr.sin_addr));
            handled = false;
            conn = NULL;

            if(servinfo->handle_incoming)
            {
                conn = malloc(sizeof(sock_conn_t));

                if(conn)
                {
                    memcpy(conn, &newconn, sizeof(sock_conn_t));
                    if(servinfo->handle_incoming(servinfo, conn) == 0)
                    {
                        log_syslog(&servinfo->log, "handled service successfully");
                        handled = true;
                    }
                }
                else
                    log_error(&servinfo->log, "couldnt allocate connection data");
            }
            else
                log_error(&servinfo->log, "service function not set");

            if(!handled)
            {
                log_error(&servinfo->log, "closing unhandled connection");
                closesocket(newconn.connfd);
                if(conn)
                    free(conn);
            }
        }
    }
    log_debug(&servinfo->log, "closing listener");

    vTaskDelete(NULL);
}


/**
 * @}
 */


