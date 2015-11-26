/**
 * @file
 * Ethernet Interface for standalone applications (without RTOS) - works only for 
 * ethernet polling mode (polling for ethernet frame reception)
 *
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
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
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include "ethernetif.h"

#include <string.h>
#include "net.h"
#include "lwip/mem.h"
#include "lwip/debug.h"
#include "etharp.h"
#include "net_config.h"
#include "board_config.h"

/* Network interface name */
#define IFNAME0 's'
#define IFNAME1 't'

#if USE_DRIVER_MII_RMII_PHY && USE_DRIVER_LWIP_NET

__ALIGN_BEGIN ETH_DMADescTypeDef  DMARxDscrTab[ETH_RXBUFNB] __ALIGN_END;/* Ethernet Rx MA Descriptor */
__ALIGN_BEGIN ETH_DMADescTypeDef  DMATxDscrTab[ETH_TXBUFNB] __ALIGN_END;/* Ethernet Tx DMA Descriptor */
__ALIGN_BEGIN uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE] __ALIGN_END; /* Ethernet Receive Buffer */
__ALIGN_BEGIN uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE] __ALIGN_END; /* Ethernet Transmit Buffer */

ETH_HandleTypeDef EthHandle;


bool eth_link_status()
{
	uint32_t reg;
	if(HAL_ETH_ReadPHYRegister(&EthHandle, PHY_BSR, &reg) == HAL_OK)
		return (bool)(reg & PHY_LINKED_STATUS);
	return false;
}

short eth_link_speed()
{
	uint32_t reg;
	if(HAL_ETH_ReadPHYRegister(&EthHandle, PHY_SR, &reg) == HAL_OK)
		return (reg & PHY_SPEED_STATUS) ? 100 : 10;
	return 0;
}

bool eth_link_full_duplex()
{
	uint32_t reg;
	if(HAL_ETH_ReadPHYRegister(&EthHandle, PHY_SR, &reg) == HAL_OK)
		return (bool)(reg & PHY_DUPLEX_STATUS);
	return false;
}


/**
  * @brief  Initializes the ETH MSP.
  * @param  heth: ETH handle
  * @retval None
  */
void HAL_ETH_MspInit(ETH_HandleTypeDef *heth)
{
	GPIO_TypeDef* eth_o_ports[] = ETH_GPIO_PORTS;
	uint16_t eth_o_pins[] = ETH_GPIO_PINS;
#if FAMILY == STM32F1
	GPIO_TypeDef* eth_i_ports[] = ETH_GPIO_INPUT_PORTS;
	uint16_t eth_i_pins[] = ETH_GPIO_INPUT_PINS;
#endif
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Configure PA1, PA2 and PA7 */
	/* Note : on MB1165 ETH_MDIO is connected to PA2 by default (SB40 is closed) */
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
#if FAMILY == STM32F4
	GPIO_InitStructure.Alternate = GPIO_AF0_MCO;
#endif

#ifdef ETH_USE_MCO
	// Configure MCO
	GPIO_InitStructure.Pin = ETH_MCO_PIN;
	HAL_GPIO_Init(ETH_MCO_PORT, &GPIO_InitStructure);
#endif

#if FAMILY == STM32F4
	GPIO_InitStructure.Alternate = GPIO_AF11_ETH;
#endif

	for(uint8_t i = 0; i < sizeof(eth_o_pins)/sizeof(eth_o_pins[0]); i++)
  	{
  		GPIO_InitStructure.Pin = eth_o_pins[i];
  		HAL_GPIO_Init(eth_o_ports[i], &GPIO_InitStructure);
  	}

#if FAMILY == STM32F1
	// STM32F1 requires input pins configured as inputs
	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;

	for(uint8_t i = 0; i < sizeof(eth_i_pins)/sizeof(eth_i_pins[0]); i++)
	{
		GPIO_InitStructure.Pin = eth_i_pins[i];
		HAL_GPIO_Init(eth_i_ports[i], &GPIO_InitStructure);
	}
#endif

	__HAL_RCC_ETH_CLK_ENABLE();

	HAL_NVIC_SetPriority(ETH_IRQn, 0x7, 0);
	HAL_NVIC_EnableIRQ(ETH_IRQn);

	__HAL_RCC_ETHMAC_CLK_ENABLE();
	__HAL_RCC_ETHMACTX_CLK_ENABLE();
	__HAL_RCC_ETHMACRX_CLK_ENABLE();
}

