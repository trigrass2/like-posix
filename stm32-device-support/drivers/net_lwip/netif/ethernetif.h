#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__


#include <stdbool.h>

#include "lwip/err.h"
#include "lwip/netif.h"

err_t ethernetif_init(struct netif *netif);
err_t ethernetif_input(struct netif *netif);
void ethernetif_set_link(struct netif *netif);
void ethernetif_update_config(struct netif *netif);

void eth_reset_phy();
bool eth_link_status();
short eth_link_speed();
bool eth_link_full_duplex();

void ethernetif_enable_m88e6063_switch_port(uint8_t port, uint8_t state);

#endif // __ETHERNETIF_H__
