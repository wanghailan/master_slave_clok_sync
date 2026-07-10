//#############################################################################
//
// FILE:   PCS_FUN.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

/* CODEMAP_PROTECT_C
 * Role: software and optional hardware protection.
 * App_AllFault_Deal() is called before the state machine in main.c.
 * Any active fault sets globalFault, blocks DcOnAllowed and clears StartEn.
 */
#include <Driver/device/device.h>
#include <Driver/device/driverlib.h>
#include "bsp.h"

#define PCS_BOARD_PROTECTION_IINV_A    0
#define PCS_BOARD_PROTECTION_IINV_B    0 //B相电流2过流保护使能
#define PCS_BOARD_PROTECTION_IINV_C    0

#define PCS_BOARD_PROTECTION_IBATT     0 //直流电流过流保护使能
#define PCS_BOARD_PROTECTION_ESTOP     0 //急停输入作为硬件保护输入


#define PCS_IINV_B_CMPSS_BASE          CMPSS5_BASE  //B相电流2   CMPIN5P
#define PCS_IINV_B_XBAR_MUX            XBAR_MUX08
#define PCS_IINV_B_XBAR_MUX_VAL        XBAR_EPWM_MUX08_CMPSS5_CTRIPH_OR_L
#define PCS_IINV_B_XBAR_FLAG1          XBAR_INPUT_FLG_CMPSS5_CTRIPL
#define PCS_IINV_B_XBAR_FLAG2          XBAR_INPUT_FLG_CMPSS5_CTRIPH

#define PCS_IBATT_CMPSS_BASE           CMPSS1_BASE  //直流电流采样   CMPIN1P
#define PCS_IBATT_XBAR_MUX             XBAR_MUX00
#define PCS_IBATT_XBAR_MUX_VAL         XBAR_EPWM_MUX00_CMPSS1_CTRIPH_OR_L
#define PCS_IBATT_XBAR_FLAG1           XBAR_INPUT_FLG_CMPSS1_CTRIPL
#define PCS_IBATT_XBAR_FLAG2           XBAR_INPUT_FLG_CMPSS1_CTRIPH


#define PCS_IBATT_MAX_SENSE_AMPS       ((float32_t)220.0)
#define PCS_IBATT_TRIP_LIMIT_AMPS      ((float32_t)200.0)

#define PCS_IACP_HOVER_GPIO             69  //硬件交流电流正向过流
#define PCS_IACP_HOVER_PIN_CONFIG       GPIO_69_GPIO69
#define PCS_IACN_HOVER_GPIO             67  //硬件交流电流反向过流
#define PCS_IACN_HOVER_PIN_CONFIG       GPIO_67_GPIO67

#define PCS_ESTOP_GPIO                 133  //外部急停输入 低电平有效
#define PCS_ESTOP_GPIO_PIN_CONFIG      GPIO_133_GPIO133
#define PCS_ESTOP_XBAR_MUX             XBAR_MUX01
#define PCS_ESTOP_XBAR_MUX_VAL         XBAR_EPWM_MUX01_INPUTXBAR1
#define PCS_ESTOP_XBAR_FLAG            XBAR_INPUT_FLG_INPUT1

#define PCS_PROTECTION_ENABLED         1   //使能PWM TripZone
#define PCS_PROTECTION_DISABLED        0   //不使能PWM TripZone
#define PCS_PROTECTION                 PCS_PROTECTION_DISABLED
/* CODEMAP_PROTECTION_NOTE
 * Commissioning note: hardware TripZone protection is currently compiled as DISABLED.
 * Verify this before any power test; software protection alone may be too slow for short circuits.
 */

uint64_t  epwm1TZIntCount = 0;

static int16_t BatOverVol_errCnt = 0;//高压侧过压故障计数
static int16_t BusOverVol_errCnt = 0;//低压侧过压故障计数
static int16_t BatUnderVol_errCnt = 0;//高压欠压故障计数
static int16_t BusUnderVol_errCnt = 0;//低压欠压故障计数
static int16_t VolUnbalance_errCnt = 0;//电压不平衡故障计数
static int16_t BatOverCurr_errCnt = 0;//高压侧过流故障计数
static int16_t BusOverCurr_errCnt = 0;//低压侧过流故障计数
static int16_t CurrUnbalance_errCnt = 0;//电流不平衡故障计数
static int16_t IgbtOverTemp_errCnt = 0;//IGBT过温故障计数
static int16_t DC_Bus_Short_errCnt = 0;//IGBT过温故障计数
static int16_t DC_Bat_Short_errCnt = 0;//IGBT过温故障计数

//static int16_t ACFreq_errCnt = 0;//AC 频率故障计数
//static int16_t DCUnbalance_errCnt = 0;//DC 电压不平衡故障计数
//static int16_t DCOverVol_errCnt = 0;//DC过压故障计数
//static int16_t DCOverCurr_errCnt = 0;//DC过流故障计数
//static int16_t DCUnderVol_errCnt = 0;//DC欠压故障计数
//static int16_t OverTemp_errCnt = 0;//过温故障计数
//static int16_t PhaseSequence_errCnt = 0;

