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
 * This file is part of the stm32-device-support project, <https://github.com/drmetal/stm32-device-support>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

#ifndef ETH_MAC_H_
#define ETH_MAC_H_

#if USE_DRIVER_MII_RMII_PHY
#include "stm32_eth.h" // includes MII_RMII.h for us
#elif USE_DRIVER_ENC28J60_PHY
#include "enc28j60.h"
#endif

// TODO - clean up the following when adding UIP support
///**
// *     ENC28J60 MAC/PHY with UIP
// */
//#if USE_DRIVER_MII_RMII_PHY && USE_DRIVER_UIP_NET
///**
// * ethernet device init.
// */
//static inline void devicedriver_init(void* conf)
//{
//    ETH_Configuration((const uint8_t*)macaddr);
//}
//
///**
// * ethernet device send.
// */
//static inline bool devicedriver_send()
//{
//     return (bool)ETH_HandleTxPkt(uip_buf, uip_len);
//}
//
///**
// * ethernet device read.
// */
//static inline uint32_t devicedriver_read()
//{
//     return ETH_HandleRxPkt(uip_buf);
//}
//
///**
// *     STM32 MAC, MII/RMII PHY with LWIP
// */
//#elif USE_DRIVER_ENC28J60_PHY && USE_DRIVER_UIP_NET
//
///**
// * ethernet device init.
// */
//static inline void devicedriver_init(void* conf)
//{
//    enc28j60_init((uint8_t*)conf);
//}
//
///**
// * ethernet device send.
// */
//static inline bool devicedriver_send()
//{
//    enc28j60_send_packet((uint8_t *) uip_buf, uip_len);
//    return true;
//}
//
///**
// * ethernet device read.
// */
//static inline uint32_t devicedriver_read()
//{
//    return (uint32_t)enc28j60_recv_packet((uint8_t *) uip_buf, UIP_BUFSIZE);
//}
//
//#else
//#error "no phy specified - do you need to include eth.h?"
//#endif

#endif // ETH_MAC_H_
