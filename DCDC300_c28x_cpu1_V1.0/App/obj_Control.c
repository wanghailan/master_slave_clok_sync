//#############################################################################
//
// FILE:   TINV_FUN.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

/* CODEMAP_CONTROL_C
 * Role: start/stop logic, relay state generation, fan control and soft-start ramps.
 * Key path: StartEn -> objControl_StateCtrl() -> RelaySt_* -> objControl_RelayControl().
 * PWM hardware is enabled later by Dsp_runISR1() only when state is DcDcOn_Mode
 * and tCpu2Cla.PwmStartEnable is already true.
 */
#include <Driver/device/device.h>
#include <Driver/device/driverlib.h>
#include "bsp.h"
#include "fastrts.h"
#include "fpu32/C28x_FPU_FastRTS.h"



float32_t  PhaseErr;



static int16_t i16_IntCnt = 0;
int16_t flag_send = 0;


void objControl_globalVarInit(void)
{

}


//
// Lab1 ISR code
//
int16_t Cnt_PwmEnDelay;
extern float32_t Vdc_MPPT;
extern float32_t Power;
extern float32_t dP;
extern float32_t dV;
#pragma FUNC_ALWAYS_INLINE(Dsp_runISR1)
/* CODEMAP_ISR1
 * Fast CPU-side ISR helper.
 * 1) periodically calculates mean voltage/current/power values
 * 2) sends selected data to another CPU/core through IPC
 * 3) gates the real PWM output: state must be DcDcOn_Mode and CLA PWM enable must be true
 */
inline void Dsp_runISR1(void)
{
    switch(i16_IntCnt)
    {
        case 0:
            obj_VolCurr_MeanCalc();
            i16_IntCnt = 1;
            break;
        case 1:
            obj_Power_MeanCalc();
            i16_IntCnt = 2;
            break;
        case 2:
            i16_IntCnt = 3;
            break;
        default:
            i16_IntCnt = 0;
            break;
    }

    if(i16_IntCnt == 3)
    {
        packetData[0] = tCla2Cpu.DC_VBus_sensed_pu*DC_VDC_MAX_SENSE_RATIO;
        packetData[1] = tCla2Cpu.DC_VBusS_sensed_pu*DC_VDC_MAX_SENSE_RATIO;//
        packetData[2] = tCla2Cpu.DC_VBat_sensed_pu*DC_VDC_MAX_SENSE_RATIO;//
        packetData[3] = tCla2Cpu.DC_VBatS_sensed_pu*DC_VDC_MAX_SENSE_RATIO;//
        packetData[4] = tCla2Cpu.DC_IBat_sensed_pu*DC_IDC200_MAX_SENSE_RATIO;//tCla2Cpu.DC_IBus_sensed_pu*DC_IDC200_MAX_SENSE_RATIOtCpu2Cla.DcDc_VdcRef
        packetData[5] = tCla2Cpu.DC_IBus_sensed_pu*DC_IDC300_MAX_SENSE_RATIO;//tCla2Cpu.DC_IBat_sensed_pu*DC_IDC200_MAX_SENSE_RATIOtCla2Cpu.PcsDebugf32_0tCla2Cpu.DC_VBatN_sensed_pu*DC_VDC_MAX_SENSE_RATIO
        packetData[6] = tCla2Cpu.PwmRatio3;//
        packetData[7] = tCla2Cpu.DC_IL1_sensed_pu*DC_IDC100_MAX_SENSE_RATIO;//*DC_IDC100_MAX_SENSE_RATIO
        packetData[8] = tCla2Cpu.DC_IL2_sensed_pu*DC_IDC100_MAX_SENSE_RATIO;//dP
        packetData[9] = tCla2Cpu.DC_IL3_sensed_pu*DC_IDC100_MAX_SENSE_RATIO;//dV
        flag_send = 1;
         ipc_TxData(10);
         flag_send = 0;
    }

    if((DcDc_gParam.DcCtrlState == DcDcOn_Mode)&&(tCpu2Cla.PwmStartEnable==1))//
    {
        if(Cnt_PwmEnDelay < 200)
        {
            Cnt_PwmEnDelay++;
        }
        else
        {
            Drv_PwmOnset();
        }
    }
    else
    {
        Cnt_PwmEnDelay = 0;
        Drv_PwmOffset();
    }
}


/*
 * AdcDat 閿熻鍑ゆ嫹鍘嬮敓鏂ゆ嫹鐗￠敓绐栨�拷
 *  TempDat = (int16_t)((float)AdcDat*0.00457771f)*6+36;
 * */