/**
  * @brief In this function, the hardware should be initialized.
  * Called from ethernetif_init().
  *
  * @param netif the already initialized lwip network interface structure
  *        for this ethernetif
  */
static void low_level_init(struct netif *netif)
{
  uint32_t regvalue = 0;

  EthHandle.Instance = ETH;
  EthHandle.State = HAL_ETH_STATE_RESET;
  EthHandle.Init.MACAddr = netif->hwaddr;
  EthHandle.Init.AutoNegotiation = ETH_AUTONEGOTIATION_ENABLE;
  EthHandle.Init.Speed = ETH_SPEED_100M;
  EthHandle.Init.DuplexMode = ETH_MODE_FULLDUPLEX;
  EthHandle.Init.MediaInterface = ETH_MEDIA_INTERFACE_MII;
  EthHandle.Init.RxMode = ETH_RXINTERRUPT_MODE;
  EthHandle.Init.ChecksumMode = ETH_CHECKSUM_BY_HARDWARE;
  EthHandle.Init.PhyAddress = PHY_ADDRESS;

#if ETH_USE_MCO
	HAL_RCC_MCOConfig(ETH_MCO_OUTPUT, ETH_MCO_SRC, RCC_MCODIV_1);
#endif

  /* configure ethernet peripheral (GPIOs, clocks, MAC, DMA) */
  if (HAL_ETH_Init(&EthHandle) != HAL_OK)
  {
	  // TODO - do what?
  }

  /* Initialize Tx Descriptors list: Chain Mode */
  HAL_ETH_DMATxDescListInit(&EthHandle, DMATxDscrTab, &Tx_Buff[0][0], ETH_TXBUFNB);

  /* Initialize Rx Descriptors list: Chain Mode  */
  HAL_ETH_DMARxDescListInit(&EthHandle, DMARxDscrTab, &Rx_Buff[0][0], ETH_RXBUFNB);

  /* Enable MAC and DMA transmission and reception */
  HAL_ETH_Start(&EthHandle);

  /**** Configure PHY to generate an interrupt when Eth Link state changes ****/
  /* Read Register Configuration */
  HAL_ETH_ReadPHYRegister(&EthHandle, PHY_MICR, &regvalue);

//  regvalue |= (PHY_MICR_INT_EN | PHY_MICR_INT_OE);
  regvalue |= (PHY_MICR_INT_EN);

  /* Enable Interrupts */
  HAL_ETH_WritePHYRegister(&EthHandle, PHY_MICR, regvalue);

  /* Read Register Configuration */
  HAL_ETH_ReadPHYRegister(&EthHandle, PHY_MISR, &regvalue);

  regvalue &= ~(PHY_MISR_LINK_INT_EN);

  /* Enable Interrupt on change of link status */
  HAL_ETH_WritePHYRegister(&EthHandle, PHY_MISR, regvalue);
}

/**
  * @brief This function should do the actual transmission of the packet. The packet is
  * contained in the pbuf that is passed to the function. This pbuf
  * might be chained.
  *
  * @param netif the lwip network interface structure for this ethernetif
  * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
  * @return ERR_OK if the packet could be sent
  *         an err_t value if the packet couldn't be sent
  *
  * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
  *       strange results. You might consider waiting for space in the DMA queue
  *       to become available since the stack doesn't retry to send a packet
  *       dropped because of memory failure (except for the TCP timers).
  */
