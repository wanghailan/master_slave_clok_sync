/*
 * pwm_master_sync.c
 *
 * 功能：
 *    Master的PPS是参考信号
 *    配置PWM同步输出功能，用于同步从机PWM模块
 *
 *  Created on: 2026年4月9日
 *      Author: whl
 */

#include "pwm_master_sync.h"
#include "bsp.h"

static void Master_InitSystemClock(void);

static void Master_InitEpwm1(void);


/*============================ 初始化函数 ============================*/
void PWM_MasterSync_Init(void)
{
    EALLOW;

//    // 配置PWM1在CTR=0时输出同步信号,用于同步从机PWM模块(PWM2)
//    EPWM_setSyncOutPulseMode(EPWM1_BASE, EPWM_SYNC_OUT_PULSE_ON_CNTR_ZERO);
//    EPWM_enableSyncOutPulseSource(EPWM1_BASE, EPWM_SYNC_OUT_PULSE_ON_CNTR_ZERO);
//    // 配置系统同步输出
//    SysCtl_setSyncOutputConfig(SYSCTL_SYNC_OUT_SRC_EPWM1SYNCOUT);

    // 1.在配置各个ePWM模块前，先禁止所有ePWM模块的时基时钟，防止初始化过程中产生意外的脉冲
    SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);

    // 2.配置同步源EPWM1，使其作为同步的起点.禁用自身同步输入，使能在CTR=0时输出SYNC0
    EPWM_setSyncInPulseSource(EPWM1_BASE, EPWM_SYNC_IN_PULSE_SRC_DISABLE);

    // 3.配置EPWM1在计数器归零时产生同步输出脉冲
    EPWM_enableSyncOutPulseSource(EPWM1_BASE, EPWM_SYNC_OUT_PULSE_ON_CNTR_ZERO);

    EDIS;
}

