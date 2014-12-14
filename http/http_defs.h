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

enum {
    HTTP_HEADER_FIELD_HOST = 0,
    HTTP_HEADER_FIELD_CONTENT_LENGTH = 1,
    HTTP_HEADER_FIELD_CONTENT_TYPE = 2,
};

#define HTTP_HOST				"Host: "
#define HTTP_CONTENT_LENGTH		"Content-Length: "
#define HTTP_CONTENT_TYPE		"Content-Type: "

#define HTTP_HEADER_CODES       {HTTP_HOST, HTTP_CONTENT_LENGTH, HTTP_CONTENT_TYPE, NULL}

enum {
    HTTP_CONTENT_FIELD_JSON = 0,
    HTTP_CONTENT_FIELD_JAVASCRIPT = 1,
    HTTP_CONTENT_FIELD_BINARY = 2,
    HTTP_CONTENT_FIELD_TEXT = 3,
    HTTP_CONTENT_FIELD_HTML = 4,
    HTTP_CONTENT_FIELD_PLAIN = 5,
};

#define HTTP_CONTENT_JSON			"application/json"
#define HTTP_CONTENT_JAVASCRIPT		"application/javascript"
#define HTTP_CONTENT_BINARY			"application/octet-stream"
#define HTTP_CONTENT_TEXT			"text/plain"
#define HTTP_CONTENT_HTML			"text/html"
#define HTTP_CONTENT_NONE			"text/plain"

#define HTTP_CONTENT_CODES       {HTTP_CONTENT_JSON, HTTP_CONTENT_JAVASCRIPT, HTTP_CONTENT_BINARY, HTTP_CONTENT_TEXT, HTTP_CONTENT_HTML, HTTP_CONTENT_NONE, NULL}

#define HTTP_GET				"GET"
#define HTTP_POST				"POST"
#define HTTP_VERS				"HTTP/1.0"
#define HTTP_EOL				"\r\n"
#define HTTP_EOH				HTTP_EOL HTTP_EOL
#define HTTP_HEADER				"%s %s " HTTP_VERS HTTP_EOL HTTP_HOST "%s" HTTP_EOL HTTP_CONTENT_LENGTH "%d" HTTP_EOL HTTP_CONTENT_TYPE "%s" HTTP_EOH
#define HTTP_SCHEMA				"http://"

#define HTTP_REQ_TYPE_NONE  0
#define HTTP_REQ_TYPE_GET   'G'
#define HTTP_REQ_TYPE_POST  'P'

#define HTTP_SLASH_CHAR 	'/'
#define HTTP_EOL_CHAR 	'\n'
#define HTTP_SPACE_CHAR 	' '
#define HTTP_DOT_CHAR 	'.'
#define HTTP_CR_CHAR    '\r'
#define HTTP_COLON_CHAR    ':'

#define HTTP_INDEX_STR "/index.html"
#define HTTP_FILE_NOT_FOUND_STR "/404.html"
#define HTTPD_TITLE_DOCUMENT "/title.html"

#define http_get_min  HTTP_GET " / HTTP/1.1" HTTP_EOL

#define http_html  ".html"
#define http_shtml  ".shtml"
#define http_htm  ".htm"
#define http_css  ".css"
#define http_png  ".png"
#define http_gif  ".gif"
#define http_jpg  ".jpg"
#define http_text  ".txt"
#define http_txt  ".txt"
#define http_cgi  ".cgi"
#define http_json  ".json"
#define http_xml  ".xml"

#define http_header1  HTTP_VERS " "
#define http_header2  HTTP_EOL "Server: nutensils/FreeRTOS" HTTP_EOL "Connection: close" HTTP_EOL HTTP_CONTENT_TYPE

#define http_200_header_title  "200 OK"
#define http_404_header_title  "404 Not found"
#define http_408_header_title  "408 Request Timeout"
#define http_500_header_title  "500 Internal Server Error"
#define http_501_header_title  "501 Not Implemented"

#define http_header_content_type_plain  "text/plain"
#define http_header_content_type_html  "text/html"
#define http_header_content_type_css   "text/css"
#define http_header_content_type_text  "text/text"
#define http_header_content_type_png   "image/png"
#define http_header_content_type_gif   "image/gif"
#define http_header_content_type_jpg   "image/jpeg"
#define http_header_content_type_binary  "application/octet-stream"
#define http_header_content_type_xml  "application/xml"
#define http_header_content_type_json  "application/json"

#define text_page_header \
"<!DOCTYPE html>\
<html>\
<head>\
<link href=\"http://fonts.googleapis.com/css?family=Istok+Web\" rel=\"stylesheet\" type=\"text/css\">\
<style type=\"text/css\">\
h1 {\
font: 200 36px/1.2 'Istok Web', Verdana, Helvetica, sans-serif;\
color:dimgray;\
text-align:left;\
margin-right:50px;\
}\
</style>\
</head>\
<body>\
<h1>"

#define text_page_footer \
"</h1>\
</body>\
</html>"

#endif /* HTTP_HTTP_DEFS_H_ */

/**
 * @}
 */