DCDC_Protect_Value     DcDc_Protect_Threshold;

////EPWM TripZone 配置为一次性关断保护状态
//void PCS_HAL_setupPWMforTrip(uint32_t base)
//{
//    // Trip 4 is the input to the DCAHCOMPSEL
//    EPWM_selectDigitalCompareTripInput(base,EPWM_DC_TRIP_TRIPIN4,EPWM_DC_TYPE_DCAH);//DCAH输入源为TRIPIN4
//    EPWM_setTripZoneDigitalCompareEventCondition(base,EPWM_TZ_DC_OUTPUT_A1,EPWM_TZ_EVENT_DCXH_HIGH);//DCAH高电平时触发 DCAEVT1
//
//    EPWM_setDigitalCompareEventSource(base,EPWM_DC_MODULE_A,EPWM_DC_EVENT_1,EPWM_DC_EVENT_SOURCE_ORIG_SIGNAL);
//    EPWM_setDigitalCompareEventSyncMode(base,EPWM_DC_MODULE_A,EPWM_DC_EVENT_1,EPWM_DC_EVENT_INPUT_NOT_SYNCED);
//
//    // What do we want the OST / CBC events to do?
//    // TZA events can force EPWMxA
//    // TZB events can force EPWMxB
//    EPWM_setTripZoneAction(base, EPWM_TZ_ACTION_EVENT_TZA, EPWM_TZ_ACTION_LOW);
//    EPWM_setTripZoneAction(base, EPWM_TZ_ACTION_EVENT_TZB, EPWM_TZ_ACTION_LOW);
//
//    // Enable the following trips - DCAEVT1
//    EPWM_enableTripZoneSignals(base, EPWM_TZ_SIGNAL_DCAEVT1);
//
//    // Clear any spurious DCAEVT1 flags
//    EPWM_clearTripZoneFlag(base, EPWM_TZ_FLAG_DCAEVT1);
//    EPWM_clearTripZoneFlag(base, EPWM_TZ_FLAG_OST);
//
//    // Force a trip on PWM to safely start the system
////    EPWM_forceTripZoneEvent(base, EPWM_TZ_FLAG_OST);
//
//}

//EPWM TripZone 配置为CBC逐波限流保护状态，同时配置EPWM1 TripZone中断，中断次数超过2个50HZ周波时，报故障彻底关断
/* CODEMAP_PWM_TRIP_SETUP: configure ePWM TripZone action for cycle-by-cycle current limit. */
void PCS_HAL_setupPWMforTrip(uint32_t base)
{
    // Trip 4 is the input to the DCAHCOMPSEL
    EPWM_selectDigitalCompareTripInput(base,EPWM_DC_TRIP_TRIPIN4,EPWM_DC_TYPE_DCAH);//DCAH输入源为TRIPIN4
    EPWM_setTripZoneDigitalCompareEventCondition(base,EPWM_TZ_DC_OUTPUT_A2,EPWM_TZ_EVENT_DCXH_HIGH);//DCAH高电平时触发 DCAEVT2

//    EPWM_setDigitalCompareEventSource(base,EPWM_DC_MODULE_A,EPWM_DC_EVENT_2,EPWM_DC_EVENT_SOURCE_ORIG_SIGNAL);
//    EPWM_setDigitalCompareEventSyncMode(base,EPWM_DC_MODULE_A,EPWM_DC_EVENT_2,EPWM_DC_EVENT_INPUT_NOT_SYNCED);

    // What do we want the OST / CBC events to do?
    // TZA events can force EPWMxA
    // TZB events can force EPWMxB
    EPWM_setTripZoneAction(base, EPWM_TZ_ACTION_EVENT_TZA, EPWM_TZ_ACTION_LOW);
    EPWM_setTripZoneAction(base, EPWM_TZ_ACTION_EVENT_TZB, EPWM_TZ_ACTION_LOW);

    // Enable the following trips - DCAEVT2
    EPWM_enableTripZoneSignals(base, EPWM_TZ_SIGNAL_DCAEVT2);
    //使能EPWM1的CBC中断，用于计数逐波限流次数
//    if(base == EPWM1_BASE)
//        EPWM_enableTripZoneInterrupt(base,EPWM_TZ_INTERRUPT_CBC);

    EPWM_selectCycleByCycleTripZoneClearEvent(base,EPWM_TZ_CBC_PULSE_CLR_CNTR_ZERO);
    // Clear any spurious DCAEVT2 flags
    EPWM_clearTripZoneFlag(base, EPWM_TZ_FLAG_DCAEVT2);
    EPWM_clearTripZoneFlag(base, EPWM_TZ_FLAG_CBC);//配置为Cycle-By-Cycle模式

    // Force a trip on PWM to safely start the system
//    EPWM_forceTripZoneEvent(base, EPWM_TZ_FLAG_OST);

}

