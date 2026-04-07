/*
 * pwm_master_sync.c
 *
 *  Created on: 2026年4月7日
 *      Author: whl
 */

#include "pwm_master_sync.h"
#include "F28x_Project.h"
#include "ipc_data.h"


extern IPC_DATA_CM2CPU   Cpu1Ipc_cm2cpu;

volatile struct ECAP_REGS ECap1Regs;
volatile struct EPWM_REGS EPwm1Regs;
volatile struct GPIO_CTRL_REGS GpioCtrlRegs;
volatile struct PIE_CTRL_REGS PieCtrlRegs;
volatile struct PIE_VECT_TABLE PieVectTable;


// PPS上升沿, PWM TBCTR=0
static const uint16_t g_masterPwmBasePhase = 0;

// 调试计数
static volatile uint32_t g_masterPpsIsrCount = 0;

static void Master_InitSystemClock(void);
static void Master_InitEPwm1(void);
static void Master_InitPPS_Input_ECAP(void);

// 初始化
void PWM_MasterSync_Init(void)
{
    Master_InitSystemClock();

    DINT;
    InitPieCtrl();
    IER = 0x0000;
    IFR = 0x0000;

    InitPieVectTable();

    Master_InitEPwm1();
    Master_InitPPS_Input_ECAP();

    EINT;
    DRTM;
}

// 系统时钟
static void Master_InitSystemClock(void)
{
    InitSysCtrl();
}

// EPWM1配置:master
static void Master_InitEPwm1(void)
{
    EALLOW;
    InitEPwm1Gpio();

    EPwm1Regs.TBCTL.bit.CTRMODE   = TB_COUNT_UPDOWN;
    EPwm1Regs.TBCTL.bit.PHSEN     = TB_DISABLE;
    EPwm1Regs.TBCTL.bit.PRDLD     = TB_SHADOW;
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
    EPwm1Regs.TBCTL.bit.CLKDIV    = TB_DIV1;

    EPwm1Regs.TBPRD = 5000;  // 100MHz / (2*5000) = 10kHz

    EPwm1Regs.TBPHS.bit.TBPHS = 0;
    EPwm1Regs.TBCTR = 0;

    // 50%占空比
    EPwm1Regs.CMPA.bit.CMPA = 2500;
    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm1Regs.AQCTLA.bit.CAD = AQ_SET;

    // 本地同步
    //EPwm1Regs.TBCTL2.bit.SYNCOSEL = TB_CTR_ZERO;

    EDIS;
}

// PPS Input + ECAP1 Interrupt
static void Master_InitPPS_Input_ECAP(void)
{
    EALLOW;

    GpioCtrlRegs.GPBPUD.bit.GPIO47 = 0;      // 0 = 使能上拉, 1 = 禁用上拉
    GpioCtrlRegs.GPBQSEL1.bit.GPIO47 = 0;    // 同步到 SYSCLKOUT
    GpioCtrlRegs.GPBMUX1.bit.GPIO47 = 3;     // GPIO47 -> ECAP1

    // ECAP1配置：捕获上升沿，用事件1触发中断
    ECap1Regs.ECEINT.all = 0x0000;
    ECap1Regs.ECCLR.all  = 0xFFFF;
    ECap1Regs.ECCTL1.all = 0x0000;
    ECap1Regs.ECCTL2.all = 0x0000;

    ECap1Regs.ECCTL1.bit.CAPLDEN = 1;
    ECap1Regs.ECCTL1.bit.CAP1POL = 0;    // 上升沿
    ECap1Regs.ECCTL1.bit.CTRRST1 = 1;    // 捕获后复位计数器

    ECap1Regs.ECCTL2.bit.TSCTRSTOP = 1;  // 启动计数
    ECap1Regs.ECCLR.bit.CEVT1 = 1;
    ECap1Regs.ECEINT.bit.CEVT1 = 1;      // 事件1中断

    PieVectTable.ECAP1_INT = &PPS_Master_ISR;
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
    PieCtrlRegs.PIEIER4.bit.INTx1 = 1;

    IER |= M_INT4;
    EDIS;
}

// PPS ISR
__interrupt void PPS_Master_ISR(void)
{
    ECap1Regs.ECCLR.bit.CEVT1 = 1;
    ECap1Regs.ECCLR.bit.INT   = 1;

    g_masterPpsIsrCount++;

    // 在CM核完成PTP同步后，才进行一次对齐
    if (Cpu1Ipc_cm2cpu.PtpSynced == 1)
    {
        // 在PPS上升沿直接把TBCTR置为基准相位
        EPwm1Regs.TBCTR = g_masterPwmBasePhase;
    }

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP4;
}

