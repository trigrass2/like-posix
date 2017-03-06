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

#ifndef ONEWIRE_H_
#define ONEWIRE_H_

#include "board_config.h"
#include "usart.h"

#define ONEWIRE_POLLED 1 // use polled usart, instead of usart dev

// ROM COMMANDS
#define ONEWIRE_MATCH_ROM           0x55
#define ONEWIRE_SEARCH_ROM          0xF0
#define ONEWIRE_SKIP_ROM            0xCC
#define ONEWIRE_READ_ROM            0x33
#define ONEWIRE_ALARM_SEARCH        0xEC


#define onewire_read_byte(fd) onewire_xfer_byte(fd, 0xff);
#define onewire_write_byte(fd, byte) onewire_xfer_byte(fd, byte);
#define onewire_read_bit(fd) onewire_xfer_bit(fd, 0xff);
#define onewire_write_bit(fd, bit) onewire_xfer_bit(fd, bit);

#if ONEWIRE_POLLED
USART_HANDLE_t onewire_create(USART_TypeDef* usart);
#else
USART_HANDLE_t onewire_create(USART_TypeDef* usart, char* filename);
int onewire_open(const char* filename);
void onewire_close(int fd);
#endif
unsigned char onewire_reset(int fd);
unsigned char onewire_xfer_byte(int fd, unsigned char byte);
unsigned char onewire_xfer_bit(int fd, unsigned char bit);
void onewire_address_command(int fd, uint64_t devcode);
void onewire_search_ids(int fd, uint64_t* buffer, int length);

#endif // ONEWIRE_H_