/* CODEMAP_NTC
 * Convert raw NTC ADC data into temperature*10.
 * main.c averages three NTC channels and stores them into DC_OutMeter temperature fields.
 */
int16_t calc_NtcTemp(uint16_t AdcDat)
{
    float temp;
    float Rt = 0;
    //Rp 閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷峰�� 10K
    float Rp = 10000.0f;
    float R25 = 5000.0f;//5K 25閿熸枻鎷�
    //T2涓�25閿熼ズ锝忔嫹閿熸枻鎷烽敓鏂ゆ嫹涓洪敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熼摪璁规嫹
    float T25 = 298.15f;//273.15+25;
    float Bx = 3433.0f;//B25/100
    float Ka = 273.15f;
    float Vol;
    uint16_t u16_temp;
    int16_t i16_tempx10;

    u16_temp = AdcDat;
//    Vol = (float)u16_temp*0.0001373312+0.18f;//3*3.0/65535.0;
    Vol = (float)u16_temp*0.0000457763671875f;//3.0/65535.0;
    Rt = (Vol*Rp)/(3.3f-Vol);//閿熸枻鎷峰帇閿熸枻鎷烽敓鏂ゆ嫹Rp 閿熸枻鎷峰帇5V
    temp = 1.0f/(1/T25+logf(Rt/R25)/Bx)-Ka+0.5f;

    i16_tempx10 = (int16_t)(temp*10.0f);

    return i16_tempx10;
}




/* CODEMAP_RELAY_CONTROL
 * Converts software relay state variables to physical outputs.
 * During commissioning, verify every mapping with a meter because variable names and
 * actual contactor wiring may not be the same as the schematic labels.
 */
void objControl_RelayControl(void)
{
    Drv_RelaySet(Relay_SoftBus,     DcDc_gParam.RelaySt_DcBusSoft);
    Drv_RelaySet(Relay_SoftBat,     DcDc_gParam.RelaySt_DcBatSoft);
    Drv_RelaySet(Relay_MastBat,     DcDc_gParam.RelaySt_DcBusMaster);
    Drv_RelaySet(Relay_MastBus,     DcDc_gParam.RelaySt_DcBatMaster);
    Drv_RelaySet(Relay_FAN,         DcDc_gParam.RelaySt_FanCtrl);
    Drv_RelaySet(Relay_WorkLed,     DcDc_gParam.RelaySt_WorkLed);
    Drv_RelaySet(Relay_FaultLed,    DcDc_gParam.RelaySt_FaultLed);

//    if(m_st_TimerFlag.u16_b5ms == 1)
//        Drv_UpExtDo();
}

/* CODEMAP_FAN_CONTROL
 * Automatic fan hysteresis in normal mode: turn on above about 45 C, off below about 30 C.
 */
void objControl_FanControl(void)
{
    DC_OutMeter.DC_IGBT_TempMax = DC_OutMeter.DC_IGBT_Temp1;
    if(DC_OutMeter.DC_IGBT_TempMax < DC_OutMeter.DC_IGBT_Temp1)
        DC_OutMeter.DC_IGBT_TempMax = DC_OutMeter.DC_IGBT_Temp1;
    if(DC_OutMeter.DC_IGBT_TempMax < DC_OutMeter.DC_IGBT_Temp2)
        DC_OutMeter.DC_IGBT_TempMax = DC_OutMeter.DC_IGBT_Temp2;
    if(DC_OutMeter.DC_IGBT_TempMax < DC_OutMeter.DC_IGBT_Temp3)
        DC_OutMeter.DC_IGBT_TempMax = DC_OutMeter.DC_IGBT_Temp3;
//    if(DC_OutMeter.DC_IGBT_TempMax < DC_OutMeter.DC_IGBT_Temp4)
//        DC_OutMeter.DC_IGBT_TempMax = DC_OutMeter.DC_IGBT_Temp4;
    DC_OutMeter.DC_IGBT_TempMax *= 0.1f;
//IGBT閿熸枻鎷烽敓鏂ゆ嫹闇茬樃閿熸枻鎷烽敓锟�45閿熼ズ锝忔嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓锟�
    if(DcDc_gParam.debugMode == 0)//閿熸枻鎷烽敓鏂ゆ嫹妯″紡閿熻璁规嫹閿熸枻鎷烽敓鐙＄户纰夋嫹閿熸枻鎷�
    {
        if(DcDc_gParam.RelaySt_FanCtrl == 0)
        {
            if(DC_OutMeter.DC_IGBT_TempMax < 45.0f) //閿熼摪搴︾鎷烽敓鏂ゆ嫹45閿熸枻鎷�
                DcDc_gParam.RelaySt_FanCtrl = 0;
            else
                DcDc_gParam.RelaySt_FanCtrl = 1;
        }
        else
        {
            if(DC_OutMeter.DC_IGBT_TempMax < 30.0f) //閿熼摪搴︾鎷烽敓鏂ゆ嫹30閿熸枻鎷�
                DcDc_gParam.RelaySt_FanCtrl = 0;
        }
    }
}
//PCS鐘舵�侀敓鏂ゆ嫹
static int16_t CtrlDelayCnt = 0;
static int16_t CtrlDelayCnt1 = 0;

