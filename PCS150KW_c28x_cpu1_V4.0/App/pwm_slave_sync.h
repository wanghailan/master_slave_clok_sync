/*
 * pwm_slave_sync.h
 *
 *  Created on: 2026��4��7��
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

__interrupt void PPS_Slave_ECAP_ISR(void);

extern volatile uint32_t g_slavePpsCapCount;
extern volatile int32_t  g_slavePwmPhaseErrTicks;
extern volatile int32_t  g_slavePwmPhaseErrNs;
extern volatile uint32_t g_slavePwmLockCount;
extern volatile uint32_t g_slavePwmSkipCount;
extern volatile bool     g_slavePwmLocked;


#ifdef __cplusplus
}
#endif
#endif /* APP_PWM_SLAVE_SYNC_H_ */
