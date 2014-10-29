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
* @defgroup http HTTP Utilities
*
* @{
* @file http_client.c
*/

#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/socket.h>
#include "http_client.h"
#include "sock_utils.h"
#include "logger.h"


static char* http_split_content(char *response);

/**
 * sends a HTTP request.
 *
 * @param	request - pointer to a request data structure.
 * @param	response - pointer to a response data structure.
 * @retval  returns the response. if sending the request failed or no
 * 			response was read, returns NULL.
 *
\code

// example HTTP GET

// get some buffer memory
char buffer[512];

// configure the request
regclient->request.remote = "remote-hostname";
regclient->request.port = 80;
regclient->request.page = "/";
regclient->request.local = "local-hostname";
regclient->request.type = HTTP_GET;
regclient->request.content_type = HTTP_CONTENT_NONE;
regclient->request.buffer = buffer;
regclient->request.content_length = 0;

// configure the response
regclient->response.buffer = buffer;
regclient->response.size = sizeof(buffer);

if(http_request(&request, &response))
{
	printf("response: %d", response->status);
	printf("response: %s", response->message);
	printf("response: %s", response->body);
}

// example HTTP POST
char resp[512] = "{\"some-json-data\":\"some-value\"}";
char* body;
if(http_request(host, port, page, HTTP_POST, HTTP_CONTENT_JSON, resp, sizeof(resp)))
{
	printf("response: %s", resp);
    body = http_split_content(resp);
	printf("body: %s", body);
}

// example HTTP POST

// init post data buffer
char buffer[512] = "{\"some-json-data\":\"some-value\"}";

// configure the request
regclient->request.remote = "remote-hostname";
regclient->request.port = 80;
regclient->request.page = "/api/v1.0";
regclient->request.local = "local-hostname";
regclient->request.type = HTTP_POST;
regclient->request.content_type = HTTP_CONTENT_JSON;
regclient->request.buffer = buffer;
regclient->request.content_length = strlen(buffer);

// configure the response
regclient->response.buffer = buffer;
regclient->response.size = sizeof(buffer);

if(http_request(&request, &response))
{
	printf("response: %d", response->status);
	printf("response: %s", response->message);
	printf("response: %s", response->body);
}

\endcode
 *
 */
http_response_t* http_request(http_request_t* request, http_response_t* response)
{
	logger_t log;
	int fd;
	int chunklen;
	char header[HTTP_MAX_HEADER_LENGTH];
	int resplen = 0;
	char* status;
	char* end;

	log_init(&log, "http_request");

	fd = sock_connect(request->remote, request->port, SOCK_STREAM);

	if (fd < 0)
	{
		log_error(&log, "failed to connect");
		return NULL;
	}

	// make HTTP request
	chunklen = snprintf(header, sizeof(header)-1, HTTP_HEADER, request->type, request->page, request->local, request->content_length, request->content_type);
	if(chunklen >= (int)sizeof(header)-1)
	{
		log_error(&log, "header too large");
		closesocket(fd);
		return NULL;
	}

	// send HTTP header
	if(send(fd, header, chunklen, 0) < 0)
	{
		log_error(&log, "failed to connect");
		closesocket(fd);
		return NULL;
	}

	// send body
	if(request->content_length > 0)
		send(fd, request->buffer, request->content_length, 0);

	// receive the header and body
	// TODO - receive header only, decode nice info like content length, then receive body...
	while(resplen < response->size-1)
	{
		chunklen = recv(fd, response->buffer + resplen, response->size - resplen, 0);
		if(chunklen <= 0)
			break;
		resplen += chunklen;
	}

	// null terminate response
	response->buffer[resplen] = '\0';

	response->status = 0;
	response->body = NULL;
	response->message = NULL;

	// determine status code, message and body
	status = strchr(response->buffer, ' ');
	if(status)
	{
		status++;
		end = strchr(status,  ' ');
		end = '\0';
		response->status = atoi(status);

		response->message = end + 1;
		end = strchr(response->message, '\r');
		end = '\0';

		response->body = http_split_content(response->buffer);
	}

	closesocket(fd);

    return response;
}

/**
 * split http content from the response buffer.
 */
char* http_split_content(char* response) {
	while(*response)
	{
		if(strncmp(HTTP_EOH, response, sizeof(HTTP_EOH)-1) == 0)
		{
			*response = '\0';
			return response + sizeof(HTTP_EOH)-1;
		}
		response++;
	}
	return NULL;
}

/**
 * @}
 */
