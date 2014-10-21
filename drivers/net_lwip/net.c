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

#include <stdlib.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "stm32_eth.h"

#include "ethernetif.h"
#include "tcp_impl.h"
#include "dhcp.h"
#include "mem.h"
#include "memp.h"
#include "tcp.h"
#include "udp.h"
#include "etharp.h"

#include "strutils.h"
#include "cutensils.h"

#include "net_config.h"
#include "net.h"

#ifdef NET_LINK_LED
#include "leds.h"
#endif

static logger_t netlog;
static TaskHandle_t net_task_handle;

void net_task(void *pvParameters);


#if USE_CONFPARSE

bool string_to_address(uint8_t* address, const uint8_t* addr)
{
    uint8_t addrstr[16];
    const uint8_t* start;
    uint8_t* end;
    uint8_t dots = 0;

    // check length
    uint32_t length = strlen((const char*)addr);
    if(length < 7 || length > 15)
        return false;

    // check number of dots
    start = addr;
    while(*start)
    {
        if(*start == '.')
            dots++;
        start++;
    }

    if(dots != 3)
        return false;

    strcpy((char*)addrstr, (const char*)addr);

    end = addrstr;
    // for every section of address
    for(uint8_t i = 0; i < 4; i++)
    {
        start = end;
        uint8_t j = 0;
        while((*end != '.') && (j++ < 3))
            end++;
        *end = '\0';
        end++;
        address[i] = atoi((const char*)start);
    }

    return true;
}

bool string_to_mac_address(uint8_t* address, const uint8_t* macaddr)
{
    uint8_t octstr[3];
    uint32_t length = strlen((const char*)macaddr);

    // check length
    if(length != 17)
        return false;

    for(uint8_t oct = 0; oct < 6; oct++)
    {
        strncpy((char*)octstr, (const char*)macaddr+(oct*3), 2);
        address[oct] = ahtoi((char*)octstr);
    }

    return true;
}

void net_config(netconf_t* netconf, const char* resolv, const char* interface)
{
	config_parser_t cfg;
	uint8_t buffer[64];
	const uint8_t* prot;
	const uint8_t* hostname;
	bool mac_configured = false;
	bool ip_configured = false;
	bool nm_configured = false;
	bool gw_configured = false;

	log_init(&netlog, "netlog");

	// check for configuration protocol
	prot = get_config_value_by_key(buffer, sizeof(buffer), (const uint8_t*)resolv, (const uint8_t*)"resolv");
	if(string_match("dhcp", prot))
		netconf->resolv = NET_RESOLV_DHCP;
	else if(string_match("dns", prot))
		netconf->resolv = NET_RESOLV_DNS;
	else
		netconf->resolv = NET_RESOLV_STATIC;
	log_info(&netlog, "%s: %s=%s", resolv, "resolv", prot);

	// check for hostname
	hostname = get_config_value_by_key(buffer, sizeof(buffer), (const uint8_t*)resolv, (const uint8_t*)"hostname");
	strncpy((char*)netconf->hostname, (const char*)hostname, sizeof(netconf->hostname)-1);
	netconf->netif.hostname = (char*)netconf->hostname;
	log_info(&netlog, "%s: %s=%s", resolv, "hostname", hostname);

	//we must not allow the device to be configured with static data from flash - MAC/IP address conflicts will arise
	netconf->netif.ip_addr.addr = 0;
	netconf->netif.netmask.addr = 0;
	netconf->netif.gw.addr = 0;
	memset(netconf->netif.hwaddr, 0, sizeof(netconf->netif.hwaddr));

	// load settings from the config file
	if(open_config_file(&cfg, buffer, sizeof(buffer), (const uint8_t*)interface))
	{
		while(get_next_config(&cfg))
		{
			log_info(&netlog, "%s: %s=%s", interface, get_config_key(&cfg), get_config_value(&cfg));
			// only set details if static IP is required
			if(netconf->resolv == NET_RESOLV_STATIC)
			{
				if(config_key_match(&cfg, (const uint8_t*)"ipaddr"))
					ip_configured = string_to_address((uint8_t*)&(netconf->addr_cache[0].addr), get_config_value(&cfg));
				else if(config_key_match(&cfg, (const uint8_t*)"netmask"))
					nm_configured = string_to_address((uint8_t*)&(netconf->addr_cache[1].addr), get_config_value(&cfg));
				else if(config_key_match(&cfg, (const uint8_t*)"gateway"))
					gw_configured = string_to_address((uint8_t*)&(netconf->addr_cache[2].addr), get_config_value(&cfg));
			}
			// always set mac address
			if(config_key_match(&cfg, (const uint8_t*)"macaddr"))
			{
				mac_configured = string_to_mac_address(netconf->netif.hwaddr, get_config_value(&cfg));
				netconf->netif.hwaddr_len = ETHARP_HWADDR_LEN;
			}
		}
		close_config_file(&cfg);
	}

	// at least the mac address has to have been configured
	assert_true(mac_configured);
	// if we require static setting then all three of ip, netmask and gateway must be configured
	if(netconf->resolv == NET_RESOLV_STATIC)
	{
		assert_true(ip_configured);
		assert_true(nm_configured);
		assert_true(gw_configured);
	}
}

#endif

void net_init(netconf_t* netconf)
{
	log_init(&netlog, "netlog");

    assert_true(
    		xTaskCreate(net_task,
    					"lwIP",
                        configMINIMAL_STACK_SIZE+NET_TASK_STACK,
						netconf,
                        tskIDLE_PRIORITY+NET_TASK_PRIORITY,
						&net_task_handle)
	);
}

