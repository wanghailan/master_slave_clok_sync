/*
 * Eth_mii.h
 *
 *  Created on: 2025Äê6ÔÂ26ÈÕ
 *      Author: guowei
 */

#ifndef SRC_ETH_MII_H_
#define SRC_ETH_MII_H_
#include "eth_common.h"

void Lwip_ParamInit(void);
void Ethernet_init(const unsigned char *mac);
void lwIPHostTimerHandler(void);

extern uint32_t genericISRCustomcount;
extern uint32_t genericISRCustomRBUcount;
extern uint32_t genericISRCustomROVcount;
extern uint32_t genericISRCustomRIcount;


extern PTPMasterState gPtpMasterState;
extern uint8_t gMsgBuf[];

extern interrupt void Ethernet_genericISRCustom(void);

extern void sendMessage(Octet * msg,
                         uint32_t messageType,
                         PTPMasterState * ptpMasterState,
                         Ethernet_Pkt_Desc * pktDesc);

#endif /* SRC_ETH_MII_H_ */
