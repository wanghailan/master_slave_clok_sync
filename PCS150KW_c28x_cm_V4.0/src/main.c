
#include <string.h>

#include "driverlib_cm.h"
#include "cm.h"
#include "utils/lwiplib.h"

#include "lwipopts.h"
#include "Eth_mii.h"
#include "ipc.h"
#include "bsp.h"
#include "ptp_basic_master.h"


uint32_t systickPeriodValue = 125000; //15000000;

extern void sys_check_timeouts(void);


#define DEVICE_FLASH_WAITSTATES 2


//*****************************************************************************
//
// The interrupt handler for the SysTick interrupt.
//
//*****************************************************************************
void
SysTickIntHandler(void)
{
    // Call the lwIP timer handler.
 //   lwIPTimer(systickPeriodValue);
	   lwIPTimer(1);
}

int main(void)
{
    uint8_t tempData[50];                 //定义的传输Buffer

    // Initializing the CM. Loading the required functions to SRAM.
    CM_init();

    SYSTICK_setPeriod(systickPeriodValue);
    SYSTICK_enableCounter();
    SYSTICK_registerInterruptHandler(SysTickIntHandler);
    SYSTICK_enableInterrupt();

    // Enable processor interrupts.
    Interrupt_enableInProcessor();

    bsp_init();
    do{
        DEVICE_DELAY_US(100);
    } while(cpuIpc_flag == 0);

    //ptp_master_init();
    //ptp_valid_init();

    Lwip_ParamInit();

    ptp_master_init();

    udpDebug_Init();
    MbTcp1_Init();
    w5500_init();
    while (1)
    {
        Drv_Timer_ClockMaintain();
        upDataHoldingCBReg();

        if(m_st_TimerFlag.u16_b500ms == 1)
            Drv_Led_toggle();

        if(m_st_TimerFlag.u16_b500ms == 1)
            rtc_read_time();
        if(m_st_TimerFlag.u16_b1ms == 1)
        {
            ipc_TxData_CmToCpu2(ipcCmd_Param,(uint16_t *)&CmIpc_cm2cpu,sizeof(CmIpc_cm2cpu)>>1);
            if(CmIpc_cm2cpu.ClearFault == 1)
                CmIpc_cm2cpu.ClearFault = 0;
        }

        if(m_st_TimerFlag.u16_b10ms == 1)
        {
            IpcCpu2Cm_ErrCnt++;

            if(IpcCpu2Cm_ErrCnt >= 100)//1s
            {
                IpcCpu2Cm_ErrCnt = 100;
                CmIpc_cm2cpu.IpcCpu2Cm_Fault = 1;
            }
        }

        ModbusRtu1_Poll();
        ModbusRtu2_Poll();

        if(m_st_TimerFlag.u16_b10ms == 1)
            w5500_TcpMainServer();

        Output_KWHDeal();
        Output_RunTimeCnt();

        if(flag_ipcRx == 1)
        {
            if(CmIpc_cm2cpu.debugData_TxEn == 1)
            {
                memcpy(tempData, (uint8_t *)ipcRx_DebugData, 40);//通过拷贝把数据重新整理
                tempData[40] = 0x00;                    //写如结尾数据
                tempData[41] = 0x00;
                tempData[42] = 0x80;
                tempData[43] = 0x7f;
                udpDebug_TxData(tempData,44);
            }
            flag_ipcRx = 0;
        }
        sys_check_timeouts();

        // ptp
        ptp_master_run();
    }
}