/* CODEMAP_STATE_MACHINE
 * Main startup/shutdown state machine, called from main.c in normal mode.
 * Important sequence:
 *   Off -> PreStart -> BatSoft/BusSoft -> BatStart/BusStart -> DcDcOn
 * It only prepares relays and state. Pwm_StartEn/PwmStartEnable are handled separately.
 */
void objControl_StateCtrl(void)
{
   if(m_st_TimerFlag.u16_b1ms == 1)
   {
      if((DcDc_gParam.DcOnAllowed == 0)||(DcDc_gParam.StartEn == 0))//閿熸枻鎷烽敓鍙櫢鎷烽敓鏂ゆ嫹閿熷彨鐧告嫹閿熸枻鎷� 閿熸枻鎷疯浆閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷锋ā寮�
      {
         CtrlDelayCnt = 0;
         CtrlDelayCnt1 = 0;
         DcDc_gParam.Pwm_StartEn = 0;
         if(tCpu2Cla.PwmStartEnable == 0)
             DcDc_gParam.DcCtrlState = DcDcOff_Mode;
      }
       /* CODEMAP_STATE_SWITCH: each case below is one visible DcCtrlState value for Modbus/status logs. */
       switch(DcDc_gParam.DcCtrlState)
       {
           case PowerOn_Mode://閿熻緝纰夋嫹
               DcDc_gParam.RelaySt_DcBusSoft   = 0;
               DcDc_gParam.RelaySt_DcBatSoft   = 0;
               DcDc_gParam.RelaySt_DcBusMaster = 0;
               DcDc_gParam.RelaySt_DcBatMaster = 0;
               if(DcDc_gParam.SelfCheckOK == 0)//閿熻緝纰夋嫹閿熺殕纭锋嫹閿熺潾鐧告嫹閿熸枻鎷�
               {
                   CtrlDelayCnt++;
                   if(CtrlDelayCnt > 5000)//閿熻緝纰夋嫹閿熸枻鎷锋椂5S
                   {
                       DcDc_gParam.DcCtrlState = DcDcOff_Mode;
                       CtrlDelayCnt = 0;
                   }
               }
               else
               {
                   CtrlDelayCnt = 0;
                   DcDc_gParam.DcCtrlState = PowerOn_Mode;
               }
               break;
           case DcDcOff_Mode://閿熸埅浼欐嫹妯″紡
               GPIO_writePin(19, 1);
               DcDc_gParam.RelaySt_DcBusSoft   = 0;
               DcDc_gParam.RelaySt_DcBatSoft   = 0;
               DcDc_gParam.RelaySt_DcBusMaster = 0;
               DcDc_gParam.RelaySt_DcBatMaster = 0;
               CtrlDelayCnt = 0;
               if(DcDc_gParam.StartEn == 1)//閿熺Ц纰夋嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹
               {
                   DcDc_gParam.DcCtrlState = PreStart_Mode;
               }
               else
               {
                   DcDc_gParam.DcCtrlState = DcDcOff_Mode;
               }
               break;
           case PreStart_Mode://棰勯敓鏂ゆ嫹閿熸枻鎷锋ā寮�
               GPIO_writePin(19, 0);
               DcDc_gParam.RelaySt_DcBusSoft   = 0;
               DcDc_gParam.RelaySt_DcBatSoft   = 0;
               DcDc_gParam.RelaySt_DcBusMaster = 0;
               DcDc_gParam.RelaySt_DcBatMaster = 0;
               CtrlDelayCnt1 = 0;
               CtrlDelayCnt++;
               if(CtrlDelayCnt > 2)//閿熸枻鎷锋椂
               {
                   CtrlDelayCnt = 0;
                   if(DcDc_gParam.WorkMode == Mode_Buck) //降压模式
                   {
                       if(DC_OutMeter.DC_VBat_Mean > (float32_t)DcDc_Protect_Threshold.Bat_UnderVol_Value*0.1f)
                       {
                           DcDc_gParam.DcCtrlState = BatSoft_Mode;
                       }
                       else //娆犲帇閿熸枻鎷烽敓鏂ゆ嫹閿燂拷
                       {
                           FaultStatus.DCDC_Fault1.tbits.bVBatUnderFault = 1;//高压侧欠压故障
                           DcDc_gParam.DcCtrlState = DcDcOff_Mode;
                       }
                   }
                   else if(DcDc_gParam.WorkMode == Mode_Boost) //升压模式
                   {
                       if(DC_OutMeter.DC_VBus_Mean > (float32_t)DcDc_Protect_Threshold.Bus_UnderVol_Value*0.1f)
                       {
                           DcDc_gParam.DcCtrlState = BusSoft_Mode;
                       }
                       else //娆犲帇閿熸枻鎷烽敓鏂ゆ嫹閿燂拷
                       {
                           FaultStatus.DCDC_Fault1.tbits.bVBusUnderFault = 1;//閿熸枻鎷峰帇閿熸枻鎷锋瑺鍘嬮敓鏂ゆ嫹閿熸枻鎷�
                           DcDc_gParam.DcCtrlState = DcDcOff_Mode;
                       }
                   }
               }
               break;
           case BatSoft_Mode://閿熸枻鎷峰帇閿熸枻鎷烽敓鏂ゆ嫹
               GPIO_writePin(19, 0);
               DcDc_gParam.RelaySt_DcBusSoft   = 0;
               DcDc_gParam.RelaySt_DcBatSoft   = 1;
//               DcDc_gParam.RelaySt_DcBusMaster = 0;
               DcDc_gParam.RelaySt_DcBatMaster = 0;
//               CtrlDelayCnt++;
//               CtrlDelayCnt1 = 0;
//               if(CtrlDelayCnt > 3000)//閿熸枻鎷锋椂3S
//               {
//                 CtrlDelayCnt = 0;
//                 DcDc_gParam.DcCtrlState = BatStart_Mode;
//               }
               if(fabsf(DC_OutMeter.DC_VBat_Mean - DC_OutMeter.DC_VBatS_Mean) < 5.0f)//鍘嬮敓鏂ゆ嫹灏忛敓鏂ゆ嫹5V閿熸枻鎷锋椂1s閿熸枻鎷稤C閿熸枻鎷烽敓鏁欑鎷烽敓鏂ゆ嫹
               {
                   CtrlDelayCnt = 0;
                   CtrlDelayCnt1++;
                   if(CtrlDelayCnt1 > 1000)//閿熸枻鎷锋椂0.5S
                   {
                       CtrlDelayCnt1 = 0;
                       DcDc_gParam.DcCtrlState = BatStart_Mode;
                   }
               }
               else
               {
                   CtrlDelayCnt1 = 0;
                   if(CtrlDelayCnt > 25000)//閿熸枻鎷锋椂25S
                   {
                      CtrlDelayCnt = 0;
                      DcDc_gParam.DcCtrlState = DcDcOff_Mode;
                      FaultStatus.DCDC_Fault2.tbits.bBatRelyTimeOutFault = 1;//鐩撮敓鏂ゆ嫹姣嶉敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹鏃�
                      DcDc_gParam.StartEn  = 0;
                   }
               }
               break;
           case BatStart_Mode://鐩撮敓鏂ゆ嫹閿熸枻鎷烽敓鏁欑鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹
               GPIO_writePin(19, 0);
               DcDc_gParam.RelaySt_DcBusSoft   = 0;
               DcDc_gParam.RelaySt_DcBatSoft   = 1;
//               DcDc_gParam.RelaySt_DcBusMaster = 0;
               DcDc_gParam.RelaySt_DcBatMaster = 1;
               CtrlDelayCnt++;
               if(CtrlDelayCnt > 500)//閿熸枻鎷锋椂1S
               {
                  CtrlDelayCnt = 0;
                  if(DcDc_gParam.WorkMode == Mode_Boost) //Boost模式
                  {
                      DcDc_gParam.DcCtrlState = DcDcOn_Mode;
                  }
                  else if(DcDc_gParam.WorkMode == Mode_Buck) //Buck模式
                  {
                      DcDc_gParam.DcCtrlState = BusSoft_Mode;
                  }
               }
               break;
           case BusSoft_Mode: //低压侧软起动
               GPIO_writePin(19, 0);
               DcDc_gParam.RelaySt_DcBusSoft   = 1;
               DcDc_gParam.RelaySt_DcBatSoft   = 0;
               DcDc_gParam.RelaySt_DcBusMaster = 0;
//               DcDc_gParam.RelaySt_DcBatMaster = 0;
//               CtrlDelayCnt++;
//               CtrlDelayCnt1 = 0;
//               if(CtrlDelayCnt > 3000)//閿熸枻鎷锋椂3S
//               {
//                  CtrlDelayCnt = 0;
//                  DcDc_gParam.DcCtrlState = BusStart_Mode;
//               }
               if(fabsf(DC_OutMeter.DC_VBus_Mean - DC_OutMeter.DC_VBusS_Mean) < 5.0f)//鍘嬮敓鏂ゆ嫹灏忛敓鏂ゆ嫹5V閿熸枻鎷锋椂1s閿熸枻鎷稤C閿熸枻鎷烽敓鏁欑鎷烽敓鏂ゆ嫹
               {
                   CtrlDelayCnt = 0;
                   CtrlDelayCnt1++;
                   if(CtrlDelayCnt1 > 1000)//閿熸枻鎷锋椂0.5S
                   {
                       CtrlDelayCnt1 = 0;
                       DcDc_gParam.DcCtrlState = BusStart_Mode; //低压侧主启动
                   }
               }
               else
               {
                   CtrlDelayCnt1 = 0;
                   if(CtrlDelayCnt > 25000)//閿熸枻鎷锋椂25S
                   {
                      CtrlDelayCnt = 0;
                      DcDc_gParam.DcCtrlState = DcDcOff_Mode;
                      FaultStatus.DCDC_Fault2.tbits.bBusRelyTimeOutFault = 1;//鐩撮敓鏂ゆ嫹姣嶉敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹鏃�
                      DcDc_gParam.StartEn  = 0;
                   }
               }
               break;
           case BusStart_Mode: //低压侧主启动
               GPIO_writePin(19, 0);
               DcDc_gParam.RelaySt_DcBusSoft   = 1;
               DcDc_gParam.RelaySt_DcBatSoft   = 0;
               DcDc_gParam.RelaySt_DcBusMaster = 1;
//               DcDc_gParam.RelaySt_DcBatMaster = 0;
              CtrlDelayCnt++;
              if(CtrlDelayCnt > 500)//閿熸枻鎷烽敓鏂ゆ嫹鏃�
              {
                CtrlDelayCnt = 0;
                if(DcDc_gParam.WorkMode == Mode_Buck)//閿熸枻鎷峰帇妯″紡
                {
                    DcDc_gParam.DcCtrlState = DcDcOn_Mode;
                }
                else
                {
                    DcDc_gParam.DcCtrlState = BatSoft_Mode;
                }
              }
              break;
           case DcDcOn_Mode://閿熸枻鎷烽敓鏂ゆ嫹妯″紡
               GPIO_writePin(19, 0);
               DcDc_gParam.RelaySt_DcBusSoft   = 0;
               DcDc_gParam.RelaySt_DcBatSoft   = 0;
               DcDc_gParam.RelaySt_DcBusMaster = 1;
               DcDc_gParam.RelaySt_DcBatMaster = 1;
//               DcDc_gParam.Pwm_StartEn = 1;
               CtrlDelayCnt = 0;
              break;
           default:CtrlDelayCnt = 0;break;
       }
   }
}