/**
 * link notification
 * TODO make this do something useful
 */
void link_callback(struct netif *netif)
{
	(void)netif;
	log_info(&netlog, "link callback");
}

/**
 * status notification
 * TODO make this do something useful
 */
void status_callback(struct netif *netif)
{
	uint8_t* pt;
	log_info(&netlog, "status callback");
	pt = (uint8_t*)&(netif->ip_addr.addr);
	log_info(&netlog, "ip: %d.%d.%d.%d", pt[0], pt[1], pt[2], pt[3]);
	pt = (uint8_t*)&(netif->netmask.addr);
	log_info(&netlog, "nm: %d.%d.%d.%d", pt[0], pt[1], pt[2], pt[3]);
	pt = (uint8_t*)&(netif->gw.addr);
	log_info(&netlog, "gw: %d.%d.%d.%d", pt[0], pt[1], pt[2], pt[3]);
	pt = netif->hwaddr;
	log_info(&netlog, "mac: %02x:%02x:%02x:%02x:%02x:%02x", pt[0], pt[1], pt[2], pt[3], pt[4], pt[5]);
#ifdef NET_LINK_LED
	set_led(NET_LINK_LED);
#endif
}

void net_task(void *pvParameters)
{
	netconf_t* netconf = (netconf_t*)pvParameters;
	uint32_t localtime;

	// Initializes the dynamic memory heap defined bybefore MEM_SIZE.
	mem_init();
	// Initializes the memory pools defined by MEMP_NUM_x.
	memp_init();

	netif_add(&netconf->netif, &netconf->addr_cache[0], &netconf->addr_cache[1], &netconf->addr_cache[2], NULL, &ethernetif_init, &ethernet_input);
	//  Registers the default network interface.
	netif_set_default(&netconf->netif);
	// setup callbacks
	netif_set_link_callback(&netconf->netif, link_callback);
	netif_set_status_callback(&netconf->netif, status_callback);

#ifdef USE_DHCP
	// DHCP brings the interface up for us if enabled
	if(netconf->resolv == NET_RESOLV_STATIC)
	{
		netif_set_up(&netconf->netif);
	}
	else
		netconf->dhcp_state = DHCP_START;
#else
	netif_set_up(&netconf->netif);
#endif

    for(;;)
    {
		if (ETH_CheckFrameReceived())
		{
			ethernetif_input(&netconf->netif);
		}

		localtime = xTaskGetTickCount()/portTICK_PERIOD_MS;

#if LWIP_TCP
		/* TCP periodic process every 250 ms */
		if (localtime - netconf->tcp_timer >= TCP_TMR_INTERVAL)
		{
			netconf->tcp_timer =  localtime;
			tcp_tmr();
		}
#endif

		/* ARP periodic process every 5s */
		if ((localtime - netconf->arp_timer) >= ARP_TMR_INTERVAL)
		{
			netconf->arp_timer =  localtime;
			etharp_tmr();
		}

#ifdef USE_DHCP
		if(netconf->resolv == NET_RESOLV_DHCP)
		{
			/* Fine DHCP periodic process every 500ms */
			if (localtime - netconf->dhcp_fine_timer >= DHCP_FINE_TIMER_MSECS)
			{
				netconf->dhcp_fine_timer =  localtime;
				dhcp_fine_tmr();

				if ((netconf->dhcp_state != DHCP_ADDRESS_ASSIGNED)&&(netconf->dhcp_state != DHCP_TIMEOUT))
				{
					/* toggle LED to indicate DHCP on-going process */
#ifdef NET_LINK_LED
					toggle_led(NET_LINK_LED);
#endif

					/* process DHCP state machine */
				  switch (netconf->dhcp_state)
				  {
					case DHCP_START:
					{
					  dhcp_start(&netconf->netif);
					  netconf->dhcp_state = DHCP_WAIT_ADDRESS;
					  log_info(&netlog, "waiting for DHCP server...");
					}
					break;

					case DHCP_WAIT_ADDRESS:
					{
					  if (netconf->netif.ip_addr.addr != 0)
					  {
						netconf->dhcp_state = DHCP_ADDRESS_ASSIGNED;
						/* Stop DHCP */
						dhcp_stop(&netconf->netif);
						log_info(&netlog, "DHCP done");
					  }
					  else
					  {
						/* DHCP timeout */
						if (netconf->netif.dhcp->tries > MAX_DHCP_TRIES)
						{
						  netconf->dhcp_state = DHCP_TIMEOUT;
						  /* Stop DHCP */
						  dhcp_stop(&netconf->netif);
						  log_info(&netlog, "DHCP timeout");

						  // TODO, hmmm, probably dont want to revert to a default address after dhcp timeout...
//						  /* Static address used */
//						  netif_set_addr(&netconf->netif, &netconf->addr_cache[0], &netconf->addr_cache[1], &netconf->addr_cache[2]);
						  // bring up the interface manually if DHCP failed
//						  netif_set_up(&netconf->netif);
						}
					  }
					}
					break;
					default: break;
				  }
				}
			}

			/* DHCP Coarse periodic process every 60s */
			if (localtime - netconf->dhcp_coarse_timer >= DHCP_COARSE_TIMER_MSECS)
			{
				netconf->dhcp_coarse_timer =  localtime;
				dhcp_coarse_tmr();
			}
		}
#endif

		taskYIELD();
	}
}


