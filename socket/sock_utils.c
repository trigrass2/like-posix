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

#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include "sock_utils.h"

/**
 * open a socket.
 *
 * @param	host may be an IP address or hostname.
 * @param	port is the TCP port to open.
 * @retval	returns the socket file descriptor, or -1 on error.
 */
int sock_connect(char *host, int port)
{
	char portbuf[16];
	struct addrinfo *addr_list, *addr_ptr;
	struct addrinfo hints;
	int fd;
	int res;

	// setup hints to help resolve our hostname
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;

	// convert integer port to string for getaddrinfo
	snprintf(portbuf, sizeof(portbuf)-1, "%d", port);

	// obtain a list of address info's
	res = getaddrinfo(host, portbuf, &hints, &addr_list);
	if (res != 0) {
		printf("getaddrinfo error");
		return -1;
	}

	// attempt to connect to remote host
	for (addr_ptr = addr_list; addr_ptr != NULL; addr_ptr = addr_ptr->ai_next)
	{
		fd = socket(addr_ptr->ai_family, addr_ptr->ai_socktype, addr_ptr->ai_protocol);
		if(fd < 0)
			continue;

		if(connect(fd, addr_ptr->ai_addr, addr_ptr->ai_addrlen) >= 0)
			break;

		closesocket(fd);
	}

	// we are done with addr_list, we can free it, and check one more time that we got a connection
	freeaddrinfo(addr_list);

	return addr_ptr != NULL ? fd : -1;
}

/**
 * @}
 */


