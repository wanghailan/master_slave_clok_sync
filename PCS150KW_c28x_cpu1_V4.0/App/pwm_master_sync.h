/*
 * pwm_master_sync.h
 *
 *  Created on: 2026.04.29
 *      Author: whl
 *
 * 功能说明:
 *   本模块通过 eCAP1捕获PTP PPS 上升沿，精确测量 PPS与EPWM1 的相位差，
 *   经PI控制器补偿后，在ISR内通过软件强制同步 (SWFSYNC) 立即调整EPWM1相位，
 *   实现主从机PWM零滞后、50ns级相位同步。
 */

#ifndef APP_PWM_MASTER_SYNC_H_
#define APP_PWM_MASTER_SYNC_H_

#include <stdint.h>
#include <stdbool.h>
#include "driverlib.h"

#ifdef __cplusplus
extern "C" {
#endif

// 主机同步模块初始化
void PWM_MasterSync_Init(void);

// eCAP1 PPS捕获中断服务程序
__interrupt void PPS_Master_ECAP_ISR(void);


/* Master PPS-to-EPWM servo diagnostics and runtime tuning variables. */
extern volatile uint32_t g_masterPpsCapCount;          // PPS捕获总次数
extern volatile int32_t  g_masterPwmPhaseErrTicks;     // 原始相位误差(TBCLK ticks)
extern volatile int32_t  g_masterPwmPhaseErrNs;        // 相位误差(ns)
extern volatile uint32_t g_masterPwmLockCount;         // 连续锁定计数
extern volatile uint32_t g_masterPwmSkipCount;         // 异常跳过计数
extern volatile bool     g_masterPwmLocked;            // 锁定状态标志

extern volatile uint16_t g_masterPwmApplySync;         // 1=允许同步; 0=仅观测不调整
extern volatile int32_t  g_masterPwmPhaseOffsetNs;     // 基础相位偏移 (主机默认0ns)
extern volatile int32_t  g_masterPwmLockThresholdNs;   // 锁定判定阈值 (默认50ns)
extern volatile int32_t  g_masterPwmMaxStepNs;         // PI 输出每步最大调整量 (默认100ns)

extern volatile uint16_t g_masterPwmHardwareSyncEnable;// 0=软件同步; 1=硬件同步
extern volatile uint16_t g_masterPwmPiEnable;          // 1=启用PI补偿
extern volatile int32_t  g_masterPwmPiOffsetNs;        // PI当前输出偏移(ns)
extern volatile int32_t  g_masterPwmPiIntegralNs;      // PI积分项(ns)
extern volatile int32_t  g_masterPwmPiMaxOffsetNs;     // PI输出限幅(默认±2000ns)
extern volatile uint16_t g_masterPwmPiKpDiv;           // Kp = 1/KpDiv (默认2, 即Kp=0.5)
extern volatile uint16_t g_masterPwmPiKiDiv;           // Ki = 1/KiDiv (默认4, 即Ki=0.25)
extern volatile uint32_t g_masterPwmApplyCount;        // 实际施加同步次数


#ifdef __cplusplus
}
#endif
#endif /* APP_PWM_MASTER_SYNC_H_ */
