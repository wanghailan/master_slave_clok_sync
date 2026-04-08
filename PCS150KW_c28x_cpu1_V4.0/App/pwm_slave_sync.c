/*
 * pwm_slave_sync.c
 *
 *  Created on: 2026年4月7日
 *      Author: whl
 */

#include "pwm_slave_sync.h"
#include <Driver/device/device.h>
#include <Driver/device/driverlib.h>

//#include "f2838x_device.h"
//#include "f2838x_epwm_defines.h"
//#include "f2838x_pie_defines.h"
//#include "f2838x_globalprototypes.h"

#if 0
#include "f28x_project.h"
#include "bsp.h"

// PI Params
#define SLAVE_PWM_KP          (0.1f)
#define SLAVE_PWM_KI          (0.01f)
#define SLAVE_PHASE_MAX_STEP  (100U)

// PPS上升沿PWM的TBCTR相位
static const uint16_t g_slavePwmBasePhase = 0;

static volatile int32_t  g_slavePhaseError = 0;
static volatile float32_t  g_slavePhaseIntegral = 0.0f;
static volatile uint32_t g_slavePpsIsrCount   = 0;

extern IPC_DATA_CPU2CM         Cpu1Ipc_cpu2cm;
extern IPC_DATA_CM2CPU         Cpu1Ipc_cm2cpu;


static void Slave_InitSystemClock(void);
static void Slave_InitEPwm1(void);
static void Slave_InitPPS_Input_ECAP(void);

void InitEPwm1Gpio(void);
void InitPieCtrl(void);
void InitPieVectTable(void);
void InitSysCtrl(void);


// 初始化
void PWM_SlaveSync_Init(void)
{
    Slave_InitSystemClock();

    DINT;
    InitPieCtrl();
    IER = 0x0000;
    IFR = 0x0000;
    InitPieVectTable();

    Slave_InitEPwm1();
    Slave_InitPPS_Input_ECAP();

    EINT;
    ERTM;
}

// system clock
static void Slave_InitSystemClock(void)
{
    InitSysCtrl();
}

// Slave EPWMS Configuration
static void Slave_InitEPwm1(void)
{
    EALLOW;
    InitEPwm1Gpio();

    // PWM Frequency (10kHz)
    EPwm1Regs.TBCTL.bit.CTRMODE   = TB_COUNT_UPDOWN;
    EPwm1Regs.TBCTL.bit.PHSEN     = TB_ENABLE;
    EPwm1Regs.TBCTL.bit.PRDLD     = TB_SHADOW;
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
    EPwm1Regs.TBCTL.bit.CLKDIV    = TB_DIV1;

    EPwm1Regs.TBPRD = 5000;  // 100MHz / (2*5000) = 10kHz
    EPwm1Regs.TBPHS.bit.TBPHS = 0;
    EPwm1Regs.TBCTR = 0;

    EPwm1Regs.CMPA.bit.CMPA  = 2500;
    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm1Regs.AQCTLA.bit.CAD = AQ_SET;

    //EPwm1Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO;

    EDIS;
}

// PPS -> ECAP1 触发PI控制
static void Slave_InitPPS_Input_ECAP(void)
{
    EALLOW;

    GpioCtrlRegs.GPBPUD.bit.GPIO47   = 0;
    GpioCtrlRegs.GPBQSEL1.bit.GPIO47 = 0;
    GpioCtrlRegs.GPBMUX1.bit.GPIO47  = 3; // ECAP1

    ECap1Regs.ECEINT.all = 0x0000;
    ECap1Regs.ECCLR.all  = 0xFFFF;
    ECap1Regs.ECCTL1.all = 0x0000;
    ECap1Regs.ECCTL2.all = 0x0000;

    ECap1Regs.ECCTL1.bit.CAPLDEN = 1;
    ECap1Regs.ECCTL1.bit.CAP1POL = 0;  // 上升沿
    ECap1Regs.ECCTL1.bit.CTRRST1 = 1;

    ECap1Regs.ECCTL2.bit.TSCTRSTOP = 1;

    ECap1Regs.ECCLR.bit.CEVT1 = 1;
    ECap1Regs.ECEINT.bit.CEVT1 = 1;

    PieVectTable.ECAP1_INT = &PPS_Slave_ISR;
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
    PieCtrlRegs.PIEIER4.bit.INTx1 = 1;

    IER |= M_INT4;

    EDIS;
}

// PPS ISR (Slave PWM phase PI sync)
__interrupt void PPS_Slave_ISR(void)
{
    ECap1Regs.ECCLR.bit.CEVT1 = 1;
    ECap1Regs.ECCLR.bit.INT   = 1;

    g_slavePpsIsrCount++;

    // 只有CM侧PTP同步完成后才闭环
    if (Cpu1Ipc_cm2cpu.PtpSynced == 1)
    {
        uint16_t tbctr = EPwm1Regs.TBCTR;

        // 相位误差: 当前计数 - 期望相位
        int32_t error = (int32_t)tbctr - (int32_t)g_slavePwmBasePhase;
        g_slavePhaseError = error;

        // PI调节
        g_slavePhaseIntegral += SLAVE_PWM_KI * (float)error;

        float delta = SLAVE_PWM_KP * (float)error + g_slavePhaseIntegral;

        // 限幅
        if (delta > (float)SLAVE_PHASE_MAX_STEP)
            delta = (float)SLAVE_PHASE_MAX_STEP;
        if (delta < -(float)SLAVE_PHASE_MAX_STEP)
            delta = -(float)SLAVE_PHASE_MAX_STEP;

        int32_t newPhase = (int32_t)g_slavePwmBasePhase - (int32_t)delta;

        if (newPhase < 0)
            newPhase += EPwm1Regs.TBPRD;
        else if (newPhase >= EPwm1Regs.TBPRD)
            newPhase -= EPwm1Regs.TBPRD;

        EPwm1Regs.TBPHS.bit.TBPHS = (uint16_t)newPhase;
    }

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP4;
}

#endif
