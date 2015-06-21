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
* @addtogroup http
*
* @{
* @file http_server.h
*/

#ifndef HTTP_HTTP_SERVER_H_
#define HTTP_HTTP_SERVER_H_

#include "threaded_server.h"
#include "http_defs.h"
#include "http_api.h"

#define DEFAULT_HTTPSERVER_CONF_PATH		"/etc/http/httpd_config"
#define DEFAULT_HTTPD_FS_ROOT				"/var/lib/httpd"
#define HTTP_FS_ROOT_CONFIG_KEY				"fsroot"

#define HTTP_FS_ROOT_LENGTH         32
#define HTTP_URL_LEN                64
#define HTTP_SCRATCH_LEN            256

#define HTTP_SERVER_STACK_SIZE      325
#define HTTP_SERVER_TASK_PRIO       1

typedef struct {
	char fsroot[HTTP_FS_ROOT_LENGTH];
	sock_server_t server;
	logger_t log;
	const http_api_t** api;
}httpserver_t;


int init_http_server(httpserver_t* httpserver, char* configfile, const http_api_t** api);


#endif /* HTTP_HTTP_SERVER_H_ */

/**
 * @}
 */
