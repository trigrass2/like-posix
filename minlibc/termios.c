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
 * @addtogroup syscalls System Calls
 *
 *
 * @file termios.c
 * @{
 */

#include "termios.h"

speed_t cfgetispeed(const struct termios* termios)
{
    return termios->c_ispeed;
}

speed_t cfgetospeed(const struct termios* termios)
{
    return termios->c_ospeed;
}

int cfsetispeed(struct termios* termios, speed_t ispeed)
{
    termios->c_ispeed = ispeed;
    return 0;
}

int cfsetospeed(struct termios* termios, speed_t ospeed)
{
    termios->c_ospeed = ospeed;
    return 0;
}

int tcgetattr(int fildes, struct termios *termios_p)
{
	return _tcgetattr(fildes, termios_p);
}

int tcsetattr(int fildes, int when, const struct termios *termios_p)
{
	return _tcsetattr(fildes, when, termios_p);
}

int tcdrain(int file)
{
    return _tcdrain(file);
}

int tcflush(int file, int flags)
{
    return _tcflush(file, flags);
}

int tcflow(int file, int flags)
{
    return _tcflow(file, flags);
}

/**
 * @}
 */
