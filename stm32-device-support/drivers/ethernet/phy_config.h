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
 * This file is part of the lollyjar project, <https://github.com/drmetal/lollyjar>
 *
 * This code is based heavily on demo code from microchip...
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

#include "net_config.h"

/* PHY Reset delay these values are based on a 1 ms Systick interrupt*/
#define PHY_RESET_DELAY                 ((uint32_t)0x000000FF)
#define PHY_CONFIG_DELAY                ((uint32_t)0x00000FFF)
#define PHY_READ_TO                     ((uint32_t)0x0000FFFF)
#define PHY_WRITE_TO                    ((uint32_t)0x0000FFFF)


#if defined(LAN8700_PHY)

#define PHY_ADDRESS       0x01

/** PHY register set */
#define PHY_BCR                          0          /*!< Tranceiver Basic Control Register */
#define PHY_BSR                          1          /*!< Tranceiver Basic Status Register */
#define PHY_SR                           31         /*!< Tranceiver Status Register */

/** PHY_BCR bit definitions */
#define PHY_RESET                       ((uint16_t)0x8000)      /*!< PHY Reset */
#define PHY_LOOPBACK                    ((uint16_t)0x4000)      /*!< Select loop-back mode */
#define PHY_FULLDUPLEX_100M             ((uint16_t)0x2100)      /*!< Set the full-duplex mode at 100 Mb/s */
#define PHY_HALFDUPLEX_100M             ((uint16_t)0x2000)      /*!< Set the half-duplex mode at 100 Mb/s */
#define PHY_FULLDUPLEX_10M              ((uint16_t)0x0100)      /*!< Set the full-duplex mode at 10 Mb/s */
#define PHY_HALFDUPLEX_10M              ((uint16_t)0x0000)      /*!< Set the half-duplex mode at 10 Mb/s */
#define PHY_AUTONEGOTIATION             ((uint16_t)0x1000)      /*!< Enable auto-negotiation function */
#define PHY_RESTART_AUTONEGOTIATION     ((uint16_t)0x0200)      /*!< Restart auto-negotiation function */
#define PHY_POWERDOWN                   ((uint16_t)0x0800)      /*!< Select the power down mode */
#define PHY_ISOLATE                     ((uint16_t)0x0400)      /*!< Isolate PHY from MII */
/** PHY_BSR bit definitions */
#define PHY_AUTONEGO_COMPLETE           ((uint16_t)0x0020)      /*!< Auto-Negotioation process completed */
#define PHY_LINKED_STATUS               ((uint16_t)0x0004)      /*!< Valid link established */
#define PHY_JABBER_DETECTION            ((uint16_t)0x0002)      /*!< Jabber condition detected */
/** PHY_SR bit definitions */
//#define PHY_LINK_STATUS                ((uint16_t)0x0001)  /*!< PHY Link mask */
#define PHY_DUPLEX_STATUS           	((uint16_t)0x0010)    /*!< Configured information of Duplex: Full-duplex */
#define PHY_SPEED_STATUS            	((uint16_t)0x0004)    /*!< Configured information of Speed: 10Mbps */

#elif defined(DP83848_PHY)

#define PHY_ADDRESS       0x01

/** PHY register set */
#define PHY_BCR                          0          /*!< Tranceiver Basic Control Register */
#define PHY_BSR                          1          /*!< Tranceiver Basic Status Register */
#define PHY_SR                           16     	 /*!< Tranceiver Status Register */
#define PHY_MICR                        ((uint16_t)0x11)    /*!< MII Interrupt Control Register                  */
#define PHY_MISR                        ((uint16_t)0x12)    /*!< MII Interrupt Status and Misc. Control Register */

/** PHY_BCR bit definitions */
#define PHY_RESET                       ((uint16_t)0x8000)      /*!< PHY Reset */
#define PHY_LOOPBACK                    ((uint16_t)0x4000)      /*!< Select loop-back mode */
#define PHY_FULLDUPLEX_100M             ((uint16_t)0x2100)      /*!< Set the full-duplex mode at 100 Mb/s */
#define PHY_HALFDUPLEX_100M             ((uint16_t)0x2000)      /*!< Set the half-duplex mode at 100 Mb/s */
#define PHY_FULLDUPLEX_10M              ((uint16_t)0x0100)      /*!< Set the full-duplex mode at 10 Mb/s */
#define PHY_HALFDUPLEX_10M              ((uint16_t)0x0000)      /*!< Set the half-duplex mode at 10 Mb/s */
#define PHY_AUTONEGOTIATION             ((uint16_t)0x1000)      /*!< Enable auto-negotiation function */
#define PHY_RESTART_AUTONEGOTIATION     ((uint16_t)0x0200)      /*!< Restart auto-negotiation function */
#define PHY_POWERDOWN                   ((uint16_t)0x0800)      /*!< Select the power down mode */
#define PHY_ISOLATE                     ((uint16_t)0x0400)      /*!< Isolate PHY from MII */
/** PHY_BSR bit definitions */
#define PHY_AUTONEGO_COMPLETE           ((uint16_t)0x0020)      /*!< Auto-Negotioation process completed */
#define PHY_LINKED_STATUS               ((uint16_t)0x0004)      /*!< Valid link established */
#define PHY_JABBER_DETECTION            ((uint16_t)0x0002)      /*!< Jabber condition detected */
/** PHY_SR bit definitions */
#define PHY_LINK_STATUS                ((uint16_t)0x0001)  /*!< PHY Link mask */
#define PHY_DUPLEX_STATUS           	((uint16_t)0x0004)    /*!< Configured information of Duplex: Full-duplex */
#define PHY_SPEED_STATUS            	((uint16_t)0x0002)    /*!< Configured information of Speed: 10Mbps */

