/*
 * Eth_mii.h
 *
 *  Created on: 2026Äê3ÔÂ31ÈÕ
 *      Author: whl
 */

#ifndef DRIVER_ETH_MII_H_
#define DRIVER_ETH_MII_H_

#include "ethernet_dispatch.h"

void Lwip_ParamInit(void);
void Ethernet_init(const unsigned char *mac);
void lwIPHostTimerHandler(void);


#endif /* DRIVER_ETH_MII_H_ */
