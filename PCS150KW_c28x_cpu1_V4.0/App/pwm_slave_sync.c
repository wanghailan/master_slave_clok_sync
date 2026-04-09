/*
 * pwm_slave_sync.c
 *
 *  Created on: 2026年4月7日
 *      Author: whl
 */

#include "pwm_slave_sync.h"
#include "bsp.h"
#include <math.h>

/*============================ PI控制器参数 ============================*/

// PI参数
#define SLAVE_PWM_KP                0.1f    // 比例增益
#define SLAVE_PWM_KI                0.01f   // 积分增益
#define SLAVE_PHASE_MAX_STEP        100U    // 最大相位调整步长

// PWM基准相位（PPS到来时PWM的目标TBCTR值）
// 上下计数模式下，0是周期中点
#define SLAVE_PWM_BASE_PHASE        0U
#define MAX_INTERGRAL_THRESHOLD     500.0f

// PPS中断计数
static volatile uint32_t g_slavePpsIsrCount = 0;
// 相位误差
static volatile int32_t  g_slavePhaseError = 0;
// 积分项
static volatile float  g_slavePhaseIntegral = 0.0f;
// PWM基准相位
static uint16_t g_slavePwmBasePhase = SLAVE_PWM_BASE_PHASE;

// 从CM核获取PTP同步状态
extern IPC_DATA_CPU2CM    Cpu1Ipc_cpu2cm;
extern IPC_DATA_CM2CPU    Cpu1Ipc_cm2cpu;


static void Slave_InitEPwm1(void);

static void Slave_InitPPS_Input_GPIO(void);


/*============================ 初始化函数 ============================*/
void PWM_SlaveSync_Init(void)
{
    // 1. 配置PWM1同步功能
    Slave_InitEPwm1();

    // 2.配置GPIO47作为PPS输入中断
    Slave_InitPPS_Input_GPIO();
}

// Slave PWM1同步配置
static void Slave_InitEPwm1(void)
{
    EALLOW;

//    // 配置PWM1同步输入, EPWM1使用EXTSYNCIN1作为同步输入源
//    SysCtl_setSyncInputConfig(SYSCTL_SYNC_IN_EPWM1, SYSCTL_SYNC_IN_SRC_EXTSYNCIN1);
//    //配置同步源EPWM1，使其作为同步链的起点
//    EPWM_setSyncInPulseSource(EPWM1_BASE, EPWM_SYNC_IN_PULSE_SRC_DISABLE);
//    // 配置同步后计数模式
//    EPWM_setCountModeAfterSync(EPWM1_BASE, EPWM_COUNT_MODE_UP_AFTER_SYNC);
//    // 使能相位加载
//    EPWM_enablePhaseShiftLoad(EPWM1_BASE);
//    // 初始相位置0
//    EPWM_setPhaseShift(EPWM1_BASE, 0);

    // 1.禁止所有EPWM模块的时基时钟
    SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);

    // 2.配置与EPWM1同步的从机模块, 配置EPWM1接收同步输入并加载相位
    EPWM_setSyncInPulseSource(EPWM1_BASE, EPWM_SYNC_IN_PULSE_SRC_SYNCOUT_EPWM1);

    // 3.配置同步后计数模式
    EPWM_setCountModeAfterSync(EPWM1_BASE, EPWM_COUNT_MODE_UP_AFTER_SYNC);

    // 4.初始相位偏移
    EPWM_setPhaseShift(EPWM1_BASE, 0U);

    // 5.使能相位加载
    EPWM_enablePhaseShiftLoad(EPWM1_BASE);

    // 6.禁用EPWM1自身的同步输出
    EPWM_disableSyncOutPulseSource(EPWM1_BASE, EPWM_SYNC_OUT_PULSE_ON_CNTR_ZERO);

    // 7.解冻时基时钟
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);

    EDIS;
}

// PPS_OUT输入中断配置，在CPU1端配置为GPIO输入中断
static void Slave_InitPPS_Input_GPIO(void)
{
    EALLOW;

    // Slave端配置GPIO47为GPIO输入
    GPIO_setPinConfig(GPIO_47_GPIO47);
    GPIO_setDirectionMode(47, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(47, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(47, GPIO_QUAL_ASYNC); // 异步输入，减少延迟

    // 配置GPIO47上升沿中断
    GPIO_setInterruptPin(47, GPIO_INT_XINT4);
    GPIO_setInterruptType(GPIO_INT_XINT4, GPIO_INT_TYPE_RISING_EDGE); // Interrupt on rising edge

    // 清除中断标志
    GPIO_disableInterrupt(GPIO_INT_XINT4);

    // 注册中断服务
    Interrupt_register(INT_XINT4, PPS_Slave_ISR);

    EDIS;
}

/*============================ PPS中断服务 ============================*/
__interrupt void PPS_Slave_ISR(void)
{
    // 清除GPIO中断标志
    GPIO_disableInterrupt(GPIO_INT_XINT4);

    // PPS中断计数
    g_slavePpsIsrCount++;

    // 检查CM核PTP同步是否完成
    if (Cpu1Ipc_cm2cpu.PtpSynced == 1)
    {
        //1. 读取当前PWM计数器值
        uint16_t tbctr = EPWM_getTimeBaseCounterValue(EPWM1_BASE);

        //2. 计算相位误差，当前计数器值 - 目标相位
        int32_t err = (int32_t)tbctr - (int32_t)g_slavePwmBasePhase;

        // 处理环绕
        uint16_t tbprd = EPWM_getTimeBasePeriod(EPWM1_BASE);
        if (err > tbprd / 2)
        {
            err -= tbprd;
        }
        else if (err < -(int32_t)(tbprd / 2))
        {
            err += tbprd;
        }

        g_slavePwmBasePhase = err;

        //3.PI计算调节相位误差
        //3.1积分项
        g_slavePhaseIntegral += SLAVE_PWM_KI * (float)err;

        // 边界限定，防止过饱和
        if (g_slavePhaseIntegral > MAX_INTERGRAL_THRESHOLD)
        {
            g_slavePhaseIntegral = MAX_INTERGRAL_THRESHOLD;
        }
        else if (g_slavePhaseIntegral < -MAX_INTERGRAL_THRESHOLD)
        {
            g_slavePhaseIntegral = -MAX_INTERGRAL_THRESHOLD;
        }

        //3.2比例项
        float g_slavePhaseProportional = SLAVE_PWM_KP * (float)err;

        //3.3总的增益
        float delta = g_slavePhaseProportional + g_slavePhaseIntegral;

        //3.4 限制调整stride
        if (delta > (float)SLAVE_PHASE_MAX_STEP)
        {
            delta = (float)SLAVE_PHASE_MAX_STEP;
        }
        else if (delta < -(float)SLAVE_PHASE_MAX_STEP)
        {
            delta = -(float)SLAVE_PHASE_MAX_STEP;
        }

        //4.计算新的相位差
        int32_t newPhase = (int32_t)g_slavePwmBasePhase - (int32_t)delta;

        //处理环绕
        if (newPhase < 0)
        {
            newPhase += tbprd;
        }
        else if (newPhase >= tbprd)
        {
            newPhase -= tbprd;
        }

        // 写入相位寄存器
        EPWM_setPhaseShift(EPWM1_BASE, (uint16_t)newPhase);
    }
}

