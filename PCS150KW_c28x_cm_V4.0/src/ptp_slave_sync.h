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


void ptp_slave_init(void);

void ptp_slave_run(void);

// 时钟调整函数
void ptp_slave_adjust_clock(void);


#endif /* SRC_PTP_SLAVE_SYNC_H_ */
