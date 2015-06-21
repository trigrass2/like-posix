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

#ifndef SHELL_DEFS_H_
#define SHELL_DEFS_H_

/**
 * SHELL_NEWLINE_PROMPT is the string that is printed a prompt.
 */
#define SHELL_DRIVE             "0:"
#define SHELL_ROOT              "/"
#define SHELL_PROMPT            "# "
#define SHELL_ROOT_PROMPT       SHELL_DRIVE SHELL_ROOT SHELL_PROMPT

#define SHELL_UPARROW			"\x1B\x5B\x41"
#define SHELL_DOWNARROW			"\x1B\x5B\x42"
#define SHELL_LEFTARROW			"\x1B\x5B\x44"
#define SHELL_RIGHTARROW		"\x1B\x5B\x43"
#define SHELL_PREVIOUS_LINE		"\x1B[1F"
#define ANSII_CLEAR_LINE        "\x1B[2K"
#define ANSII_CLEAR_LINE_FROM   "\x1B[0K"
#define ANSII_CLEAR_LINE_TO     "\x1B[1K"

#define SHELL_NO_SUCH_COMMAND       SHELL_NEWLINE "no such command "
#define SHELL_HELP_STR              "the following commands exist:"
#define SHELL_NO_HELP_STR           "no help for command "
#define SHELL_USAGE_STR             " usage"SHELL_NEWLINE

/**
 * SHELL_NEWLINE is the type of newline to use throughout the shell system.
 */
#define SHELL_NEWLINE           "\r\n"

#endif /* SHELL_DEFS_H_ */