//void PCS_HAL_setupCMPSS(uint32_t base1,float32_t current_limit,float32_t current_max_sense)
void PCS_HAL_setupCMPSS(uint32_t base1,uint16_t current_max,uint16_t current_min)
{
    // Use VDDA as the reference for comparator DACs
    CMPSS_configDAC(base1,
                    CMPSS_DACVAL_SYSCLK |
                    CMPSS_DACREF_VDDA |
                    CMPSS_DACSRC_SHDW);

    // Set DAC to H~75% and L ~25% values  正向和反向电流的保护值
//    CMPSS_setDACValueHigh(base1, 2048 + (int16_t)((float32_t)current_limit *
//                          (float32_t)2048.0f / (float32_t)current_max_sense));
//    CMPSS_setDACValueLow(base1, 2048 - (int16_t)((float32_t)current_limit *
//                         (float32_t)2048.0f / (float32_t)current_max_sense));
//    CMPSS_setDACValueHigh(base1, 3692);//3.1875V:300A,125kW:189A rms:268A pp,3.1875/300*280=2.975, 3692=2.975*4096/3.3
//    CMPSS_setDACValueLow(base1, 1954);//1.6875V:-300A,125kW:189A rms:268A pp,1.6875/300*280=1.575, 1954=1.575*4096/3.3
    CMPSS_setDACValueHigh(base1, current_max);
    CMPSS_setDACValueLow(base1, current_min);

    //
    // Make sure the asynchronous path compare high and low event
    // does not go to the OR gate with latched digital filter output
    // hence no additional parameter CMPSS_OR_ASYNC_OUT_W_FILT  is passed
    // comparator oputput is "not" inverted for high compare event
    //
    CMPSS_configHighComparator(base1, CMPSS_INSRC_DAC);

    // Comparator output is inverted for for low compare event
    CMPSS_configLowComparator(base1, CMPSS_INSRC_DAC | CMPSS_INV_INVERTED);//

    CMPSS_configFilterHigh(base1, 2, 30, 20); //10 to 30; 7 to 20
    CMPSS_configFilterLow(base1, 2, 30, 20);

    // Reset filter logic & start filtering
    CMPSS_initFilterHigh(base1);
    CMPSS_initFilterLow(base1);

    // Configure CTRIPOUT path
    CMPSS_configOutputsHigh(base1, CMPSS_TRIP_FILTER | CMPSS_TRIP_FILTER);
    CMPSS_configOutputsLow(base1,  CMPSS_TRIP_FILTER | CMPSS_TRIP_FILTER);

    // Comparator hysteresis control , set to 2x typical value
    CMPSS_setHysteresis(base1, 2);

    // Enable CMPSSx
    CMPSS_enableModule(base1);
    // Delay for CMPSS DAC to power up.
    DEVICE_DELAY_US(500);

    // Clear the latched comparator events
    CMPSS_clearFilterLatchHigh(base1);
    CMPSS_clearFilterLatchLow(base1);
}


//
//
////
//// epwm1TZISR - ePWM1 TZ ISR
////
//uint32_t  isrStarttime,isrEndtime;
//uint32_t  isrAllTime;
//__interrupt void epwm1TZISR(void)
//{
//    epwm1TZIntCount++;
//    if(epwm1TZIntCount == 1)
//        isrStarttime = m_u32_TimerCnt;
//
//        isrEndtime = m_u32_TimerCnt;
//        isrAllTime = isrEndtime - isrStarttime;
////    if(epwm1TZIntCount > 25000)//PWM计数等于0时清CBC，62.5us清一次，2个周波=40000/62.5=640
////        epwm1TZIntCount = 25000;
//
//    if(epwm1TZIntCount == 640)
//    {
//        Drv_PwmOffset();
//    }
//
//    // To re-enable the OST Interrupt, uncomment the below code:
//
//
////    // Acknowledge this interrupt to receive more interrupts from group 2
//    XBAR_clearInputFlag(PCS_ESTOP_XBAR_FLAG);
//    EPWM_clearTripZoneFlag(EPWM1_BASE, EPWM_TZ_FLAG_DCAEVT2);
//    EPWM_clearTripZoneFlag(EPWM1_BASE, (EPWM_TZ_INTERRUPT | EPWM_TZ_FLAG_CBC));//
//    EPWM_clearCycleByCycleTripZoneFlag(EPWM1_BASE, EPWM_TZ_CBC_FLAG_DCAEVT2);
//    EPWM_clearTripZoneFlag(EPWM2_BASE, EPWM_TZ_FLAG_CBC);//
//    EPWM_clearTripZoneFlag(EPWM3_BASE, EPWM_TZ_FLAG_CBC);//
//    EPWM_clearTripZoneFlag(EPWM4_BASE, EPWM_TZ_FLAG_CBC);//
//    EPWM_clearTripZoneFlag(EPWM5_BASE, EPWM_TZ_FLAG_CBC);//
//    EPWM_clearTripZoneFlag(EPWM6_BASE, EPWM_TZ_FLAG_CBC);//
//    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP2);
//}

