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

#ifndef NET_CONF_H_
#define NET_CONF_H_

#define MAX_SOCKETS             5
#define NET_TASK_PRIORITY		2
#define NET_TASK_STACK			2048

#define NET_LINK_LED			LED2

/* Redefinition of the Ethernet driver buffers size and count */
#define ETH_RX_BUF_SIZE    ETH_MAX_PACKET_SIZE /* buffer size for receive */
#define ETH_TX_BUF_SIZE    ETH_MAX_PACKET_SIZE /* buffer size for transmit */
#define ETH_RXBUFNB        4                   /* 4 Rx buffers of size ETH_RX_BUF_SIZE */
#define ETH_TXBUFNB        2                   /* 2  Tx buffers of size ETH_TX_BUF_SIZE */

#define USE_DHCP							   /* enable DHCP, if disabled static address is used */
#define MAX_DHCP_TRIES     6

//#define ENC28J60_PHY
//#define DP83848_PHY
//#define LAN8700_PHY
#define BCM5241_PHY

#define MII_MODE
//#define RMII_MODE

#define PHY_READ_TO                     ((uint32_t)0x0004FFFF)
#define PHY_WRITE_TO                    ((uint32_t)0x0004FFFF)
#define PHY_ResetDelay                  ((uint32_t)0x000FFFFF)
#define PHY_ConfigDelay                 ((uint32_t)0x00FFFFFF)

#if defined(BCM5241_PHY)
#define PHY_ADDRESS       0x00
#elif defined(DP83848_PHY)
#define PHY_ADDRESS       0x01
#elif defined(LAN8700_PHY)
#define PHY_ADDRESS       0x01
#endif

#define ETH_USE_MCO				1
#define ETH_MCO_SRC				RCC_MCO1Source_HSE
//#define ETH_MCO_SRC				RCC_MCO_HSE

#define ETH_MCO_PORT				GPIOA
#define ETH_MCO_PIN					GPIO_Pin_8
#define ETH_MCO_PINSOURCE			GPIO_PinSource8

#define ETH_MII_CRS_PORT			GPIOA
#define ETH_MII_CRS_PIN  			GPIO_Pin_0
#define ETH_MII_CRS_PINSOURCE  		GPIO_PinSource0

#define ETH_MII_RX_CLK_PORT			GPIOA
#define ETH_MII_RX_CLK_PIN			GPIO_Pin_1
#define ETH_MII_RX_CLK_PINSOURCE  	GPIO_PinSource1

#define ETH_MII_MDIO_PORT  			GPIOA
#define ETH_MII_MDIO_PIN			GPIO_Pin_2
#define ETH_MII_MDIO_PINSOURCE    	GPIO_PinSource2

#define ETH_MII_COL_PORT			GPIOA
#define ETH_MII_COL_PIN				GPIO_Pin_3
#define ETH_MII_COL_PINSOURCE  		GPIO_PinSource3

#define ETH_MII_RX_DV_PORT   		GPIOA
#define ETH_MII_RX_DV_PIN			GPIO_Pin_7
#define ETH_MII_RX_DV_PINSOURCE   	GPIO_PinSource7

#define ETH_MII_RXD2_PORT    		GPIOB
#define ETH_MII_RXD2_PIN			GPIO_Pin_0
#define ETH_MII_RXD2_PINSOURCE    	GPIO_PinSource0

#define ETH_MII_RXD3_PORT    		GPIOB
#define ETH_MII_RXD3_PIN			GPIO_Pin_1
#define ETH_MII_RXD3_PINSOURCE    	GPIO_PinSource1

//#define ETH_MII_PPS_OUT_PORT 		GPIOB
//#define ETH_MII_PPS_OUT_PIN			GPIO_Pin_5
//#define ETH_MII_PPS_OUT_PINSOURCE 	GPIO_PinSource5

#define ETH_MII_TXD3_PORT   		GPIOB
#define ETH_MII_TXD3_PIN			GPIO_Pin_8
#define ETH_MII_TXD3_PINSOURCE    	GPIO_PinSource8

#define ETH_MII_RX_ER_PORT   		GPIOB
#define ETH_MII_RX_ER_PIN			GPIO_Pin_10
#define ETH_MII_RX_ER_PINSOURCE   	GPIO_PinSource10