#define PHY_MICR_INT_EN                 ((uint16_t)0x0002)  /*!< PHY Enable interrupts                           */
#define PHY_MICR_INT_OE                 ((uint16_t)0x0001)  /*!< PHY Enable output interrupt events              */
#define PHY_MISR_LINK_INT_EN            ((uint16_t)0x0020)  /*!< Enable Interrupt on change of link status       */
#define PHY_LINK_INTERRUPT              ((uint16_t)0x2000)  /*!< PHY link status interrupt mask  */                 */

#elif defined(BCM5241_PHY)

#define PHY_ADDRESS       0x00

/** PHY register set */
#define PHY_BCR                          0x00          /*!< Tranceiver Basic Control Register */
#define PHY_BSR                          0x01          /*!< Tranceiver Basic Status Register */
#define PHY_SR 							 0x18	     	/*!< Auxiliary Control/Status register */
#define PHY_MICR                        ((uint16_t)0x1a)    /*!< MII Interrupt Control Register                  */
#define PHY_MISR                        ((uint16_t)0x1a)    /*!< MII Interrupt Status and Misc. Control Register */

/** PHY_BCR bit definitions */
#define PHY_RESET                       ((uint16_t)0x8000)      /*!< PHY Reset */
#define PHY_LOOPBACK                    ((uint16_t)0x4000)      /*!< Select loop-back mode */
#define PHY_SPEEDSELEC              	 ((uint16_t)0x2000)      /*!< Force Speed selection */
#define PHY_AUTONEGOTIATION             ((uint16_t)0x1000)      /*!< Enable Auto Negotiation */
#define PHY_POWERDOWN                   ((uint16_t)0x0800)      /*!< Power Down */
#define PHY_ISOLATE                     ((uint16_t)0x0400)      /*!< Isolate */
#define PHY_RESTART_AUTONEGOTIATION     ((uint16_t)0x0200)      /*!< Restart Auto Negotiation */
#define PHY_DUPLEXMODE                  ((uint16_t)0x0100)      /*!< Select duplex mode */
#define PHY_FULLDUPLEX_100M             ((uint16_t)0x2100)      /*!< Mask full-duplex mode at 100 Mb/s */
#define PHY_HALFDUPLEX_100M             ((uint16_t)0x2000)      /*!< Mask half-duplex mode at 100 Mb/s */
#define PHY_FULLDUPLEX_10M              ((uint16_t)0x0100)      /*!< Mask full-duplex mode at 10 Mb/s */
#define PHY_HALFDUPLEX_10M              ((uint16_t)0x0000)      /*!< Mask half-duplex mode at 10 Mb/s */
/** PHY_BSR bit definitions */
#define PHY_AUTONEGO_COMPLETE           ((uint16_t)0x0020)      /*!< Auto-Negotioation process completed */
#define PHY_LINKED_STATUS               ((uint16_t)0x0004)      /*!< Valid link established */
#define PHY_JABBER_DETECTION            ((uint16_t)0x0002)      /*!< Jabber condition detected */
/** PHY_SR bit definitions */
#define PHY_LINK_STATUS                ((uint16_t)0x0004)  /*!< PHY Link mask */
#define PHY_DUPLEX_STATUS           	((uint16_t)0x0001)    /*!< Configured information of Duplex: Full-duplex */
#define PHY_SPEED_STATUS            	((uint16_t)0x0002)    /*!< Configured information of Speed: 10Mbps */

#define PHY_MICR_INT_EN                 ((uint16_t)0x4000)  /*!< PHY Enable interrupts                           */
//#define PHY_MICR_INT_OE                 ((uint16_t)0x0001)  /*!< PHY Enable output interrupt events              */
#define PHY_MISR_LINK_INT_EN            ((uint16_t)0x0200)  /*!< Enable Interrupt on change of link status       */
#define PHY_LINK_INTERRUPT              ((uint16_t)0x0002)  /*!< PHY link status interrupt mask */


#elif defined(M88E6063_PHY)

