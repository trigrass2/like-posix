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

#ifndef NET_CONF_H_
#define NET_CONF_H_

#define NET_TASK_PRIORITY       2
#define NET_TASK_STACK          64

#define NET_LINK_LED			LED2

#define ENC28J60_PHY
//#define DP83848_PHY
//#define LAN8700_PHY
//#define BCM5241_PHY

/* Definition of the Ethernet driver buffers size and count */
#define ETH_MAX_ETH_PAYLOAD     	   600
#define ETH_RX_BUF_SIZE                ETH_MAX_PACKET_SIZE /* buffer size for receive               */
#define ETH_TX_BUF_SIZE                ETH_MAX_PACKET_SIZE /* buffer size for transmit              */
#define ETH_RXBUFNB                    ((uint32_t)4)       /* 4 Rx buffers of size ETH_RX_BUF_SIZE  */
#define ETH_TXBUFNB                    ((uint32_t)4)       /* 4 Tx buffers of size ETH_TX_BUF_SIZE  */


#define ENC28J60_SPI_PERIPH     		SPI3
#define ENC28J60_SPI_NRST_PORT   		GPIOB
#define ENC28J60_SPI_NRST_PIN    		GPIO_PIN_7
#define ENC28J60_SPI_NINT_PORT   		GPIOB
#define ENC28J60_SPI_NINT_PIN    		GPIO_PIN_6
#define ENC28J60_SPI_BAUDRATE           10000000
#define ENC28J60_RESET_TIME_US 			50000

#endif // NET_CONF_H_