void EPWM_CBCTripZoneInt_Counter(void)
{
      if(EPWM_getCycleByCycleTripZoneFlagStatus(EPWM1_BASE) == EPWM_TZ_CBC_FLAG_DCAEVT2)
      {
          EPWM_clearTripZoneFlag(EPWM1_BASE, (EPWM_TZ_FLAG_CBC | EPWM_TZ_FLAG_DCAEVT2));//
          EPWM_clearCycleByCycleTripZoneFlag(EPWM1_BASE, EPWM_TZ_CBC_FLAG_DCAEVT2);
          EPWM_clearTripZoneFlag(EPWM2_BASE, (EPWM_TZ_FLAG_CBC | EPWM_TZ_FLAG_DCAEVT2));//
          EPWM_clearCycleByCycleTripZoneFlag(EPWM2_BASE, EPWM_TZ_CBC_FLAG_DCAEVT2);
          EPWM_clearTripZoneFlag(EPWM3_BASE, (EPWM_TZ_FLAG_CBC | EPWM_TZ_FLAG_DCAEVT2));//
          EPWM_clearCycleByCycleTripZoneFlag(EPWM3_BASE, EPWM_TZ_CBC_FLAG_DCAEVT2);
          EPWM_clearTripZoneFlag(EPWM4_BASE, (EPWM_TZ_FLAG_CBC | EPWM_TZ_FLAG_DCAEVT2));//
          EPWM_clearCycleByCycleTripZoneFlag(EPWM4_BASE, EPWM_TZ_CBC_FLAG_DCAEVT2);
          EPWM_clearTripZoneFlag(EPWM5_BASE, (EPWM_TZ_FLAG_CBC | EPWM_TZ_FLAG_DCAEVT2));//
          EPWM_clearCycleByCycleTripZoneFlag(EPWM5_BASE, EPWM_TZ_CBC_FLAG_DCAEVT2);
          EPWM_clearTripZoneFlag(EPWM6_BASE, (EPWM_TZ_FLAG_CBC | EPWM_TZ_FLAG_DCAEVT2));//
          EPWM_clearCycleByCycleTripZoneFlag(EPWM6_BASE, EPWM_TZ_CBC_FLAG_DCAEVT2);
          epwm1TZIntCount++;
          if(epwm1TZIntCount > 640)//31.25us中断一次，连续2个周波触发逐波限流，则停止发波
          {
              epwm1TZIntCount = 640;
              Drv_PwmOffset();
          }
//          if(epwm1TZIntCount == 1)
//              isrStarttime = m_u32_TimerCnt;
//
//          isrEndtime = m_u32_TimerCnt;
//          isrAllTime = isrEndtime - isrStarttime;
      }
}

//*****************************************************************************
//
////*****************************************************************************
////
//// INTERRUPT Configurations
////
////*****************************************************************************
//void InterruptEpwmCBC_init(void)
//{
//    // Interrupt Setings for INT_EPWM1_TZ
//    Interrupt_register(INT_EPWM1_TZ, &epwm1TZISR);
//    Interrupt_enable(INT_EPWM1_TZ);
//}