#define ETH_MII_TX_EN_PORT   		GPIOB
#define ETH_MII_TX_EN_PIN			GPIO_Pin_11
#define ETH_MII_TX_EN_PINSOURCE   	GPIO_PinSource11

#define ETH_MII_TXD0_PORT    		GPIOB
#define ETH_MII_TXD0_PIN 			GPIO_Pin_12
#define ETH_MII_TXD0_PINSOURCE    	GPIO_PinSource12

#define ETH_MII_TXD1_PORT    		GPIOB
#define ETH_MII_TXD1_PIN			GPIO_Pin_13
#define ETH_MII_TXD1_PINSOURCE    	GPIO_PinSource13

#define ETH_MII_MDC_PORT  			GPIOC
#define ETH_MII_MDC_PIN				GPIO_Pin_1
#define ETH_MII_MDC_PINSOURCE  		GPIO_PinSource1

#define ETH_MII_TXD2_PORT    		GPIOC
#define ETH_MII_TXD2_PIN			GPIO_Pin_2
#define ETH_MII_TXD2_PINSOURCE    	GPIO_PinSource2

#define ETH_MII_TX_CLK_PORT 		GPIOC
#define ETH_MII_TX_CLK_PIN			GPIO_Pin_3
#define ETH_MII_TX_CLK_PINSOURCE  	GPIO_PinSource3

#define ETH_MII_RXD0_PORT    		GPIOC
#define ETH_MII_RXD0_PIN 			GPIO_Pin_4
#define ETH_MII_RXD0_PINSOURCE    	GPIO_PinSource4

#define ETH_MII_RXD1_PORT    		GPIOC
#define ETH_MII_RXD1_PIN 			GPIO_Pin_5
#define ETH_MII_RXD1_PINSOURCE    	GPIO_PinSource5

#define ETH_GPIO_PORTS  {\
	ETH_MII_CRS_PORT,ETH_MII_RX_CLK_PORT,ETH_MII_MDIO_PORT,ETH_MII_COL_PORT,\
	ETH_MII_RX_DV_PORT,ETH_MII_RXD2_PORT,ETH_MII_RXD3_PORT,ETH_MII_TXD3_PORT,\
	ETH_MII_RX_ER_PORT,ETH_MII_TX_EN_PORT,ETH_MII_TXD0_PORT,ETH_MII_TXD1_PORT,\
	ETH_MII_MDC_PORT,ETH_MII_TXD2_PORT,ETH_MII_TX_CLK_PORT,ETH_MII_RXD0_PORT,\
	ETH_MII_RXD1_PORT\
}

#define ETH_GPIO_PINS  {\
	ETH_MII_CRS_PIN,ETH_MII_RX_CLK_PIN,ETH_MII_MDIO_PIN,ETH_MII_COL_PIN,\
	ETH_MII_RX_DV_PIN,ETH_MII_RXD2_PIN,ETH_MII_RXD3_PIN,ETH_MII_TXD3_PIN,\
	ETH_MII_RX_ER_PIN,ETH_MII_TX_EN_PIN,ETH_MII_TXD0_PIN,ETH_MII_TXD1_PIN,\
	ETH_MII_MDC_PIN,ETH_MII_TXD2_PIN,ETH_MII_TX_CLK_PIN,ETH_MII_RXD0_PIN,\
	ETH_MII_RXD1_PIN\
}

#define ETH_GPIO_PINSOURCES  {\
	ETH_MII_CRS_PINSOURCE,ETH_MII_RX_CLK_PINSOURCE,ETH_MII_MDIO_PINSOURCE,\
	ETH_MII_COL_PINSOURCE,ETH_MII_RX_DV_PINSOURCE,ETH_MII_RXD2_PINSOURCE,\
	ETH_MII_RXD3_PINSOURCE,ETH_MII_TXD3_PINSOURCE,ETH_MII_RX_ER_PINSOURCE,\
	ETH_MII_TX_EN_PINSOURCE,ETH_MII_TXD0_PINSOURCE,ETH_MII_TXD1_PINSOURCE,\
	ETH_MII_MDC_PINSOURCE,ETH_MII_TXD2_PINSOURCE,ETH_MII_TX_CLK_PINSOURCE,\
	ETH_MII_RXD0_PINSOURCE,ETH_MII_RXD1_PINSOURCE\
}

#endif // NET_CONF_H_
