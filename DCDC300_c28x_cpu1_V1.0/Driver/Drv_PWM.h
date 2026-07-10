/*
 * Drv_PWM.h
 *
 *  Created on: 2024Äê9ÔÂ11ÈÕ
 *      Author: guowei680
 */

#ifndef DRIVER_DRV_PWM_H_
#define DRIVER_DRV_PWM_H_

//
// Part1 PWM
//#define DCP1_Q1_Q2_A_PWM_BASE       EPWM8_BASE
//#define DCP1_Q3_Q4_A_PWM_BASE       EPWM7_BASE
//
// Part2 PWM
//#define DCP2_Q1_Q2_A_PWM_BASE       EPWM6_BASE
//#define DCP2_Q3_Q4_A_PWM_BASE       EPWM5_BASE

/* CODEMAP_PWM_H
 * PWM base mapping and period definitions.
 * EPWM_TBPRD=3125 corresponds to about 32 kHz with a 200 MHz timebase in up-down count mode.
 */#define EPWM_TBPRD                 3125         //200M/32K/2
#define EPWM9_TBPRD                50000        //200M/2K/2

#define DCP1_Q1_Q2_A_PWM_BASE       EPWM1_BASE
#define DCP1_Q3_Q4_A_PWM_BASE       EPWM2_BASE
#define DCP1_Q7_Q8_A_PWM_BASE       EPWM4_BASE


void Drv_PwmPin_Init(void);
void PWMEN_Open(void);
void PWMEN_Close(void);
void Drv_Pwm_Init(void);
void Drv_PwmOnset(void);
void Drv_PwmOffset(void);
void Drv_PWM_SYNC_Init(void);
void Drv_PWM_ParamInit(void);
//void FanPwm_Init(void);
//void FanPwm_ON(void);
//void FanPwm_OFF(void);
//void FanPwm_SpeedCtrl(int16_t duty);

#endif /* DRIVER_DRV_PWM_H_ */
