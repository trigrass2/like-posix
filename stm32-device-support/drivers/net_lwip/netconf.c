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

#include <string.h>
#include <stdlib.h>
#include "etharp.h"
#include "strutils.h"
#include "netconf.h"
#include "cutensils.h"


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

/**
 * configures a netconf_t structure, from files.
 *
 * compliments the net_init function, typical usage would be:
 *
\code

    #include "net.h"

    net_config(&netconf, "/etc/network/resolv", "/etc/network/interface");
    net_init(&netconf);
    while(wait_for_address(&netconf));

\code
 *
 * @param netconf is a netconf structure - it does not need to be preinitialized.
 * @param resolv is the file path to the file that configures address resolution.
 *        for example this might reside in /etc/network/resolv, and contains the following key value pairs:
 *
 *          resolv dhcp # may be dhcp or static
 *          hostname hostname # only used if LWIP_NETIF_HOSTNAME=1 in lwipopts.h
 *
 * @param interface is the file path to the file that configures the network interface.
 *        for example this might reside in /etc/network/interface, and contains the following key value pairs:
 *
 *          ipaddr  192.168.0.10 # your IP address
 *          macaddr 00:00:00:00:01 # your mac address
 *          netmask 255.255.255.0
 *          gateway 192.168.0.1 # your local gateway address
 *          dns1 192.168.0.1 # your dns server
 *          dns2 192.168.0.1 # your secondary dns server
 *
 */
void net_config(netconf_t* netconf, const char* resolv, const char* interface)
{
	logger_t log;
	config_parser_t cfg;
	uint8_t buffer[64];
	const char* prot;
	const char* hostname;
	bool mac_configured = false;
	bool ip_configured = false;
	bool nm_configured = false;
	bool gw_configured = false;
	bool dns1_configured = false;
	bool dns2_configured = false;

	log_init(&log, __FUNCTION__);

	// check for configuration protocol
	prot = (const char*)get_config_value_by_key(buffer, sizeof(buffer), (const uint8_t*)resolv, (const uint8_t*)"resolv");
	if(strcmp("dhcp", prot) == 0)
		netconf->resolv = NET_RESOLV_DHCP;
	else
		netconf->resolv = NET_RESOLV_STATIC;
	log_info(&log, "%s: %s=%s", resolv, "resolv", prot);

#if LWIP_NETIF_HOSTNAME
	// check for hostname
	hostname = (const char*)get_config_value_by_key(buffer, sizeof(buffer), (const uint8_t*)resolv, (const uint8_t*)"hostname");
	strncpy((char*)netconf->hostname, (const char*)hostname, sizeof(netconf->hostname)-1);
	netconf->netif.hostname = (char*)netconf->hostname;
	log_info(&log, "%s: %s=%s", resolv, "hostname", hostname);
#endif

	//we must not allow the device to be configured with static data from flash - MAC/IP address conflicts will arise
	netconf->addr_cache[0].addr = 0;
	netconf->addr_cache[1].addr = 0;
	netconf->addr_cache[2].addr = 0;
	netconf->addr_cache[3].addr = 0;
	netconf->addr_cache[4].addr = 0;
	netconf->netif.ip_addr.addr = 0;
	netconf->netif.netmask.addr = 0;
	netconf->netif.gw.addr = 0;
	memset(netconf->netif.hwaddr, 0, sizeof(netconf->netif.hwaddr));

	// load settings from the config file
	if(open_config_file(&cfg, buffer, sizeof(buffer), (const uint8_t*)interface))
	{
		while(get_next_config(&cfg))
		{
			log_info(&log, "%s: %s=%s", interface, get_config_key(&cfg), get_config_value(&cfg));
			// only set details if static IP is required
			if(netconf->resolv == NET_RESOLV_STATIC)
			{
				if(config_key_match(&cfg, (const uint8_t*)"ipaddr"))
					ip_configured = string_to_address((uint8_t*)&(netconf->addr_cache[0].addr), get_config_value(&cfg));
				else if(config_key_match(&cfg, (const uint8_t*)"netmask"))
					nm_configured = string_to_address((uint8_t*)&(netconf->addr_cache[1].addr), get_config_value(&cfg));
				else if(config_key_match(&cfg, (const uint8_t*)"gateway"))
					gw_configured = string_to_address((uint8_t*)&(netconf->addr_cache[2].addr), get_config_value(&cfg));
				else if(config_key_match(&cfg, (const uint8_t*)"dns1"))
					dns1_configured = string_to_address((uint8_t*)&(netconf->addr_cache[3].addr), get_config_value(&cfg));
				else if(config_key_match(&cfg, (const uint8_t*)"dns2"))
					dns2_configured = string_to_address((uint8_t*)&(netconf->addr_cache[4].addr), get_config_value(&cfg));
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
#if !USE_FULL_ASSERT
	(void)mac_configured;
#endif
	// if we require static setting then all three of ip, netmask and gateway must be configured
	if(netconf->resolv == NET_RESOLV_STATIC)
	{
		assert_true(ip_configured);
		assert_true(nm_configured);
		assert_true(gw_configured);
#if !USE_FULL_ASSERT
		(void)ip_configured;
		(void)nm_configured;
		(void)gw_configured;
	#endif
		if(!dns1_configured && !dns2_configured)
            log_warning(&log, "neither dns1 or dns2 addresses are configured");
	}
}

#endif

void net_config_static(netconf_t*netconf, net_resolv_prot_t resolv, const char* mac, const char* hostname,
		const char* gw, const char* nm, const char* ip, const char* dns1, const char* dns2)
{
	netconf->resolv = resolv;

	netconf->addr_cache[0].addr = 0;
	netconf->addr_cache[1].addr = 0;
	netconf->addr_cache[2].addr = 0;
	netconf->addr_cache[3].addr = 0;
	netconf->addr_cache[4].addr = 0;
	netconf->netif.ip_addr.addr = 0;
	netconf->netif.netmask.addr = 0;
	netconf->netif.gw.addr = 0;
	memset(netconf->netif.hwaddr, 0, sizeof(netconf->netif.hwaddr));

	string_to_mac_address(netconf->netif.hwaddr, (const uint8_t*)mac);
	netconf->netif.hwaddr_len = ETHARP_HWADDR_LEN;

	strncpy((char*)netconf->hostname, (const char*)hostname, sizeof(netconf->hostname)-1);
	netconf->netif.hostname = (char*)netconf->hostname;

	if(netconf->resolv == NET_RESOLV_STATIC)
	{
		string_to_address((uint8_t*)&(netconf->addr_cache[0].addr), (const uint8_t*)ip);
		string_to_address((uint8_t*)&(netconf->addr_cache[1].addr), (const uint8_t*)nm);
		string_to_address((uint8_t*)&(netconf->addr_cache[2].addr), (const uint8_t*)gw);
		string_to_address((uint8_t*)&(netconf->addr_cache[3].addr), (const uint8_t*)dns1);
		string_to_address((uint8_t*)&(netconf->addr_cache[4].addr), (const uint8_t*)dns2);
	}
}