static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
  err_t errval;
  struct pbuf *q;
  uint8_t *buffer = (uint8_t *)(EthHandle.TxDesc->Buffer1Addr);
  __IO ETH_DMADescTypeDef *DmaTxDesc;
  uint32_t framelength = 0;
  uint32_t bufferoffset = 0;
  uint32_t byteslefttocopy = 0;
  uint32_t payloadoffset = 0;

  DmaTxDesc = EthHandle.TxDesc;
  bufferoffset = 0;

  /* copy frame from pbufs to driver buffers */
  for(q = p; q != NULL; q = q->next)
  {
    /* Is this buffer available? If not, goto error */
    if((DmaTxDesc->Status & ETH_DMATXDESC_OWN) != (uint32_t)RESET)
    {
      errval = ERR_USE;
      goto error;
    }

    /* Get bytes in current lwIP buffer */
    byteslefttocopy = q->len;
    payloadoffset = 0;

    /* Check if the length of data to copy is bigger than Tx buffer size*/
    while( (byteslefttocopy + bufferoffset) > ETH_TX_BUF_SIZE )
    {
      /* Copy data to Tx buffer*/
      memcpy( (uint8_t*)((uint8_t*)buffer + bufferoffset), (uint8_t*)((uint8_t*)q->payload + payloadoffset), (ETH_TX_BUF_SIZE - bufferoffset) );

      /* Point to next descriptor */
      DmaTxDesc = (ETH_DMADescTypeDef *)(DmaTxDesc->Buffer2NextDescAddr);

      /* Check if the buffer is available */
      if((DmaTxDesc->Status & ETH_DMATXDESC_OWN) != (uint32_t)RESET)
      {
        errval = ERR_USE;
        goto error;
      }

      buffer = (uint8_t *)(DmaTxDesc->Buffer1Addr);

      byteslefttocopy = byteslefttocopy - (ETH_TX_BUF_SIZE - bufferoffset);
      payloadoffset = payloadoffset + (ETH_TX_BUF_SIZE - bufferoffset);
      framelength = framelength + (ETH_TX_BUF_SIZE - bufferoffset);
      bufferoffset = 0;
    }

    /* Copy the remaining bytes */
    memcpy( (uint8_t*)((uint8_t*)buffer + bufferoffset), (uint8_t*)((uint8_t*)q->payload + payloadoffset), byteslefttocopy );
    bufferoffset = bufferoffset + byteslefttocopy;
    framelength = framelength + byteslefttocopy;
  }

  /* Prepare transmit descriptors to give to DMA */
  HAL_ETH_TransmitFrame(&EthHandle, framelength);

  errval = ERR_OK;

error:

  /* When Transmit Underflow flag is set, clear it and issue a Transmit Poll Demand to resume transmission */
  if ((EthHandle.Instance->DMASR & ETH_DMASR_TUS) != (uint32_t)RESET)
  {
    /* Clear TUS ETHERNET DMA flag */
    EthHandle.Instance->DMASR = ETH_DMASR_TUS;

    /* Resume DMA transmission*/
    EthHandle.Instance->DMATPDR = 0;
  }
  return errval;
}

/**
  * @brief Should allocate a pbuf and transfer the bytes of the incoming
  * packet from the interface into the pbuf.
  *
  * @param netif the lwip network interface structure for this ethernetif
  * @return a pbuf filled with the received packet (including MAC header)
  *         NULL on memory error
  */
static struct pbuf * low_level_input(struct netif *netif)
{
  struct pbuf *p = NULL, *q = NULL;
  uint16_t len = 0;
  uint8_t *buffer;
  __IO ETH_DMADescTypeDef *dmarxdesc;
  uint32_t bufferoffset = 0;
  uint32_t payloadoffset = 0;
  uint32_t byteslefttocopy = 0;
  uint32_t i=0;
  
  /* get received frame */
  if(HAL_ETH_GetReceivedFrame_IT(&EthHandle) != HAL_OK)
    return NULL;
  
  /* Obtain the size of the packet and put it into the "len" variable. */
  len = EthHandle.RxFrameInfos.length;
  buffer = (uint8_t *)EthHandle.RxFrameInfos.buffer;
  
  if (len > 0)
  {
    /* We allocate a pbuf chain of pbufs from the Lwip buffer pool */
    p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
  }
  
