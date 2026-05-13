//#############################################################################
//
// FILE:   main.c
//

// Included Files
//
#include <Driver/device/device.h>
#include <Driver/device/driverlib.h>
#include "bsp.h"
#include "board.h"
#include "math.h"


#define  PTP_MODE_MASTER 1    // 1=Master，0=Slave

//static void* gAllowPpsSync = NULL;

void main(void)
{
    bsp_init();

//#if PTP_MODE_MASTER
//    PWM_MasterSync_Init(); // Master PWM初始化
//#else
//    PWM_SlaveSync_Init();  // Slave PWM初始化
//#endif

    for(;;)
    {
        if(Cpu1Ipc_cm2cpu.Reset_En == 0)
            SysCtl_serviceWatchdog();//看门狗喂狗
        Drv_Timer_ClockMaintain();
        cpu2claParam_Upgrade();
        App_AllFault_Deal();
        if(Cpu1Ipc_cm2cpu.SoftStart_En)
        {
            if(Cpu1Ipc_cm2cpu.WorkMode == GridDisConnectMode)
                Pcs_InvCtrl_SoftStar();
            else
            {
                if(Cpu1Ipc_cm2cpu.OnGridMode == 2)//恒直流电压模式
                    Pcs_DcConstVolCtrl_SoftStar();
                else if(Cpu1Ipc_cm2cpu.OnGridMode == 1)//恒交流电流
                    Pcs_AcConstCurrCtrl_SoftStar();
                else if(Cpu1Ipc_cm2cpu.OnGridMode == 3)//恒直流电流
                    Pcs_DcConstCurrCtrl_SoftStar();
                else if(Cpu1Ipc_cm2cpu.OnGridMode == 0)//恒功率
                    Pcs_PQConstCtrl_SoftStar();
            }
        }
        obj_PcsOutput_TempCalc();
        objControl_StateCtrl();
        objControl_FanControl();
        objControl_DOControl();
        objControl_LedControl();
        SciC_Poll();
        SciD_Poll();

//        // 检查CM侧是否已经完成PTP同步
//#if PTP_MODE_MASTER
//        PWM_MasterSync_run(gAllowPpsSync);
//#else
//        PWM_SlaveSync_run(gAllowPpsSync);
//#endif
    }
}

// End of File

