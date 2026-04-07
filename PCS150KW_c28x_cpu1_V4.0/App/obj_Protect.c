//#############################################################################
//
// FILE:   PCS_FUN.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

#include <Driver/device/device.h>
#include <Driver/device/driverlib.h>
#include "bsp.h"

//内部比较器cbc模式
#define PCS_PROTECTION_IINV_A    0 //A相电流过流保护使能
#define PCS_PROTECTION_IINV_B    0 //B相电流过流保护使能
#define PCS_PROTECTION_IINV_C    0 //C相电流过流保护使能
#define PCS_PROTECTION_IBATT     0 //直流电流过流保护使能

#define PCS_PROTECTION_IDCN      0 //DC中线电流过流保护使能
#define PCS_PROTECTION_VDCP      0 //DC电压过压保护使能
#define PCS_PROTECTION_VDC       0 //DC正向电压过压保护使能

//GPIO输入外部比较结果 低电平有效 ost模式
#define PCS_PROTECTION_IACP_HAL  0 //交流正向过流使能
#define PCS_PROTECTION_IACN_HAL  0 //交流负向过流使能
#define PCS_PROTECTION_IDC_HAL   0 //直流电流过流使能
#define PCS_PROTECTION_ESTOP     0 //急停输入作为硬件保护输入

#define PCS_IINV_A_CMPSS_BASE          CMPSS8_BASE  //A相电流2   CMPIN84P
#define PCS_IINV_A_XBAR_MUX            XBAR_MUX14
#define PCS_IINV_A_XBAR_MUX_VAL        XBAR_EPWM_MUX14_CMPSS8_CTRIPH_OR_L
#define PCS_IINV_A_XBAR_FLAG1          XBAR_INPUT_FLG_CMPSS8_CTRIPL
#define PCS_IINV_A_XBAR_FLAG2          XBAR_INPUT_FLG_CMPSS8_CTRIPH

#define PCS_IINV_B_CMPSS_BASE          CMPSS6_BASE  //B相电流2   CMPIN6P
#define PCS_IINV_B_XBAR_MUX            XBAR_MUX10
#define PCS_IINV_B_XBAR_MUX_VAL        XBAR_EPWM_MUX10_CMPSS6_CTRIPH_OR_L
#define PCS_IINV_B_XBAR_FLAG1          XBAR_INPUT_FLG_CMPSS6_CTRIPL
#define PCS_IINV_B_XBAR_FLAG2          XBAR_INPUT_FLG_CMPSS6_CTRIPH

#define PCS_IINV_C_CMPSS_BASE          CMPSS3_BASE  //C相电流2   CMPIN36P
#define PCS_IINV_C_XBAR_MUX            XBAR_MUX04
#define PCS_IINV_C_XBAR_MUX_VAL        XBAR_EPWM_MUX04_CMPSS3_CTRIPH_OR_L
#define PCS_IINV_C_XBAR_FLAG1          XBAR_INPUT_FLG_CMPSS3_CTRIPL
#define PCS_IINV_C_XBAR_FLAG2          XBAR_INPUT_FLG_CMPSS3_CTRIPH

#define PCS_IBATT_CMPSS_BASE           CMPSS5_BASE  //直流电流采样   CMPIN5P
#define PCS_IBATT_XBAR_MUX             XBAR_MUX08
#define PCS_IBATT_XBAR_MUX_VAL         XBAR_EPWM_MUX08_CMPSS5_CTRIPH_OR_L
#define PCS_IBATT_XBAR_FLAG1           XBAR_INPUT_FLG_CMPSS5_CTRIPL
#define PCS_IBATT_XBAR_FLAG2           XBAR_INPUT_FLG_CMPSS5_CTRIPH

#define PCS_IDC_N_CMPSS_BASE           CMPSS1_BASE  //直流中线电流   CMPIN1P
#define PCS_IDC_N_XBAR_MUX             XBAR_MUX00
#define PCS_IDC_N_XBAR_MUX_VAL         XBAR_EPWM_MUX00_CMPSS1_CTRIPH_OR_L
#define PCS_IDC_N_XBAR_FLAG1           XBAR_INPUT_FLG_CMPSS1_CTRIPL
#define PCS_IDC_N_XBAR_FLAG2           XBAR_INPUT_FLG_CMPSS1_CTRIPH

#define PCS_VDC_P_CMPSS_BASE           CMPSS2_BASE  //直流正电压   CMPIN2P
#define PCS_VDC_P_XBAR_MUX             XBAR_MUX02
#define PCS_VDC_P_XBAR_MUX_VAL         XBAR_EPWM_MUX02_CMPSS2_CTRIPH_OR_L
#define PCS_VDC_P_XBAR_FLAG1           XBAR_INPUT_FLG_CMPSS2_CTRIPL
#define PCS_VDC_P_XBAR_FLAG2           XBAR_INPUT_FLG_CMPSS2_CTRIPH

#define PCS_VDC_CMPSS_BASE             CMPSS7_BASE  //直流电压   CMPIN7P
#define PCS_VDC_XBAR_MUX               XBAR_MUX12
#define PCS_VDC_XBAR_MUX_VAL           XBAR_EPWM_MUX12_CMPSS7_CTRIPH_OR_L
#define PCS_VDC_XBAR_FLAG1             XBAR_INPUT_FLG_CMPSS12_CTRIPL
#define PCS_VDC_XBAR_FLAG2             XBAR_INPUT_FLG_CMPSS12_CTRIPH

//#define PCS_IINV_MAX_SENSE_AMPS        ((float32_t)220.0)
//#define PCS_IINV_TRIP_LIMIT_AMPS       ((float32_t)200.0)

#define PCS_IBATT_MAX_SENSE_AMPS       ((float32_t)220.0)
#define PCS_IBATT_TRIP_LIMIT_AMPS      ((float32_t)200.0)

#define PCS_IACP_HOVER_GPIO             69  //交流电流正向外部比较
#define PCS_IACP_HOVER_PIN_CONFIG       GPIO_69_GPIO69
#define PCS_IACP_HOVER_XBAR_MUX         XBAR_MUX03
#define PCS_IACP_HOVER_XBAR_MUX_VAL     XBAR_EPWM_MUX03_INPUTXBAR2
#define PCS_IACP_HOVER_XBAR_FLAG        XBAR_INPUT_FLG_INPUT2

#define PCS_IACN_HOVER_GPIO             67  //交流电流反向外部比较
#define PCS_IACN_HOVER_PIN_CONFIG       GPIO_67_GPIO67
#define PCS_IACN_HOVER_XBAR_MUX         XBAR_MUX05
#define PCS_IACN_HOVER_XBAR_MUX_VAL     XBAR_EPWM_MUX05_INPUTXBAR3
#define PCS_IACN_HOVER_XBAR_FLAG        XBAR_INPUT_FLG_INPUT3

