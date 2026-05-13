/*
 * Drv_PWM.h
 *
 *  Created on: 2024年9月11日
 *      Author: guowei680
 */

#ifndef DRIVER_DRV_PWM_H_
#define DRIVER_DRV_PWM_H_

//
// Phase A PWM
#define PCS_Q1_Q3_A_PWM_BASE       EPWM1_BASE
#define PCS_Q2_Q4_A_PWM_BASE       EPWM2_BASE
//
// Phase B PWM
#define PCS_Q1_Q3_B_PWM_BASE       EPWM3_BASE
#define PCS_Q2_Q4_B_PWM_BASE       EPWM4_BASE
//
// Phase C PWM
#define PCS_Q1_Q3_C_PWM_BASE       EPWM5_BASE
#define PCS_Q2_Q4_C_PWM_BASE       EPWM6_BASE

#define PCS_Q1_Q2_N_PWM_BASE       EPWM7_BASE

#define EPWM8_TBPRD                 25000        //100M/2K/2
#define EPWM_TBPRD                  3125        //100M/16K/2
#define EPWM_MIN                    400         //窄脉冲消除 4us


void Drv_PwmPin_Init(void);
void PWMEN_Open(void);
void PWMEN_Close(void);
void Drv_Pwm_Init(void);
void Drv_PwmOnset(void);
void Drv_PwmOffset(void);
void Drv_PWM_SYNC_Init(void);
void Drv_PWM_ParamInit(void);
void FanPwm_Init(void);
void FanPwm_ON(void);
void FanPwm_OFF(void);
void FanPwm_SpeedCtrl(int16_t duty);

#endif /* DRIVER_DRV_PWM_H_ */
