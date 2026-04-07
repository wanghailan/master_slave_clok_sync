/*
 * pwm_master_sync.h
 *
 *  Created on: 2026Äê4ÔÂ7ÈÕ
 *      Author: whl
 */

#ifndef APP_PWM_MASTER_SYNC_H_
#define APP_PWM_MASTER_SYNC_H_

#ifdef __cplusplus
extern "C" {
#endif

void PWM_MasterSync_Init(void);

__interrupt void PPS_Master_ISR(void);


#ifdef __cplusplus
}
#endif
#endif /* APP_PWM_MASTER_SYNC_H_ */