#define PCS_ESTOP_GPIO                  133  //外部急停输入 低电平有效
#define PCS_ESTOP_GPIO_PIN_CONFIG       GPIO_133_GPIO133
#define PCS_ESTOP_XBAR_MUX              XBAR_MUX01
#define PCS_ESTOP_XBAR_MUX_VAL          XBAR_EPWM_MUX01_INPUTXBAR1
#define PCS_ESTOP_XBAR_FLAG             XBAR_INPUT_FLG_INPUT1

#define PCS_IDC_HOVER_GPIO              80  //直流电流外部比较
#define PCS_IDC_HOVER_PIN_CONFIG        GPIO_80_GPIO80
#define PCS_IDC_HOVER_XBAR_MUX          XBAR_MUX07
#define PCS_IDC_HOVER_XBAR_MUX_VAL      XBAR_EPWM_MUX07_INPUTXBAR4
#define PCS_IDC_HOVER_XBAR_FLAG         XBAR_INPUT_FLG_INPUT4

#define PCS_PROTECTION_ENABLED          1   //使能PWM TripZone
#define PCS_PROTECTION_DISABLED         0   //不使能PWM TripZone
#define PCS_PROTECTION                  PCS_PROTECTION_DISABLED


#define AC_GRID_VMAX                    ((float32_t)253.0)
#define AC_GRID_VMIN                    ((float32_t)187.0) //220*(1±15%)

uint64_t  epwm1TZIntCount = 0;
uint64_t  epwm1TZIntCount_CBC = 0;

//static int16_t ACGridOVol_errCnt = 0;//电网过压故障计数
//static int16_t ACGridUVol_errCnt = 0;//电网欠压故障计数
static int16_t ACOverVol_errCnt = 0;//AC过压故障计数
static int16_t ACOverCurr_errCnt = 0;//AC过流故障计数
static int16_t ACUnbalance_errCnt = 0;//AC 电压不平衡故障计数
static int16_t ACFreq_errCnt = 0;//AC 频率故障计数
static int16_t DCUnbalance_errCnt = 0;//DC 电压不平衡故障计数
static int16_t DCOverVol_errCnt = 0;//DC过压故障计数
static int16_t DCOverCurr_errCnt = 0;//DC过流故障计数
static int16_t DCUnderVol_errCnt = 0;//DC欠压故障计数
static int16_t IgbtOverTemp_errCnt = 0;//Igbt过温故障计数
static int16_t AmbOverTemp_errCnt = 0;//Amb过温故障计数
static int16_t PhaseSequence_errCnt = 0;
static int16_t DCVolRP_errCnt = 0;//直流极性反接故障计数

extern uint16_t flag1;

//EPWM TripZone 配置为一次性关断保护状态 触发源为外部硬件GPIO
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
//}

//EPWM TripZone 配置为CBC逐波限流保护状态，同时配置EPWM1 TripZone中断，中断次数超过2个50HZ周波时，报故障彻底关断 源为内部比较器
//void PCS_HAL_setupPWMforTrip(uint32_t base)
//{
//    // Trip 4 is the input to the DCAHCOMPSEL
//    EPWM_selectDigitalCompareTripInput(base,EPWM_DC_TRIP_TRIPIN4,EPWM_DC_TYPE_DCAH);//DCAH输入源为TRIPIN4
//    EPWM_setTripZoneDigitalCompareEventCondition(base,EPWM_TZ_DC_OUTPUT_A2,EPWM_TZ_EVENT_DCXH_HIGH);//DCAH高电平时触发 DCAEVT2
//
////    EPWM_setDigitalCompareEventSource(base,EPWM_DC_MODULE_A,EPWM_DC_EVENT_2,EPWM_DC_EVENT_SOURCE_ORIG_SIGNAL);
////    EPWM_setDigitalCompareEventSyncMode(base,EPWM_DC_MODULE_A,EPWM_DC_EVENT_2,EPWM_DC_EVENT_INPUT_NOT_SYNCED);
//
//    // What do we want the OST / CBC events to do?
//    // TZA events can force EPWMxA
//    // TZB events can force EPWMxB
//    EPWM_setTripZoneAction(base, EPWM_TZ_ACTION_EVENT_TZA, EPWM_TZ_ACTION_LOW);
//    EPWM_setTripZoneAction(base, EPWM_TZ_ACTION_EVENT_TZB, EPWM_TZ_ACTION_LOW);
//
//    // Enable the following trips - DCAEVT2
//    EPWM_enableTripZoneSignals(base, EPWM_TZ_SIGNAL_DCAEVT2);
//    //使能EPWM1的CBC中断，用于计数逐波限流次数
//    if(base == EPWM1_BASE)
//        EPWM_enableTripZoneInterrupt(base,EPWM_TZ_INTERRUPT_CBC);
//
//    EPWM_selectCycleByCycleTripZoneClearEvent(base,EPWM_TZ_CBC_PULSE_CLR_CNTR_ZERO);
//    // Clear any spurious DCAEVT2 flags
//    EPWM_clearTripZoneFlag(base, EPWM_TZ_FLAG_DCAEVT2);
//    EPWM_clearTripZoneFlag(base, EPWM_TZ_FLAG_CBC);//配置为Cycle-By-Cycle模式
//
//    // Force a trip on PWM to safely start the system
////    EPWM_forceTripZoneEvent(base, EPWM_TZ_FLAG_OST);
//
//}

