#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__


#include <stdbool.h>

#include "lwip/err.h"
#include "lwip/netif.h"

err_t ethernetif_incoming();
err_t ethernetif_init(struct netif *netif);
err_t ethernetif_input(struct netif *netif);

bool eth_link_status();
short eth_link_speed();
bool eth_link_full_duplex();

#endif