  if (p != NULL)
  {
    dmarxdesc = EthHandle.RxFrameInfos.FSRxDesc;
    bufferoffset = 0;

    for(q = p; q != NULL; q = q->next)
    {
      byteslefttocopy = q->len;
      payloadoffset = 0;

      /* Check if the length of bytes to copy in current pbuf is bigger than Rx buffer size */
      while( (byteslefttocopy + bufferoffset) > ETH_RX_BUF_SIZE )
      {
        /* Copy data to pbuf */
        memcpy( (uint8_t*)((uint8_t*)q->payload + payloadoffset), (uint8_t*)((uint8_t*)buffer + bufferoffset), (ETH_RX_BUF_SIZE - bufferoffset));

        /* Point to next descriptor */
        dmarxdesc = (ETH_DMADescTypeDef *)(dmarxdesc->Buffer2NextDescAddr);
        buffer = (uint8_t *)(dmarxdesc->Buffer1Addr);

        byteslefttocopy = byteslefttocopy - (ETH_RX_BUF_SIZE - bufferoffset);
        payloadoffset = payloadoffset + (ETH_RX_BUF_SIZE - bufferoffset);
        bufferoffset = 0;
      }

      /* Copy remaining data in pbuf */
      memcpy( (uint8_t*)((uint8_t*)q->payload + payloadoffset), (uint8_t*)((uint8_t*)buffer + bufferoffset), byteslefttocopy);
      bufferoffset = bufferoffset + byteslefttocopy;
    }

    /* Release descriptors to DMA */
    /* Point to first descriptor */
    dmarxdesc = EthHandle.RxFrameInfos.FSRxDesc;
    /* Set Own bit in Rx descriptors: gives the buffers back to DMA */
    for (i=0; i< EthHandle.RxFrameInfos.SegCount; i++)
    {
      dmarxdesc->Status |= ETH_DMARXDESC_OWN;
      dmarxdesc = (ETH_DMADescTypeDef *)(dmarxdesc->Buffer2NextDescAddr);
    }

    /* Clear Segment_Count */
    EthHandle.RxFrameInfos.SegCount =0;
  }
  
  /* When Rx Buffer unavailable flag is set: clear it and resume reception */
  if ((EthHandle.Instance->DMASR & ETH_DMASR_RBUS) != (uint32_t)RESET)
  {
    /* Clear RBUS ETHERNET DMA flag */
    EthHandle.Instance->DMASR = ETH_DMASR_RBUS;
    /* Resume DMA reception */
    EthHandle.Instance->DMARPDR = 0;
  }
  return p;
}

void ETH_IRQHandler(void)
{
	HAL_ETH_IRQHandler(&EthHandle);
}

// TODO - this should be included but need to hook up some way of determining link state without GPIO interrupt lines
/**
  * @brief  This function sets the netif link status.
  * @param  netif: the network interface
  * @retval None
  */
//void ethernetif_set_link(struct netif *netif)
//{
//	uint32_t regvalue = 0;
//
//	HAL_ETH_ReadPHYRegister(&EthHandle, PHY_MISR, &regvalue);
//
//	/* Check whether the link is up or down*/
//	if(regvalue & PHY_LINK_INTERRUPT)
//	{
//	  netif_set_link_up(netif);
//	}
//	else
//	{
//	  netif_set_link_down(netif);
//	}
//}

// TODO - not this function is only called by way of ethernetif_set_link(), which isnt used
/**
  * @brief  Link callback function, this function is called on change of link status
  *         to update low level driver configuration.
  * @param  netif: The network interface
  * @retval None
  */
