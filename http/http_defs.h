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
* @addtogroup http
*
* @{
* @file http_defs.h
*/

#ifndef HTTP_HTTP_DEFS_H_
#define HTTP_HTTP_DEFS_H_

#define HTTP_HOST				"Host: "
#define HTTP_CONTENT_LENGTH		"Content-Length: "
#define HTTP_CONTENT_TYPE		"Content-Type: "

#define HTTP_CONTENT_JSON			"application/json"
#define HTTP_CONTENT_JAVASCRIPT		"application/javascript"
#define HTTP_CONTENT_BINARY			"application/octet-stream"
#define HTTP_CONTENT_TEXT			"text/plain"
#define HTTP_CONTENT_HTML			"text/html"
#define HTTP_CONTENT_NONE			"text/plain"

#define HTTP_MAX_HEADER_LENGTH	512
#define HTTP_GET				"GET"
#define HTTP_POST				"POST"
#define HTTP_EOL				"\r\n"
#define HTTP_EOH				"\r\n\r\n"
#define HTTP_HEADER				"%s %s HTTP/1.0" HTTP_EOL HTTP_HOST "%s" HTTP_EOL HTTP_CONTENT_LENGTH "%d" HTTP_EOL HTTP_CONTENT_TYPE "%s" HTTP_EOH
#define HTTP_SCHEMA				"http://"


#endif /* HTTP_HTTP_DEFS_H_ */

/**
 * @}
 */
