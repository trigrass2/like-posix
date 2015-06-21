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
* @file http_client.h
*/

#ifndef HTTP_HTTP_CLIENT_H_
#define HTTP_HTTP_CLIENT_H_

#include "http_defs.h"

#define HTTP_MAX_HEADER_LENGTH    256

typedef struct {
    const char* remote;         ///< set the remote IP address or hostname
    int port;                   ///< set the remote port
    const char *page;           ///< set the page to request
    const char* local;          ///< set the local IP address or hostname
    const char* type;           ///< set the request type - GET, POST etc
    int   content_type;         ///< set content type, eg text/plain
    char* buffer;               ///< set the buffer that holds the request body data
    int content_length;         ///< set the length of the request body data
    int size;                   ///< set to the the size of the buffer in bytes.
}http_request_t;

// TODO - do a nice HTTP response parser.
typedef struct {
    int status;                 ///< not set by the user - holds the HTTP status code, of the response
    const char* message;        ///< not set by the user - points to the HTTP status message, in buffer
    const char* body;           ///< not set by the user - points to the HTTP response body, in buffer
    const char* host;           ///< not set by the user - holds the "Host" header field, of the response
    const char* server;           ///< not set by the user - holds the "Server" header field, of the response
    int content_type;           ///< not set by the user - holds the "Content-Type" header field, of the response
    int content_length;         ///< not set by the user - holds the "Content-Length" header field, of the response
    char* buffer;               ///< set the buffer that will hold the response body data
    int size;                   ///< set to the the size of the buffer in bytes.
}http_response_t;

http_response_t* http_request(http_request_t* request, http_response_t* response);
http_response_t* http_get_file(char* url, http_response_t* response, const char* output, char* buffer, int size);

#endif /* HTTP_HTTP_CLIENT_H_ */

/**
 * @}
 */
