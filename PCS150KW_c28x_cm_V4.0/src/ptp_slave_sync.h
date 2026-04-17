/*
 * ptp_slave_sync.h
 *
 *  Created on: 2026年3月31日
 *      Author: whl
 */

#ifndef SRC_PTP_SLAVE_SYNC_H_
#define SRC_PTP_SLAVE_SYNC_H_

#include "driverlib_cm.h"
#include "eth_common.h"
#include "bsp.h"


extern PTPSlaveState gPtpSlaveState;

extern Ethernet_Handle emac_handle;

extern uint8_t gMsgBuf[PACKET_LENGTH];
extern Ethernet_Pkt_Desc gPktDesc;

extern IPC_DATA_CM2CPU CmIpc_cm2cpu;


/**
 * @brief Slave端PTP初始化
 */
void ptp_slave_init(void);

/**
 * @brief Slave端PTP运行函数，需要在主循环中周期性调用
 */
void ptp_slave_run(void);

/**
 * @brief 时钟调整函数，根据offset调整Slave本地PTP时钟
 */
void ptp_slave_adjust_clock(void);


#endif /* SRC_PTP_SLAVE_SYNC_H_ */
