/*
 * ethernet_dispatch.h
 *
 *  Created on: 2026年3月31日
 *      Author: whl
 */

#ifndef DRIVER_ETHERNET_DISPATCH_H_
#define DRIVER_ETHERNET_DISPATCH_H_

#include "eth_common.h"


extern uint32_t Ethernet_numRxCallbackCustom;
extern uint32_t releaseTxCount;

extern PTPMasterState gPtpMasterState;
extern PTPSlaveState gPtpSlaveState;

extern uint8_t gMsgBuf[];

extern uint32_t sendPacketFailedCount;

extern Ethernet_Handle emac_handle;
extern Ethernet_Device Ethernet_device_struct;
extern uint32_t Ethernet_numGetPacketBufferCallback;

extern Ethernet_Pkt_Desc pktDescriptorRXCustom[NUM_PACKET_DESC_RX_APPLICATION];


// ptpd release packet callback
extern void Ethernet_releaseTxPacketBufferPtp(
        Ethernet_Handle handleApplication,
        Ethernet_Pkt_Desc *pPacket);

// Ptp接收回调
extern Ethernet_Pkt_Desc* Ethernet_receivePacketCallbackPtp(
        Ethernet_Handle handleApplication,
        Ethernet_Pkt_Desc *pPacket);


extern void sendMessage(Octet *buf,
                        Enumeration4 msgType,
                        void *ptpState,
                        Ethernet_Pkt_Desc *pktDesc);


#endif /* DRIVER_ETHERNET_DISPATCH_H_ */