#define PHY_0_ADDRESS 0x10
#define PHY_4_ADDRESS 0x14
#define SWITCH_0_ADDRESS 0x18
#define SWITCH_7_ADDRESS 0x1E
#define SWITCH_GLOBAL_CONTROL_ADDRESS 0x1F
#define SWITCH_GLOBAL_STATUS_REGISTER 0x00
#define SWITCH_GLOBAL_CONTROL_REGISTER 0x04
#define SWITCH_GLOBAL_CONTROL_REGISTER_PHY_INT_EN ((uint16_t)(0x01<<1))
#define BSP_LINK_STATUS_INDICATOR_PHY_ADDRESS 0x10 // (0x10 + portnum)
#define PHY_INTERRUPT_ENABLE_REGISTER 0x12
#define PHY_INTERRUPT_ENABLE_REGISTER_LINK_INT_EN  ((uint16_t)(0x01<<10))
#define SWITCH_PORT_CONTROL_REGISTER 0x04
#define SWITCH_PORT_CONTROL_REGISTER_PORT_STATE0 0x01
#define SWITCH_PORT_CONTROL_REGISTER_PORT_STATE1 0x02
#define PHY_INTERRUPT_STATUS_REGISTER 0x13
#define PHY_INTERRUPT_STATUS_REGISTER_LINK_INT  ((uint16_t)(0x01<<10))
#define PHY_SPECIFIC_STATUS_REGISTER 0x11
#define PHY_SPECIFIC_STATUS_REGISTER_RT_LINK ((uint16_t)(0x01<<10))

#define PHY_ADDRESS       PHY_0_ADDRESS

/** PHY register set */
#define PHY_BCR                          0x00          /*!< Tranceiver Basic Control Register */
#define PHY_BSR                          0x01          /*!< Tranceiver Basic Status Register */
#define PHY_SR 							 0x11	     	/*!< Auxiliary Control/Status register */
#define PHY_MICR                        ((uint16_t)0x12)    /*!< MII Interrupt Control Register                  */
#define PHY_MISR                        ((uint16_t)0x13)    /*!< MII Interrupt Status and Misc. Control Register */

/** PHY_BCR bit definitions */
#define PHY_RESET                       ((uint16_t)0x8000)      /*!< PHY Reset */
#define PHY_LOOPBACK                    ((uint16_t)0x4000)      /*!< Select loop-back mode */
#define PHY_SPEEDSELEC              	 ((uint16_t)0x2000)      /*!< Force Speed selection */
#define PHY_AUTONEGOTIATION             ((uint16_t)0x1000)      /*!< Enable Auto Negotiation */
#define PHY_POWERDOWN                   ((uint16_t)0x0800)      /*!< Power Down */
#define PHY_ISOLATE                     ((uint16_t)0x0400)      /*!< Isolate */
#define PHY_RESTART_AUTONEGOTIATION     ((uint16_t)0x0200)      /*!< Restart Auto Negotiation */
#define PHY_DUPLEXMODE                  ((uint16_t)0x0100)      /*!< Select duplex mode */
#define PHY_FULLDUPLEX_100M             ((uint16_t)0x2100)      /*!< Mask full-duplex mode at 100 Mb/s */
#define PHY_HALFDUPLEX_100M             ((uint16_t)0x2000)      /*!< Mask half-duplex mode at 100 Mb/s */
#define PHY_FULLDUPLEX_10M              ((uint16_t)0x0100)      /*!< Mask full-duplex mode at 10 Mb/s */
#define PHY_HALFDUPLEX_10M              ((uint16_t)0x0000)      /*!< Mask half-duplex mode at 10 Mb/s */
/** PHY_BSR bit definitions */
#define PHY_AUTONEGO_COMPLETE           ((uint16_t)0x0020)      /*!< Auto-Negotioation process completed */
#define PHY_LINKED_STATUS               ((uint16_t)0x0004)      /*!< Valid link established */
#define PHY_JABBER_DETECTION            ((uint16_t)0x0002)      /*!< Jabber condition detected */
/** PHY_SR bit definitions */
#define PHY_LINK_STATUS                ((uint16_t)0x0400)  /*!< PHY Link mask */
#define PHY_DUPLEX_STATUS           	((uint16_t)0x2000)    /*!< Configured information of Duplex: Full-duplex */
#define PHY_SPEED_STATUS            	((uint16_t)0x4000)    /*!< Configured information of Speed: 10Mbps */

//#define PHY_MICR_INT_EN                 ((uint16_t))  /*!< PHY Enable interrupts                           */
//#define PHY_MICR_INT_OE                 ((uint16_t))  /*!< PHY Enable output interrupt events              */
//#define PHY_MISR_LINK_INT_EN            ((uint16_t))  /*!< Enable Interrupt on change of link status       */
//#define PHY_LINK_INTERRUPT              ((uint16_t))  /*!< PHY link status interrupt mask */

#endif