//鐩撮敓鏂ゆ嫹閿熸枻鎷峰帇妯″紡閿熸枻鎷烽敓鏂ゆ嫹
/* CODEMAP_SOFTSTART_VOLT
 * Voltage-reference soft start. Ramps tCpu2Cla.DcDc_VdcRef toward DcConstVolSet.
 */
int16_t  objControl_ConstVolCtrl_SoftStar(void)
{
    static int16_t m_i16_DcVref = 0;

    if(tCpu2Cla.PwmStartEnable == 0)
    {
        if(DcDc_gParam.WorkMode == 1)//buck妯″紡
            m_i16_DcVref   = (int16_t)(DC_OutMeter.DC_VBatS_Mean*10.0f);//;
        else
            m_i16_DcVref   = 0;
    }
    else
    {
        if(m_st_TimerFlag.u16_b10ms == 1)
        {
            if(m_i16_DcVref == DcDc_gParam.DcConstVolSet)
            {
                m_i16_DcVref = DcDc_gParam.DcConstVolSet;
            }
            else if(m_i16_DcVref < DcDc_gParam.DcConstVolSet)
            {
                m_i16_DcVref += INV_SOFTSTART_STEP;

                if(m_i16_DcVref > DcDc_gParam.DcConstVolSet)
                {
                    m_i16_DcVref = DcDc_gParam.DcConstVolSet;
                }
            }
            else if(m_i16_DcVref > DcDc_gParam.DcConstVolSet)
            {
                m_i16_DcVref -= INV_SOFTSTART_STEP;

                if(m_i16_DcVref < DcDc_gParam.DcConstVolSet)
                {
                    m_i16_DcVref = DcDc_gParam.DcConstVolSet;
                }
            }
        }
    }

    tCpu2Cla.DcDc_VdcRef   = (float32_t)m_i16_DcVref*0.1f;

    return 0;
}

