/*
 * pwm_slave_sync.h
 *
 *  Created on: 2026Äê4ÔÂ7ÈÕ
 *      Author: whl
 */

#ifndef APP_PWM_SLAVE_SYNC_H_
#define APP_PWM_SLAVE_SYNC_H_

#include <stdint.h>
#include <stdbool.h>
#include "driverlib.h"


#ifdef __cplusplus
extern "C" {
#endif


void PWM_SlaveSync_Init(void);

__interrupt void PPS_Slave_ISR(void);


#ifdef __cplusplus
}
#endif
#endif /* APP_PWM_SLAVE_SYNC_H_ */