void ethernetif_update_config(struct netif *netif)
{
  __IO uint32_t tickstart = 0;
  uint32_t regvalue = 0;

  if(netif_is_link_up(netif))
  {
    /* Restart the auto-negotiation */
    if(EthHandle.Init.AutoNegotiation != ETH_AUTONEGOTIATION_DISABLE)
    {
      /* Enable Auto-Negotiation */
      HAL_ETH_WritePHYRegister(&EthHandle, PHY_BCR, PHY_AUTONEGOTIATION);

      /* Get tick */
      tickstart = HAL_GetTick();

      /* Wait until the auto-negotiation will be completed */
      do
      {
        HAL_ETH_ReadPHYRegister(&EthHandle, PHY_BSR, &regvalue);

        /* Check for the Timeout ( 1s ) */
        if((HAL_GetTick() - tickstart ) > 1000)
        {
          /* In case of timeout */
          goto error;
        }

      } while (((regvalue & PHY_AUTONEGO_COMPLETE) != PHY_AUTONEGO_COMPLETE));

      /* Read the result of the auto-negotiation */
      HAL_ETH_ReadPHYRegister(&EthHandle, PHY_SR, &regvalue);

      /* Configure the MAC with the Duplex Mode fixed by the auto-negotiation process */
      if((regvalue & PHY_DUPLEX_STATUS) != (uint32_t)RESET)
      {
        /* Set Ethernet duplex mode to Full-duplex following the auto-negotiation */
        EthHandle.Init.DuplexMode = ETH_MODE_FULLDUPLEX;
      }
      else
      {
        /* Set Ethernet duplex mode to Half-duplex following the auto-negotiation */
        EthHandle.Init.DuplexMode = ETH_MODE_HALFDUPLEX;
      }
      /* Configure the MAC with the speed fixed by the auto-negotiation process */
      if(regvalue & PHY_SPEED_STATUS)
      {
        /* Set Ethernet speed to 10M following the auto-negotiation */
        EthHandle.Init.Speed = ETH_SPEED_10M;
      }
      else
      {
        /* Set Ethernet speed to 100M following the auto-negotiation */
        EthHandle.Init.Speed = ETH_SPEED_100M;
      }
    }
    else /* AutoNegotiation Disable */
    {
    error :
      /* Check parameters */
      assert_param(IS_ETH_SPEED(EthHandle.Init.Speed));
      assert_param(IS_ETH_DUPLEX_MODE(EthHandle.Init.DuplexMode));

      /* Set MAC Speed and Duplex Mode to PHY */
      HAL_ETH_WritePHYRegister(&EthHandle, PHY_BCR, ((uint16_t)(EthHandle.Init.DuplexMode >> 3) |
                                                     (uint16_t)(EthHandle.Init.Speed >> 1)));
    }

    /* ETHERNET MAC Re-Configuration */
    HAL_ETH_ConfigMAC(&EthHandle, (ETH_MACInitTypeDef *) NULL);

    /* Restart MAC interface */
    HAL_ETH_Start(&EthHandle);
  }
  else
  {
    /* Stop MAC interface */
    HAL_ETH_Stop(&EthHandle);
  }
}


#elif USE_DRIVER_ENC28J60_PHY && USE_DRIVER_LWIP_NET

#include "enc28j60.h"

// TODO - does the setting CHECKSUM_BY_HARDWARE have an effect here?

/**
 * performs MAC/PHY level initialization
 */
static void low_level_init(void* macaddr)
{
    enc28j60_init((uint8_t*)macaddr);
}

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
    (void)netif;
    struct pbuf *q;
    int framelength = 0;

    enc28j60_send_packet_start();

    /* copy frame from pbufs to driver buffers */
    for(q = p; q != NULL; q = q->next)
    {
        enc28j60_write_buffer(q->payload, q->len);
        framelength = framelength + q->len;
    }

    enc28j60_send_packet_end(framelength);
    return ERR_OK;
}

static struct pbuf * low_level_input(struct netif *netif)
{
    (void)netif;
    struct pbuf *p, *q;
    uint16_t len;

    len = enc28j60_recv_packet_start(ETH_MAX_PACKET_SIZE);

    /* We allocate a pbuf chain of pbufs from the Lwip buffer pool */
    p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);

    if(p != NULL)
    {
        for (q = p; q != NULL; q = q->next)
        {
            enc28j60_read_buffer(q->payload, q->len);
        }
    }

    enc28j60_recv_packet_end();

    return p;
}

#else
#error "invalid ethernet device configuration"
#endif




/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
err_t ethernetif_input(struct netif *netif)
{
  err_t err;
  struct pbuf *p;

  /* move received packet into a new pbuf */
  p = low_level_input(netif);

  /* no packet could be read, silently ignore this */
  if (p == NULL) return ERR_MEM;

  /* entry point to the LwIP stack */
  err = netif->input(p, netif);
  
  if (err != ERR_OK)
  {
    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
    pbuf_free(p);
    p = NULL;
  }
  return err;
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t ethernetif_init(struct netif *netif)
{
  LWIP_ASSERT("netif != NULL", (netif != NULL));

  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output = etharp_output;
  netif->linkoutput = low_level_output;
  /* maximum transfer unit */
  netif->mtu = ETH_MAX_ETH_PAYLOAD;
  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
  /* initialize the hardware */
  low_level_init(netif);

  return ERR_OK;
}