//閿熷鍔熼敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹
/* CODEMAP_SOFTSTART_POWER
 * Power-reference soft start intent. Check carefully during commissioning:
 * local ramp m_i16_PowerCurrRef is calculated, but tCpu2Cla.DcDc_PdcRef uses PowerPreSet directly.
 */
int16_t  objControl_ConstPowerCtrl_SoftStar(void)
{
    static int32_t m_i16_PowerCurrRef = 0;
    int32_t  PowerPreSet;

    PowerPreSet  = DcDc_gParam.DcConstPowerSet;//0.1kw
    if(tCpu2Cla.PwmStartEnable == 0)
    {
        m_i16_PowerCurrRef   = 0;
    }
    else
    {
        if(m_st_TimerFlag.u16_b10ms == 1)

        {
            if(m_i16_PowerCurrRef == PowerPreSet)
            {
                m_i16_PowerCurrRef = PowerPreSet;
            }
            else if(m_i16_PowerCurrRef < PowerPreSet)
            {
                m_i16_PowerCurrRef += 1;//1閿熻娇纭锋嫹0.1kw

                if(m_i16_PowerCurrRef > PowerPreSet)
                {
                    m_i16_PowerCurrRef = PowerPreSet;
                }
            }
            else if(m_i16_PowerCurrRef > PowerPreSet)
            {
                m_i16_PowerCurrRef -= 1;

                if(m_i16_PowerCurrRef < PowerPreSet)
                {
                    m_i16_PowerCurrRef = PowerPreSet;
                }
            }
        }
    }

    tCpu2Cla.DcDc_PdcRef     = (float32_t)PowerPreSet*100.0f;//w

    return 0;
}