//逐波限流设置
/* CODEMAP_CBC_SETUP: optional CMPSS/XBAR/ePWM hardware protection wiring. Controlled by macros above. */
void PCS_HAL_setupCBCProtection()
{
    //
    // Disable all the muxes first
    //
    XBAR_disableEPWMMux(XBAR_TRIP4, 0xFF);

    //模块2 B相电流 过流硬件保护
#if PCS_BOARD_PROTECTION_IINV_B == 1

//    CMPSS_setDACValueHigh(base1, 3692);//3.1875V:300A,125kW:189A rms:268A pp,3.1875/300*280=2.975, 3692=2.975*4096/3.3
//    CMPSS_setDACValueLow(base1, 1954);//1.6875V:-300A,125kW:189A rms:268A pp,1.6875/300*280=1.575, 1954=1.575*4096/3.3
    PCS_HAL_setupCMPSS(PCS_IINV_B_CMPSS_BASE,3692,1954);

    XBAR_setEPWMMuxConfig(XBAR_TRIP4, PCS_IINV_B_XBAR_MUX_VAL);
    XBAR_enableEPWMMux(XBAR_TRIP4, PCS_IINV_B_XBAR_MUX);
    XBAR_clearInputFlag(PCS_IINV_B_XBAR_FLAG1);
    XBAR_clearInputFlag(PCS_IINV_B_XBAR_FLAG2);

#endif

//DC过流 过流硬件保护
#if PCS_BOARD_PROTECTION_IBATT == 1
//    CMPSS_setDACValueHigh(base1, 3692);//2.5V:300A,125kW:208A ,2.5/300*210=1.75, 2172=1.75*4096/3.3
//    CMPSS_setDACValueLow(base1, 1954);//0.5V:-300A,125kW:208A,0.5/300*210=1.575, 1954=1.575*4096/3.3
    PCS_HAL_setupCMPSS(PCS_IBATT_CMPSS_BASE,
                        2172,
                        1920);

    XBAR_setEPWMMuxConfig(XBAR_TRIP4, PCS_IBATT_XBAR_MUX_VAL);
    XBAR_enableEPWMMux(XBAR_TRIP4, PCS_IBATT_XBAR_MUX);
    XBAR_clearInputFlag(PCS_IBATT_XBAR_FLAG1);
    XBAR_clearInputFlag(PCS_IBATT_XBAR_FLAG2);

#endif



#if PCS_BOARD_PROTECTION_ESTOP== 1
    GPIO_setDirectionMode(PCS_ESTOP_GPIO, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(PCS_ESTOP_GPIO, GPIO_QUAL_SYNC);
    GPIO_setPinConfig(PCS_ESTOP_GPIO_PIN_CONFIG);
    GPIO_setPadConfig(PCS_ESTOP_GPIO, GPIO_PIN_TYPE_STD);//DI低电平有效，TripZone是高电平触发 GPIO_PIN_TYPE_INVERT |

    XBAR_setInputPin(INPUTXBAR_BASE,XBAR_INPUT1, PCS_ESTOP_GPIO);
    XBAR_setEPWMMuxConfig(XBAR_TRIP4, PCS_ESTOP_XBAR_MUX_VAL);
    XBAR_enableEPWMMux(XBAR_TRIP4, PCS_ESTOP_XBAR_MUX);
    XBAR_clearInputFlag(PCS_ESTOP_XBAR_FLAG);
#endif

//    XBAR_invertEPWMSignal(XBAR_TRIP4,false);

//逐波限流EPWM TripZone使能
#if PCS_PROTECTION == PCS_PROTECTION_ENABLED
    InterruptEpwmCBC_init();
    PCS_HAL_setupPWMforTrip(DCP1_Q1_Q2_A_PWM_BASE);
    PCS_HAL_setupPWMforTrip(DCP1_Q3_Q4_A_PWM_BASE);
    PCS_HAL_setupPWMforTrip(DCP2_Q1_Q2_A_PWM_BASE);
    PCS_HAL_setupPWMforTrip(DCP2_Q3_Q4_A_PWM_BASE);
//    PCS_HAL_setupPWMforTrip(PCS_Q1_Q3_C_PWM_BASE);
//    PCS_HAL_setupPWMforTrip(PCS_Q2_Q4_C_PWM_BASE);

#endif

}

//高压过压故障
/* CODEMAP_FAULT_MONITORS
 * The functions below are individual fault monitors. Most voltage/temp faults use 10 ms timing
 * and a counter debounce. Some current faults currently run without an explicit timer guard.
 */
static void FaultMonitor_BatVoltageOver(void)
{
    float32_t  OverVol_Value = (float32_t)DcDc_Protect_Threshold.Bat_OverVol_Value*0.1f;

    if(m_st_TimerFlag.u16_b10ms == 1)
    {
        //母线过压
        if(DC_OutMeter.DC_VBat_Mean > OverVol_Value)
        {
            BatOverVol_errCnt++;
        }
        else if(DC_OutMeter.DC_VBat_Mean < OverVol_Value-10.0f)
        {
            if(BatOverVol_errCnt > 0)
                BatOverVol_errCnt--;
            else
                BatOverVol_errCnt = 0;
        }

        if(BatOverVol_errCnt >= 100)
        {
            BatOverVol_errCnt = 100;
            FaultStatus.DCDC_Fault1.tbits.bVBatOverFault = 1;
        }
    }
}

//低压过压故障
static void FaultMonitor_BusVoltageOver(void)
{
    float32_t  OverVol_Value = (float32_t)DcDc_Protect_Threshold.Bus_OverVol_Value*0.1f;

    if(m_st_TimerFlag.u16_b10ms == 1)
    {
        //母线过压
        if(DC_OutMeter.DC_VBus_Mean > OverVol_Value)
        {
            BusOverVol_errCnt++;
        }
        else if(DC_OutMeter.DC_VBus_Mean < OverVol_Value-10.0f)
        {
            if(BusOverVol_errCnt > 0)
                BusOverVol_errCnt--;
            else
                BusOverVol_errCnt = 0;
        }

        if(BusOverVol_errCnt >= 100)
        {
            BusOverVol_errCnt = 100;
            FaultStatus.DCDC_Fault1.tbits.bVBusOverFault = 1;
        }
    }
}


//高压欠压故障
static void FaultMonitor_BatVoltageUnder(void)
{
    float32_t  UnderVol_Value = (float32_t)DcDc_Protect_Threshold.Bat_UnderVol_Value*0.1f;

    if(m_st_TimerFlag.u16_b10ms == 1)
    {
        //DC欠压
        if(DC_OutMeter.DC_VBat_Mean < UnderVol_Value)
        {
            BatUnderVol_errCnt++;
        }
        else if(DC_OutMeter.DC_VBat_Mean > UnderVol_Value+20.0f)
        {
            if(BatUnderVol_errCnt > 0)
                BatUnderVol_errCnt--;
            else
                BatUnderVol_errCnt = 0;
        }

        if(BatUnderVol_errCnt >= 100)
        {
            BatUnderVol_errCnt = 100;
            FaultStatus.DCDC_Fault1.tbits.bVBatUnderFault = 1;
        }
    }
}

//低压欠压故障
static void FaultMonitor_BusVoltageUnder(void)
{
    float32_t  UnderVol_Value = (float32_t)DcDc_Protect_Threshold.Bus_UnderVol_Value*0.1f;

    if(m_st_TimerFlag.u16_b10ms == 1)
    {
        //DC欠压
        if(DC_OutMeter.DC_VBus_Mean < UnderVol_Value)
        {
            BusUnderVol_errCnt++;
        }
        else if(DC_OutMeter.DC_VBus_Mean > UnderVol_Value+20.0f)
        {
            if(BusUnderVol_errCnt > 0)
                BusUnderVol_errCnt--;
            else
                BusUnderVol_errCnt = 0;
        }

        if(BusUnderVol_errCnt >= 100)
        {
            BusUnderVol_errCnt = 100;
            FaultStatus.DCDC_Fault1.tbits.bVBusUnderFault = 1;
        }
    }
}

//DC电压不平衡故障
static void FaultMonitor_VoltageUnbanlance(void)
{
    float32_t  UnbanceVol_Value = (float32_t)DcDc_Protect_Threshold.Vol_Unbance_Value*0.1f;

    if(m_st_TimerFlag.u16_b10ms == 1)
    {
        //DC不平衡
        if(fabsf(DC_OutMeter.DC_VBatP_Mean-DC_OutMeter.DC_VBatN_Mean) > UnbanceVol_Value)
        {
            VolUnbalance_errCnt++;
        }
        else if(fabsf(DC_OutMeter.DC_VBatP_Mean-DC_OutMeter.DC_VBatN_Mean) < (UnbanceVol_Value-10.0f))
        {
            if(VolUnbalance_errCnt > 0)
                VolUnbalance_errCnt--;
            else
                VolUnbalance_errCnt = 0;
        }

        if(VolUnbalance_errCnt >= 100)
        {
            VolUnbalance_errCnt = 100;
            FaultStatus.DCDC_Fault1.tbits.bVUnBanlanFault = 1;
        }
    }
}

//高压过流故障
static  void FaultMonitor_BatCurrentOver(void)
{
    float32_t  OverCurr_Value = (float32_t)DcDc_Protect_Threshold.Bat_OverCurr_Value*0.1f;

//       if(m_st_TimerFlag.u16_b10ms == 1)
       {
           if(DC_OutMeter.DC_IBat_Mean > OverCurr_Value)
           {
               BatOverCurr_errCnt++;
           }
           else if(DC_OutMeter.DC_IBat_Mean < OverCurr_Value-10.0f)
           {
               if(BatOverCurr_errCnt > 0)
                   BatOverCurr_errCnt--;
               else
                   BatOverCurr_errCnt = 0;
           }

           if(BatOverCurr_errCnt >= 100)
           {
               BatOverCurr_errCnt = 100;
               FaultStatus.DCDC_Fault1.tbits.bIBatOverFault = 1;
           }
       }
}

//低压过流故障
static  void FaultMonitor_BusCurrentOver(void)
{
    float32_t  OverCurr_Value = (float32_t)DcDc_Protect_Threshold.Bus_OverCurr_Value*0.1f;

//       if(m_st_TimerFlag.u16_b10ms == 1)
       {
           if(DC_OutMeter.DC_IBus_Mean > OverCurr_Value)
           {
               BusOverCurr_errCnt++;
           }
           else if(DC_OutMeter.DC_IBus_Mean < OverCurr_Value-10.0f)
           {
               if(BusOverCurr_errCnt > 0)
                   BusOverCurr_errCnt--;
               else
                   BusOverCurr_errCnt = 0;
           }

           if(BusOverCurr_errCnt >= 100)
           {
               BusOverCurr_errCnt = 100;
               FaultStatus.DCDC_Fault1.tbits.bIBusOverFault = 1;
           }
       }
}

static void FaultMonitor_CurrUnbanlance(void)
{
    float32_t  UnbanceCurr_Value = (float32_t)DcDc_Protect_Threshold.Curr_Unbance_Value*0.1f;

//    if(m_st_TimerFlag.u16_b10ms == 1)
    {
        //DC不平衡
        if(fabsf(DC_OutMeter.DC_IL1_Mean-DC_OutMeter.DC_IL2_Mean) > UnbanceCurr_Value || fabsf(DC_OutMeter.DC_IL2_Mean-DC_OutMeter.DC_IL3_Mean) > UnbanceCurr_Value || fabsf(DC_OutMeter.DC_IL1_Mean-DC_OutMeter.DC_IL3_Mean) > UnbanceCurr_Value)
        {
            CurrUnbalance_errCnt++;
        }
        else if(fabsf(DC_OutMeter.DC_IL1_Mean-DC_OutMeter.DC_IL2_Mean) < (UnbanceCurr_Value-10.0f) && fabsf(DC_OutMeter.DC_IL2_Mean-DC_OutMeter.DC_IL3_Mean) < (UnbanceCurr_Value-10.0f) && fabsf(DC_OutMeter.DC_IL1_Mean-DC_OutMeter.DC_IL3_Mean) < (UnbanceCurr_Value-10.0f))
        {
            if(CurrUnbalance_errCnt > 0)
                CurrUnbalance_errCnt--;
            else
                CurrUnbalance_errCnt = 0;
        }

        if(CurrUnbalance_errCnt >= 100)
        {
            CurrUnbalance_errCnt = 100;
            FaultStatus.DCDC_Fault1.tbits.bIUnBanlanFault = 1;
        }
    }
}

//IGBT过温故障
static void FaultMonitor_IgbtOverTemp(void)
{
    float32_t  OverTemp_Value = (float32_t)DcDc_Protect_Threshold.Temp_Over_Value*0.1f;

    if(m_st_TimerFlag.u16_b10ms == 1)
    {
        if(DC_OutMeter.DC_IGBT_TempMax > OverTemp_Value)
        {
            IgbtOverTemp_errCnt++;
        }
        else if(DC_OutMeter.DC_IGBT_TempMax < (OverTemp_Value - 5.0f))
        {
            if(IgbtOverTemp_errCnt > 0)
                IgbtOverTemp_errCnt--;
            else
                IgbtOverTemp_errCnt = 0;
        }

        if(IgbtOverTemp_errCnt >= 100)
        {
            IgbtOverTemp_errCnt = 100;
            FaultStatus.DCDC_Fault1.tbits.bIgbtOverTempFault = 1;
        }
    }
}

void FaultMonitor_FanFault(void)
{
//    static int16_t FanFault_errCnt = 0;//风扇故障计数
//    if(m_st_TimerFlag.u16_b50ms == 1)
//    {
//        if(0==Drv_FanStatusGet())
//        {
//            FanFault_errCnt++;
//        }
//        else
//        {
//            if(FanFault_errCnt > 0)
//                FanFault_errCnt--;
//            else
//                FanFault_errCnt = 0;
//        }
//
//        if(FanFault_errCnt >= 10)
//        {
//            FanFault_errCnt = 10;
//            FaultStatus.PCS_Fault3.tbits.bFanFault = 1;
//        }
//        else
//        {
//            FaultStatus.PCS_Fault3.tbits.bFanFault = 0;
//        }
//    }
}

static void FaultMonitor_EmergStop(void)
{
    static int16_t EstopFault_errCnt = 0;//IGBT故障计数
//    if(m_st_TimerFlag.u16_b50ms == 1)
    {
        if(0==tca9535_io1_p1Get(1))
        {
            EstopFault_errCnt++;
        }
        else
        {
            if(EstopFault_errCnt > 0)
                EstopFault_errCnt--;
            else
                EstopFault_errCnt = 0;
        }

        if(EstopFault_errCnt >= 10)
        {
            EstopFault_errCnt = 10;
            FaultStatus.DCDC_Fault2.tbits.bEmergencyStop = 1;
        }
    }
}

static void FaultMonitor_FaultIgbt(void)
{
    static int16_t IgbtFault_errCnt = 0;//IGBT故障计数
//    if(m_st_TimerFlag.u16_b50ms == 1)
    {
        if(0==Drv_FigbtStatusGet())
        {
            IgbtFault_errCnt++;
        }
        else
        {
            if(IgbtFault_errCnt > 0)
                IgbtFault_errCnt--;
            else
                IgbtFault_errCnt = 0;
        }

        if(IgbtFault_errCnt >= 10)
        {
            IgbtFault_errCnt = 10;
            FaultStatus.DCDC_Fault2.tbits.bIGBTFault = 1;
        }
    }
}

void Fault_Init(void)//极性反接故障
{
    static int16_t DCVolRP_errCnt = 0;//极性反接故障计数
    if(m_st_TimerFlag.u16_b10ms == 1)
    {
        if(DC_OutMeter.DC_Vdc7_Mean < -15.0f || DC_OutMeter.DC_Vdc8_Mean < -15.0f)
        {
            DCVolRP_errCnt++;
        }
        else if(DC_OutMeter.DC_Vdc7_Mean >= 0.0f && DC_OutMeter.DC_Vdc8_Mean >= 0.0f)
        {
            if(DCVolRP_errCnt > 0)
                DCVolRP_errCnt--;
            else
                DCVolRP_errCnt = 0;
        }
        if(DCVolRP_errCnt >= 100)
        {
            DCVolRP_errCnt = 100;
            FaultStatus.DCDC_Fault4.tbits.DCRPFault = 1;
        }
    }
}
static void FaultMonitor_Short(void)
{
    float32_t  BusShort_Value = (float32_t)DcDc_Protect_Threshold.Bus_VShort_Value*0.1f;
    float32_t  BatShort_Value = (float32_t)DcDc_Protect_Threshold.Bat_VShort_Value*0.1f;

    if (DcDc_gParam.CtrlMode == 1 || DcDc_gParam.CtrlMode == 0)
    {
        if (DC_OutMeter.DC_IBus_Mean > 230.0f)
        {
            DC_Bus_Short_errCnt++;
        }
        if (DC_OutMeter.DC_IBat_Mean > 165.0f)
        {
            DC_Bat_Short_errCnt++;
        }
        if(DC_Bus_Short_errCnt >= 100)
        {
            DC_Bus_Short_errCnt = 100;
            FaultStatus.DCDC_Fault4.tbits.DC_Bus_ShortFault = 1;
        }
        if(DC_Bat_Short_errCnt >= 100)
        {
            DC_Bat_Short_errCnt = 100;
            FaultStatus.DCDC_Fault4.tbits.DC_Bat_ShortFault = 1;
        }
    }
    else
    {
        if (DC_OutMeter.DC_VBus_Mean < BusShort_Value || DC_OutMeter.DC_IBus_Mean > 740.0f)
        {
            DC_Bus_Short_errCnt++;
        }
        if (DC_OutMeter.DC_VBat_Mean < BatShort_Value || DC_OutMeter.DC_IBat_Mean > 480.0f)
        {
            DC_Bat_Short_errCnt++;
        }
        if(DC_Bus_Short_errCnt >= 100)
        {
            DC_Bus_Short_errCnt = 100;
            FaultStatus.DCDC_Fault4.tbits.DC_Bus_ShortFault = 1;
        }
        if(DC_Bat_Short_errCnt >= 100)
        {
            DC_Bat_Short_errCnt = 100;
            FaultStatus.DCDC_Fault4.tbits.DC_Bat_ShortFault = 1;
        }
    }
}

//static void FaultMonitor_BatShort(void)
//{
//    float32_t  BatShort_Value = (float32_t)DcDc_Protect_Threshold.Bat_Short_Value*0.1f;
//    if (DcDc_gParam.WorkMode == Mode_Boost)
//    {
//        if(DC_OutMeter.DC_VBat_Mean < BatShort_Value || DC_OutMeter.DC_IBat_Mean > 480.0f)
//        {
//            BatUnderVol_errCnt++;
//        }
//        else if(DC_OutMeter.DC_VBat_Mean > BatShort_Value + 10.0f)
//        {
//            if(BatUnderVol_errCnt > 0)
//                BatUnderVol_errCnt--;
//            else
//                BatUnderVol_errCnt = 0;
//        }
//
//        if(BatUnderVol_errCnt >= 100)
//        {
//            BatUnderVol_errCnt = 100;
//            FaultStatus.DCDC_Fault4.tbits.DC_BatShortFault = 1;
//        }
//    }
//}

//故障处理
/* CODEMAP_ALL_FAULT_DEAL
 * Fault aggregator. It calls monitor functions, sets globalFault if any fault word is nonzero,
 * then disables startup by DcOnAllowed=0 and StartEn=0.
 */
int16_t App_AllFault_Deal(void)
{
//    if(m_st_TimerFlag.u16_b1ms == 0)
//        return 0;
//      FaultMonitor_FanFault();
        FaultMonitor_EmergStop();
        FaultMonitor_FaultIgbt();
//   if(Pcs_gParam.PcsCtrlState == PcsOn_Mode)
   {
        FaultMonitor_BatVoltageOver();
        FaultMonitor_BusVoltageOver();
        if (DcDc_gParam.StartEn)
        {
//            FaultMonitor_BatVoltageUnder();
//            FaultMonitor_BusVoltageUnder();
            FaultMonitor_Short();
        }
//        FaultMonitor_VoltageUnbanlance();
        FaultMonitor_BatCurrentOver();
        FaultMonitor_BusCurrentOver();
        FaultMonitor_CurrUnbanlance();
        FaultMonitor_IgbtOverTemp();
        Fault_Init();
//        if(SCIB_RxTimeOut > 100000)//超过10S没有收到光纤板数据报通信故障
//        {
//            FaultStatus.DCDC_Fault3.tbits.bFiberIgbtFault = 1;
//        }
        if((FaultStatus.DCDC_Fault1.DCDC_Fault1_All != 0)||
           (FaultStatus.DCDC_Fault2.DCDC_Fault2_All != 0)||
           (FaultStatus.DCDC_Fault3.DCDC_Fault3_All != 0)||
           (FaultStatus.DCDC_Fault4.DCDC_Fault4_All != 0))
            FaultStatus.globalFault = 1;
   }
//有故障时封波，停止输出
   if(FaultStatus.globalFault == 1)
   {
       DcDc_gParam.DcOnAllowed = 0;
        if(DcDc_gParam.StartEn == 1)
        {
            DcDc_gParam.StartEn = 0;
        }
   }

    return 0;
}