//void PCS_HAL_setupPWMforTrip(uint32_t base)
//{
//    //----------------------------------------ost模式--------------------------------------
//    // Trip 4 is the input to the DCAHCOMPSEL
//    EPWM_selectDigitalCompareTripInput(base,EPWM_DC_TRIP_TRIPIN5,EPWM_DC_TYPE_DCAH);//DCAH输入源为TRIPIN5 GPIO触发
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
////    if(base == EPWM1_BASE)
////           EPWM_enableTripZoneInterrupt(base,EPWM_TZ_INTERRUPT_OST);
//
//    // Clear any spurious DCAEVT1 flags
//    EPWM_clearTripZoneFlag(base, EPWM_TZ_FLAG_DCAEVT1);
//    EPWM_clearTripZoneFlag(base, EPWM_TZ_FLAG_OST);
//
//    // Force a trip on PWM to safely start the system
////    EPWM_forceTripZoneEvent(base, EPWM_TZ_FLAG_OST);
//
////    ------------------------------------------cbc模式--------------------------------------
//    // Trip 4 is the input to the DCAHCOMPSEL
//    EPWM_selectDigitalCompareTripInput(base,EPWM_DC_TRIP_TRIPIN4,EPWM_DC_TYPE_DCAH);//DCAH输入源为TRIPIN4
//    EPWM_setTripZoneDigitalCompareEventCondition(base,EPWM_TZ_DC_OUTPUT_A2,EPWM_TZ_EVENT_DCXH_HIGH);//DCAH高电平时触发 DCAEVT2
//
////    EPWM_setDigitalCompareEventSource(base,EPWM_DC_MODULE_A,EPWM_DC_EVENT_2,EPWM_DC_EVENT_SOURCE_ORIG_SIGNAL);
////    EPWM_setDigitalCompareEventSyncMode(base,EPWM_DC_MODULE_A,EPWM_DC_EVENT_2,EPWM_DC_EVENT_INPUT_NOT_SYNCED);
//
//    // What do we want the OST / CBC events to do?
//    // TZA events can force EPWMxA
//    // TZB events can force EPWMxB
//    EPWM_setTripZoneAction(base, EPWM_TZ_ACTION_EVENT_TZA, EPWM_TZ_ACTION_LOW);
//    EPWM_setTripZoneAction(base, EPWM_TZ_ACTION_EVENT_TZB, EPWM_TZ_ACTION_LOW);
//
//    // Enable the following trips - DCAEVT2
//    EPWM_enableTripZoneSignals(base, EPWM_TZ_SIGNAL_DCAEVT2);
//    //使能EPWM1的CBC中断，用于计数逐波限流次数
////    if(base == EPWM1_BASE)
////        EPWM_enableTripZoneInterrupt(base,EPWM_TZ_INTERRUPT_CBC);
//    EPWM_selectCycleByCycleTripZoneClearEvent(base,EPWM_TZ_CBC_PULSE_CLR_CNTR_ZERO);
//    // Clear any spurious DCAEVT2 flags
//    EPWM_clearTripZoneFlag(base, EPWM_TZ_FLAG_DCAEVT2);
//    EPWM_clearTripZoneFlag(base, EPWM_TZ_FLAG_CBC);//配置为Cycle-By-Cycle模式
//
//    // Force a trip on PWM to safely start the system
////    EPWM_forceTripZoneEvent(base, EPWM_TZ_FLAG_OST);
//
//}

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
// epwm1TZISR - ePWM1 TZ ISR
//
/*uint32_t  isrStarttime,isrEndtime;
uint32_t  isrAllTime;
__interrupt void epwm1TZISR(void)
{
    epwm1TZIntCount++;
//    if(epwm1TZIntCount == 1)
//    {
//        isrStarttime = m_u32_TimerCnt;
//    }
//    isrEndtime = m_u32_TimerCnt;
//    isrAllTime = isrEndtime - isrStarttime;
//    if(epwm1TZIntCount > 25000)//PWM计数等于0时清CBC，62.5us清一次，2个周波=40000/62.5=640
//        epwm1TZIntCount = 25000;
    if(EPWM_getTripZoneFlagStatus(EPWM1_BASE) & EPWM_TZ_FLAG_DCAEVT1)
    {
        Drv_PwmOffset();
        Pcs_gParam.StartEn = 0;
        FaultStatus.PCS_HFault.tbits.bEmergencyStop         = XBAR_getInputFlagStatus(XBAR_INPUT_FLG_INPUT1);
        FaultStatus.PCS_HFault.tbits.bACHIOver_Charge       = XBAR_getInputFlagStatus(XBAR_INPUT_FLG_INPUT2);
        FaultStatus.PCS_HFault.tbits.bACHIOver_DisCharge    = XBAR_getInputFlagStatus(XBAR_INPUT_FLG_INPUT3);
        FaultStatus.PCS_HFault.tbits.bDCHIOver_Charge       = XBAR_getInputFlagStatus(XBAR_INPUT_FLG_INPUT4);
    }
    if(EPWM_getTripZoneFlagStatus(EPWM1_BASE) & EPWM_TZ_FLAG_DCAEVT2)
    {
        epwm1TZIntCount_CBC++;
        if(epwm1TZIntCount_CBC >= 320)
        {
            epwm1TZIntCount_CBC = 320;
            Drv_PwmOffset();
            Pcs_gParam.StartEn = 0;
            FaultStatus.PCS_HFault.tbits.bACCIOver_DisCharge    = XBAR_getInputFlagStatus(XBAR_INPUT_FLG_CMPSS3_CTRIPL);
            FaultStatus.PCS_HFault.tbits.bACCIOver_Charge       = XBAR_getInputFlagStatus(XBAR_INPUT_FLG_CMPSS3_CTRIPH);

            FaultStatus.PCS_HFault.tbits.bACBIOver_DisCharge    = XBAR_getInputFlagStatus(XBAR_INPUT_FLG_CMPSS6_CTRIPL);
            FaultStatus.PCS_HFault.tbits.bACBIOver_Charge       = XBAR_getInputFlagStatus(XBAR_INPUT_FLG_CMPSS6_CTRIPH);

            FaultStatus.PCS_HFault.tbits.bACAIOver_DisCharge    = XBAR_getInputFlagStatus(XBAR_INPUT_FLG_CMPSS8_CTRIPL);
            FaultStatus.PCS_HFault.tbits.bACAIOver_Charge       = XBAR_getInputFlagStatus(XBAR_INPUT_FLG_CMPSS8_CTRIPH);

            FaultStatus.PCS_HFault.tbits.bDCIOver_DisCharge     = XBAR_getInputFlagStatus(XBAR_INPUT_FLG_CMPSS5_CTRIPL);
            FaultStatus.PCS_HFault.tbits.bDCIOver_Charge        = XBAR_getInputFlagStatus(XBAR_INPUT_FLG_CMPSS5_CTRIPH);
        }
    }
    else
    {
        epwm1TZIntCount_CBC=0;
    }
    XBAR_clearInputFlag(XBAR_INPUT_FLG_CMPSS8_CTRIPL);
    EPWM_clearTripZoneFlag(EPWM1_BASE, EPWM_TZ_FLAG_DCAEVT2);
    EPWM_clearTripZoneFlag(EPWM1_BASE, (EPWM_TZ_INTERRUPT | EPWM_TZ_FLAG_CBC));
    EPWM_clearCycleByCycleTripZoneFlag(EPWM1_BASE, EPWM_TZ_CBC_FLAG_DCAEVT2);

//    EPWM_clearTripZoneFlag(EPWM2_BASE, EPWM_TZ_FLAG_CBC);//
//    EPWM_clearTripZoneFlag(EPWM3_BASE, EPWM_TZ_FLAG_CBC);//
//    EPWM_clearTripZoneFlag(EPWM4_BASE, EPWM_TZ_FLAG_CBC);//
//    EPWM_clearTripZoneFlag(EPWM5_BASE, EPWM_TZ_FLAG_CBC);//
//    EPWM_clearTripZoneFlag(EPWM6_BASE, EPWM_TZ_FLAG_CBC);//
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP2);
}*/

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
// INTERRUPT Configurations
//
//*****************************************************************************
//void InterruptEpwmCBC_OST_init(void)
//{
//    // Interrupt Setings for INT_EPWM1_TZ
//    Interrupt_register(INT_EPWM1_TZ, &epwm1TZISR);
//    Interrupt_enable(INT_EPWM1_TZ);
//}