//鐩撮敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹
/* CODEMAP_SOFTSTART_CURRENT
 * Current-reference soft start. Ramps tCpu2Cla.DcDc_IdcRef toward DcConstCurrSet.
 */
int16_t  objControl_ConstCurrCtrl_SoftStar(void)
{
    static int16_t m_i16_DcCurrRef = 0;

    if(tCpu2Cla.PwmStartEnable == 0)
    {
        m_i16_DcCurrRef   = 0;
    }
    else
    {
        if(m_st_TimerFlag.u16_b1ms == 1)
        {
            if(m_i16_DcCurrRef == DcDc_gParam.DcConstCurrSet)
            {
                m_i16_DcCurrRef = DcDc_gParam.DcConstCurrSet;
            }
            else if(m_i16_DcCurrRef < DcDc_gParam.DcConstCurrSet)
            {
                m_i16_DcCurrRef += INV_SOFTSTART_STEP;

                if(m_i16_DcCurrRef > DcDc_gParam.DcConstCurrSet)
                {
                    m_i16_DcCurrRef = DcDc_gParam.DcConstCurrSet;
                }
            }
            else if(m_i16_DcCurrRef > DcDc_gParam.DcConstCurrSet)
            {
                m_i16_DcCurrRef -= INV_SOFTSTART_STEP;

                if(m_i16_DcCurrRef < DcDc_gParam.DcConstCurrSet)
                {
                    m_i16_DcCurrRef = DcDc_gParam.DcConstCurrSet;
                }
            }
        }
    }

    tCpu2Cla.DcDc_IdcRef     = (float32_t)m_i16_DcCurrRef*0.1f;

    return 0;
}






