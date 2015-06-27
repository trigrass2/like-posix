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
 * This code is based heavily on demo code from STM...
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

/**
 * @defgroup ethernet
 *
 * @todo lots of duplication in this file, stm32f1 and f4 code can mostly be merged.
 *
 * @file MII_RMII.c
 * @{
 */

#include <stdbool.h>
#include "stm32_eth.h" // includes "MII_RMII.h" for us

static void ETH_GPIO_Config(void);

__IO uint32_t  EthInitStatus = 0;

static ETH_DMADESCTypeDef  DMARxDscrTab[ETH_RXBUFNB] __attribute__ ((aligned (4))); /* Ethernet Rx DMA Descriptor */
static ETH_DMADESCTypeDef  DMATxDscrTab[ETH_TXBUFNB] __attribute__ ((aligned (4))); /* Ethernet Tx DMA Descriptor */
static uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE] __attribute__ ((aligned (4))); /* Ethernet Receive Buffer */
static uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE] __attribute__ ((aligned (4))); /* Ethernet Transmit Buffer */

#if FAMILY == STM32F1

/**
  * @brief  Configures the Ethernet port.
  */
void ETH_Configuration(const uint8_t* macaddr)
{
	ETH_InitTypeDef ETH_InitStructure;

	// configure IO pins
	ETH_GPIO_Config();

/* MII/RMII Media interface selection */
#ifdef MII_MODE
	GPIO_ETH_MediaInterfaceConfig(GPIO_ETH_MediaInterface_MII);
#elif defined RMII_MODE
	GPIO_ETH_MediaInterfaceConfig(GPIO_ETH_MediaInterface_RMII);
	/* Get HSE clock = 25MHz on PA8 pin(MCO) */
	/* set PLL3 clock output to 50MHz (25MHz /5 *10 =50MHz) */
	RCC_PLL3Config(RCC_PLL3Mul_10);
	/* Enable PLL3 */
	RCC_PLL3Cmd(ENABLE);
	/* Wait till PLL3 is ready */
	while (RCC_GetFlagStatus(RCC_FLAG_PLL3RDY) == RESET);
#endif
#if ETH_USE_MCO
	RCC_MCOConfig(ETH_MCO_SRC);
#endif
	/* Reset ETHERNET on AHB Bus */
	ETH_DeInit();

	/* Enable ETHERNET clock  */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ETH_MAC | RCC_AHBPeriph_ETH_MAC_Tx | RCC_AHBPeriph_ETH_MAC_Rx, ENABLE);

	/* Software reset */
	ETH_SoftwareReset();

	/* Wait for software reset */
	while(ETH_GetSoftwareResetStatus()==SET);

	/* ETHERNET Configuration ------------------------------------------------------*/
	/* Call ETH_StructInit if you don't like to configure all ETH_InitStructure parameter */
	ETH_StructInit(&ETH_InitStructure);

	/* Fill ETH_InitStructure parametrs */
	/*------------------------   MAC   -----------------------------------*/
	ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable ;
	//ETH_InitStructure.ETH_Speed = ETH_Speed_100M;
	ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;
	//ETH_InitStructure.ETH_Mode = ETH_Mode_FullDuplex;
	ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable;
	ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;
	ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Enable;
	ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Disable;
	ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;
	ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;
	ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;

	/* Configure ETHERNET */
	EthInitStatus = ETH_Init(&ETH_InitStructure, PHY_ADDRESS);

	/* Initialize Tx Descriptors list: Chain Mode */
	ETH_DMATxDescChainInit(DMATxDscrTab, &Tx_Buff[0][0], ETH_TXBUFNB);
	/* Initialize Rx Descriptors list: Chain Mode  */
	ETH_DMARxDescChainInit(DMARxDscrTab, &Rx_Buff[0][0], ETH_RXBUFNB);

#if CHECKSUM_BY_HARDWARE
	/* Enable the TCP, UDP and ICMP checksum insertion for the Tx frames */
	for(uint8_t i=0; i<ETH_TXBUFNB; i++)
		ETH_DMATxDescChecksumInsertionConfig(&DMATxDscrTab[i], ETH_DMATxDesc_ChecksumTCPUDPICMPFull);
#endif

	/* initialize MAC address in ethernet MAC */
	ETH_MACAddressConfig(ETH_MAC_Address0, (uint8_t*)macaddr);

	/* Enable MAC and DMA transmission and reception */
	ETH_Start();
}

void ETH_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_TypeDef* eth_o_ports[] = ETH_GPIO_PORTS;
	uint16_t eth_o_pins[] = ETH_GPIO_PINS;
	GPIO_TypeDef* eth_i_ports[] = ETH_GPIO_INPUT_PORTS;
	uint16_t eth_i_pins[] = ETH_GPIO_INPUT_PINS;

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;

	for(uint8_t i = 0; i < sizeof(eth_o_pins)/sizeof(eth_o_pins[0]); i++)
	{
		GPIO_InitStructure.GPIO_Pin = eth_o_pins[i];
		GPIO_Init(eth_o_ports[i], &GPIO_InitStructure);
	}

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;

	for(uint8_t i = 0; i < sizeof(eth_i_pins)/sizeof(eth_i_pins[0]); i++)
	{
		GPIO_InitStructure.GPIO_Pin = eth_i_pins[i];
		GPIO_Init(eth_i_ports[i], &GPIO_InitStructure);
	}

#ifdef ETH_USE_MCO
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = ETH_MCO_PIN;
	GPIO_Init(ETH_MCO_PORT, &GPIO_InitStructure);