//逐波限流设置
void PCS_HAL_setupCBCProtection()
{
    //
    // Disable all the muxes first
    //
    XBAR_disableEPWMMux(XBAR_TRIP4, 0xFF);
//    XBAR_disableEPWMMux(XBAR_TRIP5, 0xFF);

    //模块2 A相过流硬件保护
#if PCS_PROTECTION_IINV_A == 1
    PCS_HAL_setupCMPSS(PCS_IINV_A_CMPSS_BASE,3065,655);
    XBAR_setEPWMMuxConfig(XBAR_TRIP4, PCS_IINV_A_XBAR_MUX_VAL);
    XBAR_enableEPWMMux(XBAR_TRIP4, PCS_IINV_A_XBAR_MUX);
    XBAR_clearInputFlag(PCS_IINV_A_XBAR_FLAG1);
    XBAR_clearInputFlag(PCS_IINV_A_XBAR_FLAG2);
#endif

    //模块2 B相过流硬件保护
#if PCS_PROTECTION_IINV_B == 1

//    CMPSS_setDACValueHigh(base1, 3692);//3.1875V:300A,125kW:189A rms:268A pp,3.1875/300*280=2.975, 3692=2.975*4096/3.3
//    CMPSS_setDACValueLow(base1, 1954);//1.6875V:-300A,125kW:189A rms:268A pp,1.6875/300*280=1.575, 1954=1.575*4096/3.3
    PCS_HAL_setupCMPSS(PCS_IINV_B_CMPSS_BASE,3065,655);

    XBAR_setEPWMMuxConfig(XBAR_TRIP4, PCS_IINV_B_XBAR_MUX_VAL);
    XBAR_enableEPWMMux(XBAR_TRIP4, PCS_IINV_B_XBAR_MUX);
    XBAR_clearInputFlag(PCS_IINV_B_XBAR_FLAG1);
    XBAR_clearInputFlag(PCS_IINV_B_XBAR_FLAG2);

#endif
// C相过流硬件保护
#if PCS_PROTECTION_IINV_C == 1
    PCS_HAL_setupCMPSS(PCS_IINV_C_CMPSS_BASE,3065,655);
    XBAR_setEPWMMuxConfig(XBAR_TRIP4, PCS_IINV_C_XBAR_MUX_VAL);
    XBAR_enableEPWMMux(XBAR_TRIP4, PCS_IINV_C_XBAR_MUX);
    XBAR_clearInputFlag(PCS_IINV_C_XBAR_FLAG1);
    XBAR_clearInputFlag(PCS_IINV_C_XBAR_FLAG2);
#endif

//DC过流硬件保护
#if PCS_PROTECTION_IBATT == 1
//    CMPSS_setDACValueHigh(base1, 3692);//2.5V:300A,125kW:208A ,2.5/300*210=1.75, 2172=1.75*4096/3.3
//    CMPSS_setDACValueLow(base1, 1954);//0.5V:-300A,125kW:208A,0.5/300*210=1.575, 1954=1.575*4096/3.3
    PCS_HAL_setupCMPSS(PCS_IBATT_CMPSS_BASE,3065,655);
    XBAR_setEPWMMuxConfig(XBAR_TRIP4, PCS_IBATT_XBAR_MUX_VAL);
    XBAR_enableEPWMMux(XBAR_TRIP4, PCS_IBATT_XBAR_MUX);
    XBAR_clearInputFlag(PCS_IBATT_XBAR_FLAG1);
    XBAR_clearInputFlag(PCS_IBATT_XBAR_FLAG2);

#endif

#if PCS_PROTECTION_IDCN == 1 //直流中线电流
    PCS_HAL_setupCMPSS(PCS_IDC_N_CMPSS_BASE,3065,655);
    XBAR_setEPWMMuxConfig(XBAR_TRIP4, PCS_IDC_N_XBAR_MUX_VAL);
    XBAR_enableEPWMMux(XBAR_TRIP4, PCS_IDC_N_XBAR_MUX);
    XBAR_clearInputFlag(PCS_IDC_N_XBAR_FLAG1);
    XBAR_clearInputFlag(PCS_IDC_N_XBAR_FLAG2);
#endif

#if PCS_PROTECTION_VDCP == 1 //直流正电压
    PCS_HAL_setupCMPSS(PCS_VDC_P_CMPSS_BASE,3065,655);
    XBAR_setEPWMMuxConfig(XBAR_TRIP4, PCS_VDC_P_XBAR_MUX_VAL);
    XBAR_enableEPWMMux(XBAR_TRIP4, PCS_VDC_P_XBAR_MUX);
    XBAR_clearInputFlag(PCS_VDC_P_XBAR_FLAG1);
    XBAR_clearInputFlag(PCS_VDC_P_XBAR_FLAG2);
#endif

#if PCS_PROTECTION_VDC == 1 //直流电压
    PCS_HAL_setupCMPSS(PCS_VDC_CMPSS_BASE,3065,655);
    XBAR_setEPWMMuxConfig(XBAR_TRIP4, PCS_VDC_XBAR_MUX_VAL);
    XBAR_enableEPWMMux(XBAR_TRIP4, PCS_VDC_XBAR_MUX);
    XBAR_clearInputFlag(PCS_VDC_XBAR_FLAG1);
    XBAR_clearInputFlag(PCS_VDC_XBAR_FLAG2);
#endif

//AC硬件电路外部正向电流保护
#if PCS_PROTECTION_IACP_HAL== 1
    GPIO_setDirectionMode(PCS_IACP_HOVER_GPIO, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(PCS_IACP_HOVER_GPIO, GPIO_QUAL_SYNC);
    GPIO_setPinConfig(PCS_IACP_HOVER_PIN_CONFIG);
    GPIO_setPadConfig(PCS_IACP_HOVER_GPIO, GPIO_PIN_TYPE_STD);

    XBAR_setInputPin(INPUTXBAR_BASE,XBAR_INPUT2, PCS_IACP_HOVER_GPIO);
    XBAR_setEPWMMuxConfig(XBAR_TRIP5, PCS_IACP_HOVER_XBAR_MUX_VAL);
    XBAR_enableEPWMMux(XBAR_TRIP5, PCS_IACP_HOVER_XBAR_MUX);
    XBAR_clearInputFlag(PCS_IACP_HOVER_XBAR_FLAG);
#endif

//AC硬件电路外部反向电流保护
#if PCS_PROTECTION_IACN_HAL== 1
    GPIO_setDirectionMode(PCS_IACN_HOVER_GPIO, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(PCS_IACN_HOVER_GPIO, GPIO_QUAL_SYNC);
    GPIO_setPinConfig(PCS_IACN_HOVER_PIN_CONFIG);
    GPIO_setPadConfig(PCS_IACN_HOVER_GPIO, GPIO_PIN_TYPE_STD);

    XBAR_setInputPin(INPUTXBAR_BASE,XBAR_INPUT3, PCS_IACN_HOVER_GPIO);
    XBAR_setEPWMMuxConfig(XBAR_TRIP5, PCS_IACN_HOVER_XBAR_MUX_VAL);
    XBAR_enableEPWMMux(XBAR_TRIP5, PCS_IACN_HOVER_XBAR_MUX);
    XBAR_clearInputFlag(PCS_IACN_HOVER_XBAR_FLAG);
#endif

//急停按钮输入
#if PCS_PROTECTION_ESTOP== 1
    GPIO_setDirectionMode(PCS_ESTOP_GPIO, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(PCS_ESTOP_GPIO, GPIO_QUAL_SYNC);
    GPIO_setPinConfig(PCS_ESTOP_GPIO_PIN_CONFIG);
    GPIO_setPadConfig(PCS_ESTOP_GPIO, GPIO_PIN_TYPE_INVERT);//DI低电平有效，TripZone是高电平触发 GPIO_PIN_TYPE_INVERT |

    XBAR_setInputPin(INPUTXBAR_BASE,XBAR_INPUT1, PCS_ESTOP_GPIO);
    XBAR_setEPWMMuxConfig(XBAR_TRIP5, PCS_ESTOP_XBAR_MUX_VAL);
    XBAR_enableEPWMMux(XBAR_TRIP5, PCS_ESTOP_XBAR_MUX);
    XBAR_clearInputFlag(PCS_ESTOP_XBAR_FLAG);
#endif

//DC硬件外部反向电流保护
#if PCS_PROTECTION_IDC_HAL== 1
    GPIO_setDirectionMode(PCS_IDC_HOVER_GPIO, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(PCS_IDC_HOVER_GPIO, GPIO_QUAL_SYNC);
    GPIO_setPinConfig(PCS_IDC_HOVER_PIN_CONFIG);
    GPIO_setPadConfig(PCS_IDC_HOVER_GPIO, GPIO_PIN_TYPE_STD);

    XBAR_setInputPin(INPUTXBAR_BASE,XBAR_INPUT4, PCS_IDC_HOVER_GPIO);
    XBAR_setEPWMMuxConfig(XBAR_TRIP5, PCS_IDC_HOVER_XBAR_MUX_VAL);
    XBAR_enableEPWMMux(XBAR_TRIP5, PCS_IDC_HOVER_XBAR_MUX);
    XBAR_clearInputFlag(PCS_IDC_HOVER_XBAR_FLAG);
#endif

//    XBAR_invertEPWMSignal(XBAR_TRIP4,false);

//逐波限流EPWM TripZone使能
#if PCS_PROTECTION == PCS_PROTECTION_ENABLED
    InterruptEpwmCBC_OST_init();
    PCS_HAL_setupPWMforTrip(PCS_Q1_Q3_A_PWM_BASE);
    PCS_HAL_setupPWMforTrip(PCS_Q2_Q4_A_PWM_BASE);
    PCS_HAL_setupPWMforTrip(PCS_Q1_Q3_B_PWM_BASE);
    PCS_HAL_setupPWMforTrip(PCS_Q2_Q4_B_PWM_BASE);
    PCS_HAL_setupPWMforTrip(PCS_Q1_Q3_C_PWM_BASE);
    PCS_HAL_setupPWMforTrip(PCS_Q2_Q4_C_PWM_BASE);
    PCS_HAL_setupPWMforTrip(PCS_Q1_Q2_N_PWM_BASE);

#endif

}
/**
 * 网侧相序故障监测
 * 连续1S内监测到异常，报故障
 * */
void FaultMonitor_PhaseSequence(void)
{
    static float32_t Pcs_vGrid_A_sensed_pu_bak;
//    if(m_st_TimerFlag.u16_b1ms == 0)
//        return 0;
    if((Pcs_Output_Meter.PCS_AC_VRms_PhaseA > 110.0f)&&(Pcs_Output_Meter.PCS_AC_VRms_PhaseB > 110.0f)&&(Pcs_Output_Meter.PCS_AC_VRms_PhaseC > 110.0f))
    {
        if((tCla2Cpu.Pcs_vGrid_A_sensed_pu > 0)&&(Pcs_vGrid_A_sensed_pu_bak < 0))
        {
            if((tCla2Cpu.Pcs_vGrid_B_sensed_pu < 0)&&(tCla2Cpu.Pcs_vGrid_C_sensed_pu > 0))
            {
                PhaseSequence_errCnt = 0;
            }
            else
            {
                PhaseSequence_errCnt++;
                if(PhaseSequence_errCnt > 200)
                {
                    PhaseSequence_errCnt = 200;
                    Cpu1Ipc_cpu2cm.FaultStatus.PCS_Fault1.tbits.bACVPhaseSequFault = 1;
                }
            }
        }
        Pcs_vGrid_A_sensed_pu_bak = tCla2Cpu.Pcs_vGrid_A_sensed_pu;
    }
//    else
//    {
//        PhaseSequence_errCnt = 0;
//        FaultStatus.PCS_Fault1.tbits.bACVPhaseSequFault = 0;
//    }
}
//AC过压故障
void FaultMonitor_ACOverVoltage(void)
{
    if(m_st_TimerFlag.u16_b10ms == 1)
    {
        //母线过压
        if((Pcs_Output_Meter.PCS_AC_VRms_LineAB > (float32_t)Cpu1Ipc_cm2cpu.AC_OverVol_Value*0.1f)||
           (Pcs_Output_Meter.PCS_AC_VRms_LineBC > (float32_t)Cpu1Ipc_cm2cpu.AC_OverVol_Value*0.1f)||
           (Pcs_Output_Meter.PCS_AC_VRms_LineCA > (float32_t)Cpu1Ipc_cm2cpu.AC_OverVol_Value*0.1f))
        {
            ACOverVol_errCnt++;
        }
        else if((Pcs_Output_Meter.PCS_AC_VRms_LineAB < (float32_t)Cpu1Ipc_cm2cpu.AC_OverVol_Value*0.1f-10.0f)||
                (Pcs_Output_Meter.PCS_AC_VRms_LineBC < (float32_t)Cpu1Ipc_cm2cpu.AC_OverVol_Value*0.1f-10.0f)||
                (Pcs_Output_Meter.PCS_AC_VRms_LineCA < (float32_t)Cpu1Ipc_cm2cpu.AC_OverVol_Value*0.1f-10.0f))
        {
            if(ACOverVol_errCnt > 0)
                ACOverVol_errCnt--;
            else
                ACOverVol_errCnt = 0;
        }

        if(ACOverVol_errCnt >= 100)
        {
            ACOverVol_errCnt = 100;
            Cpu1Ipc_cpu2cm.FaultStatus.PCS_Fault1.tbits.bACVOverFault = 1;
        }
    }
}

//AC欠压故障
void FaultMonitor_ACUnderVoltage(void)
{
    if(m_st_TimerFlag.u16_b10ms == 1)
    {
        //母线过压
        if((Pcs_Output_Meter.PCS_AC_VRms_LineAB < (float32_t)Cpu1Ipc_cm2cpu.AC_UnderVol_Value*0.1f)||
           (Pcs_Output_Meter.PCS_AC_VRms_LineBC < (float32_t)Cpu1Ipc_cm2cpu.AC_UnderVol_Value*0.1f)||
           (Pcs_Output_Meter.PCS_AC_VRms_LineCA < (float32_t)Cpu1Ipc_cm2cpu.AC_UnderVol_Value*0.1f))
        {
            ACOverVol_errCnt++;
        }
        else if((Pcs_Output_Meter.PCS_AC_VRms_PhaseA > (float32_t)Cpu1Ipc_cm2cpu.AC_UnderVol_Value*0.1f+10.0f)||
                (Pcs_Output_Meter.PCS_AC_VRms_PhaseB > (float32_t)Cpu1Ipc_cm2cpu.AC_UnderVol_Value*0.1f+10.0f)||
                (Pcs_Output_Meter.PCS_AC_VRms_PhaseC > (float32_t)Cpu1Ipc_cm2cpu.AC_UnderVol_Value*0.1f+10.0f))
        {
            if(ACOverVol_errCnt > 0)
                ACOverVol_errCnt--;
            else
                ACOverVol_errCnt = 0;
        }

        if(ACOverVol_errCnt >= 100)
        {
            ACOverVol_errCnt = 100;
            Cpu1Ipc_cpu2cm.FaultStatus.PCS_Fault1.tbits.bACVUnderFault = 1;
        }
    }
}

//AC过流故障
void FaultMonitor_ACCurrentOver(void)
{
    float32_t Iac_Temp;
    Iac_Temp = tCla2Cpu.Pcs_IoutMd*0.70721f;
//    if(m_st_TimerFlag.u16_b10ms == 1)
    {
        //
        if(Iac_Temp > (float32_t)Cpu1Ipc_cm2cpu.AC_OverCurr_Value*0.1f)
        {
            ACOverCurr_errCnt++;
        }
        else if(Iac_Temp < (float32_t)Cpu1Ipc_cm2cpu.AC_OverCurr_Value*0.1f-10.0f)
        {
            if(ACOverCurr_errCnt > 0)
                ACOverCurr_errCnt--;
            else
                ACOverCurr_errCnt = 0;
        }

        if(ACOverCurr_errCnt >= 50)
        {
            ACOverCurr_errCnt = 50;
            Cpu1Ipc_cpu2cm.FaultStatus.PCS_Fault1.tbits.bIINVOverFault = 1;
        }
    }
}

//AC三相不平衡故障
void FaultMonitor_ACVoltageUnbanlance(void)
{
    if(m_st_TimerFlag.u16_b10ms == 1)
    {
        //三相不平衡
        if((fabsf(Pcs_Output_Meter.PCS_AC_VRms_PhaseA-Pcs_Output_Meter.PCS_AC_VRms_PhaseB) > (float32_t)Cpu1Ipc_cm2cpu.AC_UnbanceVol_Value*0.1f)||
           (fabsf(Pcs_Output_Meter.PCS_AC_VRms_PhaseB-Pcs_Output_Meter.PCS_AC_VRms_PhaseC) > (float32_t)Cpu1Ipc_cm2cpu.AC_UnbanceVol_Value*0.1f)||
           (fabsf(Pcs_Output_Meter.PCS_AC_VRms_PhaseC-Pcs_Output_Meter.PCS_AC_VRms_PhaseA) > (float32_t)Cpu1Ipc_cm2cpu.AC_UnbanceVol_Value*0.1f))
        {
            ACUnbalance_errCnt++;
        }
        else if((fabsf(Pcs_Output_Meter.PCS_AC_VRms_PhaseA-Pcs_Output_Meter.PCS_AC_VRms_PhaseB) < ((float32_t)Cpu1Ipc_cm2cpu.AC_UnbanceVol_Value*0.1f-10.0f))||
                (fabsf(Pcs_Output_Meter.PCS_AC_VRms_PhaseB-Pcs_Output_Meter.PCS_AC_VRms_PhaseC) < ((float32_t)Cpu1Ipc_cm2cpu.AC_UnbanceVol_Value*0.1f-10.0f))||
                (fabsf(Pcs_Output_Meter.PCS_AC_VRms_PhaseC-Pcs_Output_Meter.PCS_AC_VRms_PhaseA) < ((float32_t)Cpu1Ipc_cm2cpu.AC_UnbanceVol_Value*0.1f-10.0f)))
        {
            if(ACUnbalance_errCnt > 0)
                ACUnbalance_errCnt--;
            else
                ACUnbalance_errCnt = 0;
        }

        if(ACUnbalance_errCnt >= 100)
        {
            ACUnbalance_errCnt = 100;
            Cpu1Ipc_cpu2cm.FaultStatus.PCS_Fault1.tbits.bACVBanlanceFault = 1;
        }
    }
}

//AC频率故障
void FaultMonitor_ACFreq(void)
{
    if((m_st_TimerFlag.u16_b10ms == 1)&&
        (Pcs_Output_Meter.PCS_AC_VRms_PhaseA > 15.0f)&&(Pcs_Output_Meter.PCS_AC_VRms_PhaseB > 15.0f)&&(Pcs_Output_Meter.PCS_AC_VRms_PhaseC > 15.0f))
    {
        //频率故障
        if((fabsf(Pcs_Output_Meter.PCS_AC_Freq_A-50.0f) > 3.8f)||
           (fabsf(Pcs_Output_Meter.PCS_AC_Freq_B-50.0f) > 3.8f)||
           (fabsf(Pcs_Output_Meter.PCS_AC_Freq_C-50.0f) > 3.8f))
        {
            ACFreq_errCnt++;
        }
        else if((fabsf(Pcs_Output_Meter.PCS_AC_Freq_A-50.0f) < 1.5f)||
                (fabsf(Pcs_Output_Meter.PCS_AC_Freq_B-50.0f) < 1.5f)||
                (fabsf(Pcs_Output_Meter.PCS_AC_Freq_C-50.0f) < 1.5f))
        {
            if(ACUnbalance_errCnt > 0)
                ACFreq_errCnt--;
            else
                ACFreq_errCnt = 0;
        }

        if(ACFreq_errCnt >= 100)
        {
            ACFreq_errCnt = 100;
            Cpu1Ipc_cpu2cm.FaultStatus.PCS_Fault1.tbits.bACVFreqFault = 1;
        }
    }
}

//DC不平衡故障
void FaultMonitor_DCVoltageUnbanlance(void)
{
    if(m_st_TimerFlag.u16_b10ms == 1)
    {
        //DC不平衡
        if(fabsf(Pcs_Output_Meter.PCS_DC_PBusVol-Pcs_Output_Meter.PCS_DC_NBusVol) > (float32_t)Cpu1Ipc_cm2cpu.DC_UnbanceVol_Value*0.1f)
        {
            DCUnbalance_errCnt++;
        }
        else if(fabsf(Pcs_Output_Meter.PCS_DC_PBusVol-Pcs_Output_Meter.PCS_DC_NBusVol) < ((float32_t)Cpu1Ipc_cm2cpu.DC_UnbanceVol_Value*0.1f-10.0f))
        {
            if(DCUnbalance_errCnt > 0)
                DCUnbalance_errCnt--;
            else
                DCUnbalance_errCnt = 0;
        }

        if(DCUnbalance_errCnt >= 100)
        {
            DCUnbalance_errCnt = 100;
            Cpu1Ipc_cpu2cm.FaultStatus.PCS_Fault2.tbits.bBUSVUnBanlanceFault = 1;
        }
    }
}

//DC过压故障
void FaultMonitor_DCVoltageOver(void)
{
    if(m_st_TimerFlag.u16_b10ms == 1)
    {
        //DC过压
        if(Pcs_Output_Meter.PCS_DC_BusVol > (float32_t)Cpu1Ipc_cm2cpu.DC_OverVol_Value*0.1f)
        {
            DCOverVol_errCnt++;
        }
        else if(Pcs_Output_Meter.PCS_DC_BusVol < (float32_t)Cpu1Ipc_cm2cpu.DC_OverVol_Value*0.1f-20.0f)
        {
            if(DCOverVol_errCnt > 0)
                DCOverVol_errCnt--;
            else
                DCOverVol_errCnt = 0;
        }

        if(DCOverVol_errCnt >= 100)
        {
            DCOverVol_errCnt = 100;
            Cpu1Ipc_cpu2cm.FaultStatus.PCS_Fault2.tbits.bBUSVOverFault = 1;
        }
    }
}

//DC过流故障
void FaultMonitor_DCCurrentOver(void)
{
    float32_t iBus_Temp;//直流电流实时值
    iBus_Temp = tCla2Cpu.Pcs_iBus_sensed_pu*PCS_IDC_MAX_SENSE_VOLTS;
//    if(m_st_TimerFlag.u16_b10ms == 1)
    {
        //DC过流
        if(iBus_Temp > (float32_t)Cpu1Ipc_cm2cpu.DC_OverCurr_Value*0.1f)
        {
            DCOverCurr_errCnt++;
        }
        else if(iBus_Temp < (float32_t)Cpu1Ipc_cm2cpu.DC_OverCurr_Value*0.1f-10.0f)
        {
            if(DCOverCurr_errCnt > 0)
                DCOverCurr_errCnt--;
            else
                DCOverCurr_errCnt = 0;
        }

        if(DCOverCurr_errCnt >= 50)
        {
            DCOverCurr_errCnt = 50;
            Cpu1Ipc_cpu2cm.FaultStatus.PCS_Fault2.tbits.bBUSIOverFault = 1;
        }
    }
}

//DC欠压故障
void FaultMonitor_DCVoltageUnder(void)
{
    if(m_st_TimerFlag.u16_b10ms == 1)
    {
        //DC欠压
        if(Pcs_Output_Meter.PCS_DC_BusVol < (float32_t)Cpu1Ipc_cm2cpu.DC_UnderVol_Value*0.1f)
        {
            DCUnderVol_errCnt++;
        }
        else if(Pcs_Output_Meter.PCS_DC_BusVol > (float32_t)Cpu1Ipc_cm2cpu.DC_UnderVol_Value*0.1f+20.0f)
        {
            if(DCUnderVol_errCnt > 0)
                DCUnderVol_errCnt--;
            else
                DCUnderVol_errCnt = 0;
        }

        if(DCUnderVol_errCnt >= 100)
        {
            DCUnderVol_errCnt = 100;
            Cpu1Ipc_cpu2cm.FaultStatus.PCS_Fault2.tbits.bBUSVUnderFault = 1;
        }
    }
}

//IGBT过温故障
void FaultMonitor_IgbtOverTemp(void)
{
    if(m_st_TimerFlag.u16_b50ms == 1)
    {
        if(Cpu1Ipc_cpu2cm.Temp_igbtMax > Cpu1Ipc_cm2cpu.TempIgbt_Over_Value)
        {
            IgbtOverTemp_errCnt++;
        }
        else if(Cpu1Ipc_cpu2cm.Temp_igbtMax < (Cpu1Ipc_cm2cpu.TempIgbt_Over_Value - 100))
        {
            if(IgbtOverTemp_errCnt > 0)
                IgbtOverTemp_errCnt--;
            else
                IgbtOverTemp_errCnt = 0;
        }

        if(IgbtOverTemp_errCnt >= 10)
        {
            IgbtOverTemp_errCnt = 10;
            Cpu1Ipc_cpu2cm.FaultStatus.PCS_Fault3.tbits.bIgbtTemptOverFault = 1;
        }
    }
}

//AMB环温过温故障
void FaultMonitor_AmbOverTemp(void)
{
    if(m_st_TimerFlag.u16_b50ms == 1)
    {
        if(Cpu1Ipc_cpu2cm.Temp_AmbOutlet > Cpu1Ipc_cm2cpu.TempAmb_Over_Value)
        {
            AmbOverTemp_errCnt++;
        }
        else if(Cpu1Ipc_cpu2cm.Temp_AmbOutlet < (Cpu1Ipc_cm2cpu.TempAmb_Over_Value - 100))
        {
            if(AmbOverTemp_errCnt > 0)
                AmbOverTemp_errCnt--;
            else
                AmbOverTemp_errCnt = 0;
        }

        if(AmbOverTemp_errCnt >= 10)
        {
            AmbOverTemp_errCnt = 10;
            Cpu1Ipc_cpu2cm.FaultStatus.PCS_Fault3.tbits.bAmbTemptOverFault = 1;
        }
    }
}

void FaultMonitor_DCRPFault(void)//极性反接故障
{
    if(m_st_TimerFlag.u16_b10ms == 1)
    {
        if(Pcs_Output_Meter.PCS_DC_BusVol < -30.0f)
        {
            DCVolRP_errCnt++;
        }
        else if(Pcs_Output_Meter.PCS_DC_BusVol > 0.0f)
        {
            if(DCVolRP_errCnt > 0)
                DCVolRP_errCnt--;
            else
                DCVolRP_errCnt = 0;
        }
        if(DCVolRP_errCnt >= 100)
        {
            DCVolRP_errCnt = 100;
            Cpu1Ipc_cpu2cm.FaultStatus.PCS_Fault2.tbits.bBUSReversedFault = 1;
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

void FaultMonitor_DCIMFault(void)
{
    static int16_t DCIMFault_errCnt = 0;
//    if(m_st_TimerFlag.u16_b50ms == 1)
//    {
//        if(0==Drv_DCIMStatusGet())
//        {
//            DCIMFault_errCnt++;
//        }
//        else
//        {
//            if(DCIMFault_errCnt > 0)
//                DCIMFault_errCnt--;
//            else
//                DCIMFault_errCnt = 0;
//        }
//        if(DCIMFault_errCnt >= 10)
//        {
//            DCIMFault_errCnt = 10;
//            Cpu1Ipc_cpu2cm.FaultStatus.PCS_Fault2.tbits.bDCIMFault = 1;
//        }
//    }
}

//电网电压故障 bGridACVFault
//void FaultMonitor_ACGridVFault(void)
//{
//    if((m_st_TimerFlag.u16_b10ms == 1)&&
//            (Pcs_Output_Meter.PCS_AC_VRms_PhaseA > 15.0f)&&(Pcs_Output_Meter.PCS_AC_VRms_PhaseB > 15.0f)&&(Pcs_Output_Meter.PCS_AC_VRms_PhaseC > 15.0f))
//    {
//
//        if((fabsf(Pcs_Output_Meter.PCS_AC_VRms_PhaseA-220.0f) > 33.0f)||
//           (fabsf(Pcs_Output_Meter.PCS_AC_VRms_PhaseB-220.0f) > 33.0f)||
//           (fabsf(Pcs_Output_Meter.PCS_AC_VRms_PhaseC-220.0f) > 33.0f))
//        {
//            ACFreq_errCnt++;
//        }
//        else if((fabsf(Pcs_Output_Meter.PCS_AC_VRms_PhaseA-220.0f) < 15.0f)||
//                (fabsf(Pcs_Output_Meter.PCS_AC_VRms_PhaseB-220.0f) < 15.0f)||
//                (fabsf(Pcs_Output_Meter.PCS_AC_VRms_PhaseC-220.0f) < 15.0f))
//        {
//            if(ACUnbalance_errCnt > 0)
//                ACFreq_errCnt--;
//            else
//                ACFreq_errCnt = 0;
//        }
//
//        if(ACFreq_errCnt >= 100)
//        {
//            ACFreq_errCnt = 100;
//            FaultStatus.PCS_Fault1.tbits.bGridACVFault = 1;
//        }
//    }
////    if(m_st_TimerFlag.u16_b10ms == 1)
////    {
////        if((Pcs_Output_Meter.PCS_AC_VRms_PhaseA > AC_GRID_VMAX)||
////           (Pcs_Output_Meter.PCS_AC_VRms_PhaseB > AC_GRID_VMAX)||
////           (Pcs_Output_Meter.PCS_AC_VRms_PhaseC > AC_GRID_VMAX))
////        {
////            ACGridOVol_errCnt++;
////        }
////        else if((Pcs_Output_Meter.PCS_AC_VRms_PhaseA < AC_GRID_VMAX)||
////                (Pcs_Output_Meter.PCS_AC_VRms_PhaseB < AC_GRID_VMAX)||
////                (Pcs_Output_Meter.PCS_AC_VRms_PhaseC < AC_GRID_VMAX))
////        {
////            if(ACGridOVol_errCnt > 0)
////                ACGridOVol_errCnt--;
////            else
////                ACGridOVol_errCnt = 0;
////        }
////
////        if(ACGridOVol_errCnt >= 50)
////        {
////            ACGridOVol_errCnt = 50;
////            FaultStatus.PCS_Fault1.tbits.bGridACVFault = 1;
////        }
////
////        //欠压
////        if((Pcs_Output_Meter.PCS_AC_VRms_PhaseA < AC_GRID_VMIN)||
////           (Pcs_Output_Meter.PCS_AC_VRms_PhaseB < AC_GRID_VMIN)||
////           (Pcs_Output_Meter.PCS_AC_VRms_PhaseC < AC_GRID_VMIN))
////        {
////            ACGridUVol_errCnt++;
////        }
////        else if((Pcs_Output_Meter.PCS_AC_VRms_PhaseA > AC_GRID_VMIN)||
////                (Pcs_Output_Meter.PCS_AC_VRms_PhaseB > AC_GRID_VMIN)||
////                (Pcs_Output_Meter.PCS_AC_VRms_PhaseC > AC_GRID_VMIN))
////        {
////            if(ACGridUVol_errCnt > 0)
////                ACGridUVol_errCnt--;
////            else
////                ACGridUVol_errCnt = 0;
////        }
////
////        if(ACGridUVol_errCnt >= 50)
////        {
////            ACGridUVol_errCnt = 50;
////            FaultStatus.PCS_Fault1.tbits.bGridACVFault = 1;
////        }
////    }
//
//}

//故障处理
int16_t App_AllFault_Deal(void)
{
//    if(m_st_TimerFlag.u16_b1ms == 0)
//        return 0;
    if(Cpu1Ipc_cm2cpu.debugMode == 0)//调试模式
    {
          FaultMonitor_PhaseSequence();
          FaultMonitor_FanFault();
          FaultMonitor_DCIMFault();
    //   if(Pcs_gParam.PcsCtrlState == PcsOn_Mode)
            FaultMonitor_DCRPFault();
            FaultMonitor_ACOverVoltage();
            FaultMonitor_ACUnderVoltage();
            FaultMonitor_ACCurrentOver();
            FaultMonitor_ACVoltageUnbanlance();
    //        FaultMonitor_ACFreq();
            FaultMonitor_DCVoltageOver();
            FaultMonitor_DCCurrentOver();
    //        FaultMonitor_ACGridVFault();
    //        FaultMonitor_DCVoltageUnder();
            FaultMonitor_DCVoltageUnbanlance();
            FaultMonitor_IgbtOverTemp();
            FaultMonitor_AmbOverTemp();
            if((Cpu1Ipc_cpu2cm.FaultStatus.PCS_Fault1.PCS_Fault1_All != 0)||
               (Cpu1Ipc_cpu2cm.FaultStatus.PCS_Fault2.PCS_Fault2_All != 0)||
               (Cpu1Ipc_cpu2cm.FaultStatus.PCS_Fault3.PCS_Fault3_All != 0))
    //           (FaultStatus.PCS_HFault.PCS_HFault_All != 0))//(FaultStatus.PCS_HFault.PCS_HFault_All != 0)
                Cpu1Ipc_cpu2cm.FaultStatus.globalFault = 1;
    }
//有故障时封波，停止输出
   if(Cpu1Ipc_cpu2cm.FaultStatus.globalFault == 1)
   {
       Cpu1Ipc_cpu2cm.PcsOnAllowed = 0;
   }

    return 0;
}
