//#############################################################################
//
// FILE:   main.c
//

//
// Included Files
//
/* CODEMAP_MAIN
 * Role: CPU1 slow scheduler.
 * Read path: init -> parameter handoff -> fault check -> state machine
 * -> relay outputs -> soft-start references -> Modbus refresh/poll.
 * Fast ADC/control/PWM math is handled by CLA and PWM driver modules.
 */
#include <Driver/device/device.h>
#include <Driver/device/driverlib.h>
#include "bsp.h"
#include "aes_128.h"
#include "board.h"
#include "math.h"
extern float32_t MPPT_Duty;
extern float32_t Vdc_MPPT;
extern uint16_t ad_ntc1[3];
uint16_t TempBuf1 = 0, TempBuf2 = 0, TempBuf3 = 0;
int32_t  TempBuf[4] = {0};
int32_t TempCnt[4] = {0};

void main(void)
{
    int16_t runCnt = 0;
    int16_t ret_i16;

    /* CODEMAP_MAIN_INIT: initialize drivers, board IO, PWM, ADC, CLA, Modbus and globals. */
    bsp_init();
    EINT;
    ERTM;
    ret_i16 = Aes_Check();

    // Loop Forever
    for(;;)
    {
//        while(ret_i16>0) //检查错误，程序不运行
//        {
//            Drv_Led_toggle();
//            DEVICE_DELAY_US(100000);
//            ModbusRtu1_Poll();
//            ModbusRtu2_Poll();
//        }
        Drv_Timer_ClockMaintain();

    if(ret_i16==0)
    {
        /* CODEMAP_MAIN_CLA: copy CPU-side parameters/refs/gains/calibration to CLA. */
        cpu2claParam_Upgrade();
        if(DcDc_gParam.debugMode == 0)//调试模式手动控制继电器
        {
            /* CODEMAP_MAIN_PROTECT: protection runs before the start/stop state machine. */
            App_AllFault_Deal();
            objControl_StateCtrl();
        }
        /* CODEMAP_MAIN_RELAY: apply RelaySt_* variables to physical relay outputs. */
        objControl_RelayControl();
        obj_Output_KWHDeal();
        if(DcDc_gParam.SoftStart_En)
        {
            if(DcDc_gParam.CtrlMode == 1)//恒电压模式
                objControl_ConstVolCtrl_SoftStar();//
            if(DcDc_gParam.CtrlMode == 2)//恒电流模式
                objControl_ConstCurrCtrl_SoftStar();
            else if(DcDc_gParam.CtrlMode == 3)//恒功率模式
                objControl_ConstPowerCtrl_SoftStar();
        }
//        if (m_st_TimerFlag.u16_b100ms == 1)
//        {
//            if (DcDc_gParam.MPPT_En)
//            {
//                MPPT_PandO_Boost();
//            }
//            else
//            {
//                MPPT_Duty = 0.5f;
//                Vdc_MPPT = 70.0f;
//            }
//        }

        if(m_st_TimerFlag.u16_b50ms == 1)
        {
//            int16_t i = 0;
            TempBuf1 = (int16_t)calc_NtcTemp(ad_ntc1[0]);
            TempBuf2 = (int16_t)calc_NtcTemp(ad_ntc1[1]);
            TempBuf3 = (int16_t)calc_NtcTemp(ad_ntc1[2]);
            TempBuf[0] += TempBuf1;
            TempBuf[1] += TempBuf2;
            TempBuf[2] += TempBuf3;
            TempCnt[0]++;
            if(TempCnt[0] >= 7)
            {
                DC_OutMeter.DC_IGBT_Temp1 = TempBuf[0]>>3;
                DC_OutMeter.DC_IGBT_Temp2 = TempBuf[1]>>3;
                DC_OutMeter.DC_IGBT_Temp3 = TempBuf[2]>>3;
                TempCnt[0] = 0;
                TempBuf[0] = 0;
                TempBuf[1] = 0;
                TempBuf[2] = 0;
            }
        }
        if(m_st_TimerFlag.u16_b500ms == 1)
        {
            Drv_Led_toggle();
            Drv_WorkLed_toggle();
        }
        if(tCla2Cpu.PcsDebugi16_0 < 16000)
            GPIO_writePin(29, 1);
        else
            GPIO_writePin(29, 0);


        if(m_st_TimerFlag.u16_b500ms == 1)
            rtc_read_time();

        if(m_st_TimerFlag.u16_b100ms == 1)
        {
            /* CODEMAP_MAIN_MODBUS: refresh holding-register mirrors every 100 ms. */
            upDataHoldingCBReg();
        }
        if(m_st_TimerFlag.u16_b50ms == 1)
        {
            if(DcDc_gParam.debugMode == 0)//调试模式手动控制继电器
            {
                if(DcDc_gParam.DcCtrlState == DcDcOn_Mode)
                    DcDc_gParam.RelaySt_WorkLed = 1;
                else
                    DcDc_gParam.RelaySt_WorkLed = 0;
                if(DcDc_gParam.DcOnAllowed == 0) //有故障时
                    DcDc_gParam.RelaySt_FaultLed = 1;
                else
                    DcDc_gParam.RelaySt_FaultLed = 0;
            }
            objControl_FanControl();
        }
        if(m_st_TimerFlag.u16_b1s == 1)
        {
            if(tCpu2Cla.PwmStartEnable == 1)
                runCnt++;
            if(runCnt > 59)
            {
                runCnt = 0;
                DC_OutMeter.DC_RUN_TimeM++;
                if(DC_OutMeter.DC_RUN_TimeM > 65535)
                    DC_OutMeter.DC_RUN_TimeM = 0;
            }
        }
    }
    else
    {
        if(m_st_TimerFlag.u16_b100ms == 1)
            Drv_Led_toggle();
    }
        ModbusRtu1_Poll();
        ModbusRtu2_Poll();
    }
}

//
// End of File
//
