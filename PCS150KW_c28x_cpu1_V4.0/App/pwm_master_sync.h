/*
 * pwm_master_sync.h
 *
 *  Created on: 2026.04.29
 *      Author: whl
 */

#ifndef APP_PWM_MASTER_SYNC_H_
#define APP_PWM_MASTER_SYNC_H_

#include <stdint.h>
#include <stdbool.h>
#include "driverlib.h"


#ifdef __cplusplus
extern "C" {
#endif

void PWM_MasterSync_Init(void);
void PWM_SyncScopeDebug_Service(void);

__interrupt void PPS_Master_ECAP_ISR(void);

extern volatile uint16_t g_pwmScopeDebugEnable;
extern volatile uint16_t g_pwmScopeDebugState;
extern volatile uint16_t g_pwmScopeDebugCmpTicks;
extern volatile uint32_t g_pwmScopeDebugPpsCount;
extern volatile uint16_t g_pwmScopeDebugForceConfig;
extern volatile uint16_t g_pwmScopeDebugGpioMode;
extern volatile uint16_t g_pwmScopeDebugEpwmToggleMode;
extern volatile uint16_t g_pwmScopeDebugForceReload;
extern volatile uint16_t g_pwmScopeDebugLastToggleMode;
extern volatile uint32_t g_pwmScopeDebugConfigCount;
extern volatile uint32_t g_pwmScopeDebugServiceCount;

extern volatile uint32_t g_masterPpsCapCount;
extern volatile int32_t  g_masterPwmPhaseErrTicks;
extern volatile int32_t  g_masterPwmPhaseErrNs;
extern volatile uint32_t g_masterPwmLockCount;
extern volatile uint32_t g_masterPwmSkipCount;
extern volatile bool     g_masterPwmLocked;
extern volatile uint16_t g_masterPwmApplySync;


#ifdef __cplusplus
}
#endif
#endif /* APP_PWM_MASTER_SYNC_H_ */
