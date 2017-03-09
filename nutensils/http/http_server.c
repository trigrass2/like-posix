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
* @addtogroup http
*
* @{
* @file http_server.c
*/

#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include "sock_utils.h"
#include "logger.h"
#include "http_server.h"
#include "http_api.h"


typedef struct {
	int length;
	const http_api_t* api_call;
	const char* fsroot;
	const char* req_type;
	const char* header;
	const char* content_type;
	int content_length;
	char scratch[HTTP_SCRATCH_LEN];
	char url[HTTP_URL_LEN];
	FILE* file;
	struct stat stat;
}http_server_conn_t;

static void http_server_connection(sock_conn_t* conn);
static void message_response(int fdes, const char* message);

/**
 * compare string to a constant string.
 */
#define compare_string(str, const_comp) (strncmp(const_comp, str, (int)sizeof(const_comp)-1) == 0)

/**
 * @brief   A simple HTTP server with support for GET and POST requests.
 */
int init_http_server(httpserver_t* httpserver, char* configfile, const http_api_t** api)
{
	char buffer[sizeof(httpserver->fsroot) + sizeof(HTTP_FS_ROOT_CONFIG_KEY) + 2];
	const char* fsroot = (const char*)get_config_value_by_key((uint8_t*)buffer, sizeof(buffer), (const uint8_t*)configfile, (const uint8_t*)HTTP_FS_ROOT_CONFIG_KEY);

	httpserver->api = api;

	log_init(&httpserver->log, "http_server");

	if(fsroot == NULL)
		strncpy(httpserver->fsroot, DEFAULT_HTTPD_FS_ROOT, sizeof(httpserver->fsroot)-1);
	else
		strncpy(httpserver->fsroot, fsroot, sizeof(httpserver->fsroot)-1);

	log_debug(&httpserver->log, "fsroot: %s", httpserver->fsroot);

	return start_threaded_server(&httpserver->server, configfile, http_server_connection, httpserver, HTTP_SERVER_STACK_SIZE, HTTP_SERVER_TASK_PRIO);
}

/**
 * @brief   sends a complete HTTP response containing a message.
 * @param   message is a pointer to a string containing the message to sent.
 */
void message_response(int fdes, const char* message)
{
	send(fdes, text_page_header, sizeof(text_page_header)-1, 0);
	send(fdes, message, strlen(message), 0);
	send(fdes, text_page_footer, sizeof(text_page_footer)-1, 0);
}

/**
 * @brief   the HTTP server thread.
 * Processes requests on a per connection, per request basis.
 */
void http_server_connection(sock_conn_t* conn)
{
	httpserver_t* httpserver = (httpserver_t*)conn->ctx;
	http_server_conn_t* httpconn = malloc(sizeof(http_server_conn_t));

	if(!httpconn)
	{
		log_error(&httpserver->log, "failed to allocate memory");
		return;
	}

	struct timeval tv;
	tv.tv_sec = 2000; // take care, lwip sets s as ms
	tv.tv_usec = 0;
	setsockopt(conn->connfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval));

	httpconn->req_type = NULL;
	httpconn->content_length = 0;
	httpconn->content_type = NULL;
	httpconn->api_call = NULL;
	httpconn->url[0] = '\0';
	httpconn->file = NULL;

	//*********************************
	//  receive request
	//********************************
	/**
	 * After receiving a whole request header:
	 *  - if a whole HTTP POST or GET is received, url contains the received url.
     *  - if a whole HTTP POST is received, content_length holds the length of the outstanding message data.
	 */
	while(1)
	{
		httpconn->length = 0;
		httpconn->scratch[0] = '\0';
        // receive up to the HTTP_EOL
        do
        {
        	if(recv(conn->connfd, &httpconn->scratch[httpconn->length], 1, 0) < 1)
        	{
        	    free(httpconn);
        	    log_error(&httpserver->log, "aborting");
        	    return;
        	}
            httpconn->length++;
        }
        while(httpconn->scratch[httpconn->length-1] != HTTP_EOL_CHAR);

        httpconn->scratch[httpconn->length] = '\0';

        // check for end of header
        if(!strcmp(httpconn->scratch, HTTP_EOL))
            break;

		// find the GET or POST line
		if(!httpconn->req_type)
		{
			// look for the "POST " in the header line
			if(compare_string(httpconn->scratch, HTTP_POST))
				httpconn->req_type = HTTP_POST;
			// look for the "GET " in the header line
			else if(compare_string(httpconn->scratch, HTTP_GET))
				httpconn->req_type = HTTP_GET;
			else
				break;

			if(httpconn->req_type)
			{
				// extract url part, occurs after the "GET " part of the header line
				// find the first space in the line AFTER the URL part of the line
				char* urlstart = httpconn->scratch + strlen(httpconn->req_type) + 1;
				char* urlend = strchr(urlstart, HTTP_SPACE_CHAR);
				if(urlend)
				{
					// change that space to a 0
					*urlend = '\0';
					strcpy(httpconn->url, urlstart);
				}
			}
		}
		// find content length if it is included in header
		else if(compare_string(httpconn->scratch, HTTP_CONTENT_LENGTH))
		{
			char* clstart = httpconn->scratch + (sizeof(HTTP_CONTENT_LENGTH)-1);
			char* clend = strchr(clstart, HTTP_CR_CHAR);
			if(clend)
			{
				*clend = '\0';
				httpconn->content_length = atoi(clstart);
			}
		}
	}

    log_debug(&httpserver->log, "url %s", httpconn->url);

	// test for RPC
	httpconn->api_call = http_api_check(httpserver->api, httpconn->url);
	// set default response
	httpconn->header = http_500_header_title;
	httpconn->content_type = http_header_content_type_html;

	//*********************************
	//*  determine response type
	//*********************************
	if(!httpconn->req_type)
	{
		log_error(&httpserver->log, http_501_header_title);
		httpconn->header = http_501_header_title;
		httpconn->content_type = http_header_content_type_html;
	}
	// POST or GET response
	else if((httpconn->req_type == (char*)HTTP_POST) || (httpconn->req_type == (char*)HTTP_GET))
	{
	    log_debug(&httpserver->log, "processing");
		// change default response
		httpconn->header = http_404_header_title;
		httpconn->content_type = http_header_content_type_html;

		// no file io needed for RPC
		if(httpconn->api_call)
		{
			httpconn->header = http_202_header_title;
			httpconn->content_type = http_header_content_type_json;
		}
		else
		{
			strcpy(httpconn->scratch, httpserver->fsroot);

			// if the URL is just a "/" then change it to "/index.html"
			if(httpconn->url[0] == HTTP_SLASH_CHAR && httpconn->url[1] == '\0')
				strcat(httpconn->scratch, HTTP_INDEX_STR);
			// prepend HTTPD_FS_ROOT path
			else
				strcat(httpconn->scratch, httpconn->url);

			log_debug(&httpserver->log, "path: %s", httpconn->scratch);

			httpconn->file = fopen(httpconn->scratch, httpconn->req_type == (char*)HTTP_POST ? "w" : "r");

			if(httpconn->file)
			{
			    if(httpconn->req_type == (char*)HTTP_POST)
			        httpconn->header = http_201_header_title;
			    else
			        httpconn->header = http_200_header_title;

				// find the file extension
				httpconn->content_type = strrchr(httpconn->scratch, HTTP_DOT_CHAR);
				if(!httpconn->content_type)
					httpconn->content_type = http_header_content_type_binary;
				else if(!strncmp(http_html, httpconn->content_type, sizeof(http_html)) ||
						!strncmp(http_shtml, httpconn->content_type, sizeof(http_shtml)))
					httpconn->content_type = http_header_content_type_html;
				else if(!strncmp(http_css, httpconn->content_type, sizeof(http_css)))
					httpconn->content_type = http_header_content_type_css;
				else if(!strncmp(http_png, httpconn->content_type, sizeof(http_png)))
					httpconn->content_type = http_header_content_type_png;
				else if(!strncmp(http_gif, httpconn->content_type, sizeof(http_gif)))
					httpconn->content_type = http_header_content_type_gif;
				else if(!strncmp(http_jpg, httpconn->content_type, sizeof(http_jpg)))
					httpconn->content_type = http_header_content_type_jpg;
				else if(!strncmp(http_json, httpconn->content_type, sizeof(http_json)))
					httpconn->content_type = http_header_content_type_json;
				else if(!strncmp(http_xml, httpconn->content_type, sizeof(http_xml)))
					httpconn->content_type = http_header_content_type_xml;
				else
					httpconn->content_type = http_header_content_type_plain;
			}
			else if(httpconn->req_type == (char*)HTTP_POST)
			{
		        httpconn->header = http_500_header_title;
			}
		}
	}

	//*********************************
	//  send response header
	//*********************************
	send(conn->connfd, http_header1, sizeof(http_header1)-1, 0);
	send(conn->connfd, httpconn->header, strlen(httpconn->header), 0);
	send(conn->connfd, http_header2, sizeof(http_header2)-1, 0);
	send(conn->connfd, httpconn->content_type, strlen(httpconn->content_type), 0);
	send(conn->connfd, HTTP_EOH, sizeof(HTTP_EOH)-1, 0);

    log_debug(&httpserver->log, "sent header");
    log_debug(&httpserver->log, "responding to %s request", httpconn->req_type);


	//*********************************
	//  send response body
	//*********************************

	// serve error message
	if(httpconn->header != (char*)http_200_header_title &&
	   httpconn->header != (char*)http_201_header_title &&
       httpconn->header != (char*)http_202_header_title)
	{
		log_error(&httpserver->log, (char*)httpconn->header);
		snprintf(httpconn->scratch, sizeof(httpconn->scratch)-1, "oops...<br>%s: %s", httpconn->header, httpconn->url);
		message_response(conn->connfd, httpconn->scratch);
	}
	// POST or GET, RPC response
	else if(httpconn->api_call)
	{
        log_debug(&httpserver->log, "process API call");
		http_api_process(httpconn->api_call, conn->connfd, httpconn->content_length, httpconn->scratch, sizeof(httpconn->scratch));
	}
	// POST file response
	else if(httpconn->file && httpconn->req_type == (char*)HTTP_POST)
	{
		log_debug(&httpserver->log, "write %s %ub", httpconn->scratch, httpconn->content_length);
		while(httpconn->content_length)
		{
			httpconn->length = recv(conn->connfd, httpconn->scratch, sizeof(httpconn->scratch), 0);
			if(httpconn->length)
			{
				fwrite(httpconn->scratch, 1, httpconn->length, httpconn->file);
				httpconn->content_length -= httpconn->length;
			}
		}
	}
	// GET file response
	else if(httpconn->file && httpconn->req_type == (char*)HTTP_GET)
	{
		log_debug(&httpserver->log, "read %s", httpconn->scratch);
		httpconn->length = sizeof(httpconn->scratch);
		while(httpconn->length > 0)
		{
			httpconn->length = fread(httpconn->scratch, 1, sizeof(httpconn->scratch), httpconn->file);
			if(httpconn->length > 0)
				send(conn->connfd, httpconn->scratch, httpconn->length, 0);
		}
	}

	if(httpconn->file)
	{
		fclose(httpconn->file);
	}

    log_debug(&httpserver->log, "done");

	free(httpconn);
}

/**
 * @}
 */
