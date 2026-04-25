/*
 * ptp_master_sync.h
 */

#ifndef SRC_PTP_MASTER_SYNC_H_
#define SRC_PTP_MASTER_SYNC_H_

#include <stdbool.h>
#include "driverlib_cm.h"

void ptp_master_init(void);

void ptp_master_run(void);

void ptp_master_receive_packet(Ethernet_Handle handleApplication,
                               Ethernet_Pkt_Desc *pPacket);

bool ptp_master_release_tx_packet(Ethernet_Handle handleApplication,
                                  Ethernet_Pkt_Desc *pPacket);

#endif /* SRC_PTP_MASTER_SYNC_H_ */
