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
#include "cutensils.h"
#include "net.h"

static char* http_split_content(char* response);
static void http_get_resp_codes(char* header, int length, http_response_t* resp);
int pack_header(http_request_t* request);

const char* http_header_strings[] = HTTP_HEADER_CODES;
const char* http_content_strings[] = HTTP_CONTENT_CODES;

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
request.remote = "remote-hostname";
request.port = 80;
request.page = "/";
request.local = "local-hostname";
request.type = HTTP_GET;
request.content_type = HTTP_CONTENT_FIELD_PLAIN;
request.buffer = buffer;
request.content_length = 0;

// configure the response
response.buffer = buffer;
response.size = sizeof(buffer);

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
request.remote = "remote-hostname";
request.port = 80;
request.page = "/api/v1.0";
request.local = "local-hostname";
request.type = HTTP_POST;
request.content_type = HTTP_CONTENT_FIELD_JSON;
request.buffer = buffer;
request.content_length = strlen(buffer);

// configure the response
response.buffer = buffer;
response.size = sizeof(buffer);key_length

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
    http_response_t* resp = NULL;
	logger_t log;
	int fd;
	int chunklen;
    int sent;
	char* header;
	int resplen = 0;
	char* status;
	char* end;

	log_init(&log, "http_request");

	header = malloc(HTTP_MAX_HEADER_LENGTH);

	if(header)
	{
        fd = sock_connect(request->remote, request->port, SOCK_STREAM);

        usleep(10000);

    	if(fd >= 0)
    	{
        	// make HTTP request
        	chunklen = snprintf(header, HTTP_MAX_HEADER_LENGTH - 1, HTTP_HEADER, request->type, request->page, request->local, request->content_length, http_content_strings[request->content_type]);
        	// send HTTP header
        	if(chunklen < (HTTP_MAX_HEADER_LENGTH - 1))
        	{
        	    sent = send(fd, header, chunklen, 0);

        	    if(sent == chunklen)
        	    {
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
                        *end = '\0';
                        response->status = atoi(status);

                        response->message = end + 1;
                        end = strchr(response->message, '\r');
                        *end = '\0';
                        end++;
                        end = http_split_content(end);
                        if(end < (response->buffer + resplen))
                            response->body = end;
                    }

                    resp = response;
        	    }
        	    else
        	        log_error(&log, "send header failed, %d/%dbytes", sent, chunklen);
        	}
        	else
        	    log_error(&log, "header too large, %d/%dbytes", chunklen, HTTP_MAX_HEADER_LENGTH);

    	    closesocket(fd);
        }
        else
            log_error(&log, "failed to connect, %d", fd);

        free(header);
	}
	else
	    log_error(&log, "mem alloc failed");

    return resp;
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

//char* http_get_resp_codes(char* header, char* endofheader, http_response_t* resp)
//{
//    char* end;
//    int field;
//    char* start = strchr(header, HTTP_SPACE_CHAR);
//
//    if(!start)
//        return NULL;
//
//    start++;
//    end = strchr(start,  HTTP_SPACE_CHAR);
//    if(!end || end > endofheader)
//        return NULL;
//    end = '\0';
//    resp->status = atoi(start);
//    resp->message = end + 1;
//    end = strchr(resp->message, HTTP_CR_CHAR);
//    if(!end || end > endofheader)
//        return NULL;
//    end = '\0';
//
//    while(1)
//    {
//        start = end + 2;
//        end = strchr(start, HTTP_COLON_CHAR);
//
//        if(!end || end > endofheader)
//            return NULL;
//
//        *end = '\0';
//        field = string_in_list(start, strlen(start), http_header_strings);
//
//        start = end + 2;
//        end = strchr(start,  HTTP_CR_CHAR);
//
//        if(!end || end > endofheader)
//            return NULL;
//
//        if((end - start) < 1)
//            break;
//
//        *end = '\0';
//        switch(field)
//        {
//            case HTTP_HEADER_FIELD_HOST:
//                resp->host = start;
//            break;
//            case HTTP_HEADER_FIELD_CONTENT_LENGTH:
//                resp->content_length = atoi(start);
//            break;
//            case HTTP_HEADER_FIELD_CONTENT_TYPE:
//                resp->content_type = string_in_list(start, strlen(start), http_content_strings);
//            break;
//        }
//    }
//
//    resp->body = end + 2;
//    return resp->body;
//}
//
//int pack_header(http_request_t* request)
//{
//    return snprintf(request->buffer, HTTP_MAX_HEADER_LENGTH - 1, HTTP_HEADER,
//            request->type, request->page, request->local,
//            request->content_length, http_content_strings[request->content_type]);
//}
//
///**
// * perform HTTP get, saving the response to a file.
// *
// * response must be initialized:
// * response.buffer = malloc(N);
// * response.size = N;
// * response.fdes = open("outputfile", O_WRONLY | O_TRUNC | O_CREAT);
// */
//http_response_t* http_get_file(const char* host, unsigned short port, const char* page, http_response_t* response)
//{
//    http_response_t* resp = NULL;
//    logger_t log;
//    int fd;
//    int length;
//    int sent;
//    char* header;
//    int resplen = 0;
//    char* status;
//    char* end;
//
//    http_request_t request = {
//        .remote = host,
//        .port = port,
//        .page = page,
//        .local = net_lip(),
//        .type = HTTP_GET,
//        .content_type = HTTP_CONTENT_NONE,
//        .buffer = response->buffer,
//        .size = response->size,
//        .content_length = 0
//    };
//
//    log_init(&log, "http_get_file");
//
//    if(!request.buffer)
//    {
//        log_error(&log, "mem alloc failed");
//        return NULL;
//    }
//
//    fd = sock_connect(request->remote, request->port, SOCK_STREAM);
//
//    if(fd == -1)
//    {
//        log_error(&log, "failed to connect, %d", fd);
//        return NULL;
//    }
//
//    // make HTTP request
//    length = pack_header(&request);
//
//    // send HTTP header
//    if(length >= request.size)
//    {
//        closesocket(fd);
//        log_error(&log, "header too large, %d/%dbytes", chunklen, HTTP_MAX_HEADER_LENGTH);
//        return NULL;
//    }
//
//    sent = send(fd, request.buffer, length, 0);
//
//    if(sent != length)
//    {
//        closesocket(fd);
//        log_error(&log, "send header failed, %d/%dbytes", sent, chunklen);
//        return NULL;
//    }
//
//    // receive header and possibly part of body
//    length = recv(fd, request.buffer, HTTP_MAX_HEADER_LENGTH, 0);
//
//    http_get_resp_codes(request.buffer, request.buffer + length, response);
//
//    length = (request.buffer + length) - response->body;
//
//    if(length > 0 && response->content_length > 0)
//        write(response->fdes, response->body, length);
//
//    closesocket(fd);
//
//    return resp;
//}


/**
 * @}
 */