#endif
}

#elif FAMILY==STM32F4

/**
  * @brief  ETH_BSP_Config
  * @param  None
  * @retval None
  */
void ETH_Configuration(const uint8_t* macaddr)
{
	ETH_InitTypeDef ETH_InitStructure;
	ETH_GPIO_Config();

	// Enable SYSCFG clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

#ifdef MII_MODE
	SYSCFG_ETH_MediaInterfaceConfig(SYSCFG_ETH_MediaInterface_MII);
#elif defined RMII_MODE
	SYSCFG_ETH_MediaInterfaceConfig(SYSCFG_ETH_MediaInterface_RMII);
#endif
#ifdef ETH_USE_MCO
	RCC_MCO1Config(ETH_MCO_SRC, RCC_MCO1Div_1);
#endif

	/* Reset ETHERNET on AHB Bus */
	ETH_DeInit();

	/* Enable ETHERNET clock  */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_ETH_MAC | RCC_AHB1Periph_ETH_MAC_Tx |
						RCC_AHB1Periph_ETH_MAC_Rx, ENABLE);

	/* Software reset */
	ETH_SoftwareReset();

	/* Wait for software reset */
	while (ETH_GetSoftwareResetStatus() == SET);

	/* ETHERNET Configuration --------------------------------------------------*/
	/* Call ETH_StructInit if you don't like to configure all ETH_InitStructure parameter */
	ETH_StructInit(&ETH_InitStructure);

	/* Fill ETH_InitStructure parametrs */
	/*------------------------   MAC   -----------------------------------*/
	ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable;
	//ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Disable;
	//  ETH_InitStructure.ETH_Speed = ETH_Speed_10M;
	//  ETH_InitStructure.ETH_Mode = ETH_Mode_FullDuplex;

	ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;
	ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable;
	ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;
	ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Disable;
	ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;
	ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;
	ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;
	ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;
#if CHECKSUM_BY_HARDWARE
	ETH_InitStructure.ETH_ChecksumOffload = ETH_ChecksumOffload_Enable;
#endif

	/*------------------------   DMA   -----------------------------------*/

	/* When we use the Checksum offload feature, we need to enable the Store and Forward mode:
	the store and forward guarantee that a whole frame is stored in the FIFO, so the MAC can insert/verify the checksum,
	if the checksum is OK the DMA can handle the frame otherwise the frame is dropped */
	ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable;
	ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;
	ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;

	ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;
	ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable;
	ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;
	ETH_InitStructure.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;
	ETH_InitStructure.ETH_FixedBurst = ETH_FixedBurst_Enable;
	ETH_InitStructure.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;
	ETH_InitStructure.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;
	ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;

	/* Configure Ethernet */
	EthInitStatus = ETH_Init(&ETH_InitStructure, PHY_ADDRESS);

	/* Initialize Tx Descriptors list: Chain Mode */
	ETH_DMATxDescChainInit(DMATxDscrTab, &Tx_Buff[0][0], ETH_TXBUFNB);
	/* Initialize Rx Descriptors list: Chain Mode  */
	ETH_DMARxDescChainInit(DMARxDscrTab, &Rx_Buff[0][0], ETH_RXBUFNB);

#if CHECKSUM_BY_HARDWARE
	/* Enable the TCP, UDP and ICMP checksum insertion for the Tx frames */
	for(uint8_t i=0; i<ETH_TXBUFNB; i++)
		ETH_DMATxDescChecksumInsertionConfig(&DMATxDscrTab[i], ETH_DMATxDesc_ChecksumTCPUDPICMPFull);
#endif

	/* initialize MAC address in ethernet MAC */
	ETH_MACAddressConfig(ETH_MAC_Address0, (uint8_t*)macaddr);

	ETH_Start();
}

/**
  * @brief  Configures the different GPIO ports.
  * @param  None
  * @retval None
  */
void ETH_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_TypeDef* eth_io_ports[] = ETH_GPIO_PORTS;
	uint16_t eth_io_pins[] = ETH_GPIO_PINS;
	uint8_t eth_io_pinsources[] = ETH_GPIO_PINSOURCES;

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;

	for(uint8_t i = 0; i < sizeof(eth_io_pinsources)/sizeof(eth_io_pinsources[0]); i++)
	{
		GPIO_InitStructure.GPIO_Pin = eth_io_pins[i];
		GPIO_Init(eth_io_ports[i], &GPIO_InitStructure);
		GPIO_PinAFConfig(eth_io_ports[i], eth_io_pinsources[i], GPIO_AF_ETH);
	}

#ifdef ETH_USE_MCO
	// Configure MCO
	GPIO_InitStructure.GPIO_Pin = ETH_MCO_PIN;
	GPIO_Init(ETH_MCO_PORT, &GPIO_InitStructure);
	GPIO_PinAFConfig(ETH_MCO_PORT, ETH_MCO_PINSOURCE, GPIO_AF_MCO);
#endif

}

#endif


bool eth_link_status()
{
	return (bool)(ETH_ReadPHYRegister(PHY_ADDRESS, PHY_BSR) & PHY_Linked_Status);
}

uint16_t eth_link_speed()
{
	return ETH_ReadPHYRegister(PHY_ADDRESS, PHY_SR) & PHY_SPEED_STATUS ? 100 : 10;
}

bool eth_link_full_duplex()
{
	return (bool)(ETH_ReadPHYRegister(PHY_ADDRESS, PHY_SR) & PHY_DUPLEX_STATUS);
}

/**
 * @}
 */
