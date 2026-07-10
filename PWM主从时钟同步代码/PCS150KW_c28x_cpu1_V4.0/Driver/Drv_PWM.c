//#############################################################################
//
// FILE:   Drv_PWM.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

#include <Driver/device/device.h>
#include <Driver/device/driverlib.h>

#include "Drv_PWM.h"
#include "bsp.h"


// EPWM1_A - GPIO Settings
#define GPIO_PIN_EPWM1_A            0
#define EPWM1_EPWMA_GPIO            0
#define EPWM1_EPWMA_PIN_CONFIG      GPIO_0_EPWM1A
// EPWM1_B - GPIO Settings
#define GPIO_PIN_EPWM1_B            1
#define EPWM1_EPWMB_GPIO            1
#define EPWM1_EPWMB_PIN_CONFIG      GPIO_1_EPWM1B
// EPWM2_A - GPIO Settings
#define GPIO_PIN_EPWM2_A            2
#define EPWM2_EPWMA_GPIO            2
#define EPWM2_EPWMA_PIN_CONFIG      GPIO_2_EPWM2A
// EPWM2_B - GPIO Settings
#define GPIO_PIN_EPWM2_B            3
#define EPWM2_EPWMB_GPIO            3
#define EPWM2_EPWMB_PIN_CONFIG      GPIO_3_EPWM2B
// EPWM3_A - GPIO Settings
#define GPIO_PIN_EPWM3_A            4
#define EPWM3_EPWMA_GPIO            4
#define EPWM3_EPWMA_PIN_CONFIG      GPIO_4_EPWM3A
// EPWM3_B - GPIO Settings
#define GPIO_PIN_EPWM3_B            5
#define EPWM3_EPWMB_GPIO            5
#define EPWM3_EPWMB_PIN_CONFIG      GPIO_5_EPWM3B
// EPWM4_A - GPIO Settings
#define GPIO_PIN_EPWM4_A            151
#define EPWM4_EPWMA_GPIO            151
#define EPWM4_EPWMA_PIN_CONFIG      GPIO_151_EPWM4A
// EPWM4_B - GPIO Settings
#define GPIO_PIN_EPWM4_B            152
#define EPWM4_EPWMB_GPIO            152
#define EPWM4_EPWMB_PIN_CONFIG      GPIO_152_EPWM4B
// EPWM5_A - GPIO Settings
#define GPIO_PIN_EPWM5_A            8
#define EPWM5_EPWMA_GPIO            8
#define EPWM5_EPWMA_PIN_CONFIG      GPIO_8_EPWM5A
// EPWM5_B - GPIO Settings
#define GPIO_PIN_EPWM5_B            9
#define EPWM5_EPWMB_GPIO            9
#define EPWM5_EPWMB_PIN_CONFIG      GPIO_9_EPWM5B
// EPWM6_A - GPIO Settings
#define GPIO_PIN_EPWM6_A            10
#define EPWM6_EPWMA_GPIO            10
#define EPWM6_EPWMA_PIN_CONFIG      GPIO_10_EPWM6A
// EPWM6_B - GPIO Settings
#define GPIO_PIN_EPWM6_B            11
#define EPWM6_EPWMB_GPIO            11
#define EPWM6_EPWMB_PIN_CONFIG      GPIO_11_EPWM6B

// EPWM7_A - GPIO Settings  平锟斤拷锟斤拷瞎锟�
#define GPIO_PIN_EPWM7_A            12
#define EPWM7_EPWMA_GPIO            12
#define EPWM7_EPWMA_PIN_CONFIG      GPIO_12_EPWM7A
// EPWM7_B - GPIO Settings  平锟斤拷锟斤拷鹿锟�
#define GPIO_PIN_EPWM7_B            13
#define EPWM7_EPWMB_GPIO            13
#define EPWM7_EPWMB_PIN_CONFIG      GPIO_13_EPWM7B

// EPWM8_A - GPIO Settings   FAN_CTRL
#define GPIO_PIN_EPWM8_A            14
#define EPWM8_EPWMA_GPIO            14
#define EPWM8_EPWMA_PIN_CONFIG      GPIO_14_EPWM8A


#define EPWM1_TIMER_TBPRD           EPWM_TBPRD
#define EPWM2_TIMER_TBPRD           EPWM_TBPRD
#define EPWM3_TIMER_TBPRD           EPWM_TBPRD
#define EPWM4_TIMER_TBPRD           EPWM_TBPRD
#define EPWM5_TIMER_TBPRD           EPWM_TBPRD
#define EPWM6_TIMER_TBPRD           EPWM_TBPRD

#define EPWM_DEAD_TIME              250 //锟斤拷锟斤拷时锟斤拷2.4us 240*10

#define GPIO_PIN_PWMEN              97U  // GPIO number for PWMEN
#define GPIO_CFG_PWMEN              GPIO_97_GPIO97


void Drv_Pwm_PinMux_Init(void);
void Drv_PWM_SYNC_Init(void);
void Drv_PWM_ParamInit(void);

static void Drv_EPWMx_Init(uint32_t PwmXBase,uint32_t PwmPrd,uint32_t PhaseShift,uint32_t ComPareA,uint32_t ComPareB,int16_t DeadZEn,int16_t SyncEn);
static void Drv_EPWM1_ReferenceOutputInit(void);
//
// Globals to hold the ePWM information used in this example
//
void PWMEN_Open(void)
{
    GPIO_writePin(GPIO_PIN_PWMEN, 0);
}

void PWMEN_Close(void)
{
    GPIO_writePin(GPIO_PIN_PWMEN, 1);
}


//*****************************************************************************
//
// PINMUX Configurations
//
//*****************************************************************************
void Drv_PwmPin_Init(void)
{
    // EPWM1A -> EPWM1A Pinmux
    GPIO_setPinConfig(EPWM1_EPWMA_PIN_CONFIG);//閰嶇疆PIN澶嶇敤妯″紡
    GPIO_setPadConfig(EPWM1_EPWMA_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(EPWM1_EPWMA_GPIO, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(EPWM1_EPWMA_GPIO, GPIO_QUAL_SYNC);
    // EPWM1B
    GPIO_setPinConfig(EPWM1_EPWMB_PIN_CONFIG);
    GPIO_setPadConfig(EPWM1_EPWMB_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(EPWM1_EPWMB_GPIO, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(EPWM1_EPWMB_GPIO, GPIO_QUAL_SYNC);

    // EPWM2A -> myEPWM2 Pinmux
    GPIO_setPinConfig(EPWM2_EPWMA_PIN_CONFIG);
    GPIO_setPadConfig(EPWM2_EPWMA_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(EPWM2_EPWMA_GPIO, GPIO_QUAL_SYNC);
    // EPWM2B
    GPIO_setPinConfig(EPWM2_EPWMB_PIN_CONFIG);
    GPIO_setPadConfig(EPWM2_EPWMB_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(EPWM2_EPWMB_GPIO, GPIO_QUAL_SYNC);

    // EPWM3A -> myEPWM3 Pinmux
    GPIO_setPinConfig(EPWM3_EPWMA_PIN_CONFIG);
    GPIO_setPadConfig(EPWM3_EPWMA_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(EPWM3_EPWMA_GPIO, GPIO_QUAL_SYNC);
    // EPWM3B
    GPIO_setPinConfig(EPWM3_EPWMB_PIN_CONFIG);
    GPIO_setPadConfig(EPWM3_EPWMB_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(EPWM3_EPWMB_GPIO, GPIO_QUAL_SYNC);

    // EPWM4 -> myEPWM4 Pinmux
    GPIO_setPinConfig(EPWM4_EPWMA_PIN_CONFIG);
    GPIO_setPadConfig(EPWM4_EPWMA_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(EPWM4_EPWMA_GPIO, GPIO_QUAL_SYNC);
    // EPWM4B
    GPIO_setPinConfig(EPWM4_EPWMB_PIN_CONFIG);
    GPIO_setPadConfig(EPWM4_EPWMB_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(EPWM4_EPWMB_GPIO, GPIO_QUAL_SYNC);

    // EPWM5 -> myEPWM5 Pinmux
    GPIO_setPinConfig(EPWM5_EPWMA_PIN_CONFIG);
    GPIO_setPadConfig(EPWM5_EPWMA_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(EPWM5_EPWMA_GPIO, GPIO_QUAL_SYNC);
    // EPWM5B
    GPIO_setPinConfig(EPWM5_EPWMB_PIN_CONFIG);
    GPIO_setPadConfig(EPWM5_EPWMB_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(EPWM5_EPWMB_GPIO, GPIO_QUAL_SYNC);

    // EPWM6 -> myEPWM5 Pinmux
    GPIO_setPinConfig(EPWM6_EPWMA_PIN_CONFIG);
    GPIO_setPadConfig(EPWM6_EPWMA_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(EPWM6_EPWMA_GPIO, GPIO_QUAL_SYNC);
    // EPWM6B
    GPIO_setPinConfig(EPWM6_EPWMB_PIN_CONFIG);
    GPIO_setPadConfig(EPWM6_EPWMB_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(EPWM6_EPWMB_GPIO, GPIO_QUAL_SYNC);

    // EPWM7A
    GPIO_setPinConfig(EPWM7_EPWMA_PIN_CONFIG);
    GPIO_setPadConfig(EPWM7_EPWMA_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(EPWM7_EPWMA_GPIO, GPIO_QUAL_SYNC);
    // EPWM7B
    GPIO_setPinConfig(EPWM7_EPWMB_PIN_CONFIG);
    GPIO_setPadConfig(EPWM7_EPWMB_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(EPWM7_EPWMB_GPIO, GPIO_QUAL_SYNC);

    // EPWM8A
    GPIO_setPinConfig(EPWM8_EPWMA_PIN_CONFIG);
    GPIO_setPadConfig(EPWM8_EPWMA_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(EPWM8_EPWMA_GPIO, GPIO_QUAL_SYNC);

    // PWMEN, low active.
    GPIO_setPinConfig(GPIO_CFG_PWMEN);
    GPIO_setPadConfig(GPIO_PIN_PWMEN, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_PWMEN, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(GPIO_PIN_PWMEN, GPIO_QUAL_SYNC);
    GPIO_writePin(GPIO_PIN_PWMEN, 1);
}

void Drv_Pwm_Init(void)
{
    EALLOW;
    PWMEN_Close();//楂樼數骞充笉杈撳嚭,PWM纭紑鍏�
    Drv_PWM_SYNC_Init();
    Drv_PWM_ParamInit();
    Drv_PwmOffset();
    EDIS;
}

//*****************************************************************************
//
// SYNC Scheme Configurations
//
//*****************************************************************************
void Drv_PWM_SYNC_Init(void)
{
//    if(m_i16_MasterSlave == 1)//涓绘満妯″紡
//    {
//        SysCtl_setSyncOutputConfig(SYSCTL_SYNC_OUT_SRC_EPWM1SYNCOUT);
//        //璁剧疆GPIO5涓烘爣鍑嗘帹鎸借緭鍑�
//        GPIO_setDirectionMode(28, GPIO_DIR_MODE_OUT);
//        GPIO_setPadConfig(28, GPIO_PIN_TYPE_STD);
//        GPIO_setQualificationMode(28, GPIO_QUAL_ASYNC);
//        //璁剧疆GPIO28涓篨BAR_OUTPUT5
//        GPIO_setPinConfig(GPIO_28_OUTPUTXBAR5);
//
//        //璁剧疆XBAR_OUTPUT5鐨勪俊鍙锋簮涓篗UX14鐨凟XTSYNCOUT
//        XBAR_setOutputMuxConfig(XBAR_OUTPUT5, XBAR_OUT_MUX14_EXTSYNCOUT);
//        XBAR_enableOutputMux(XBAR_OUTPUT5,XBAR_MUX14);
//
//    }
//    else
//    {
//        // 璁剧疆EXTSYNCIN1淇″彿浣滀负EPWM1鐨勫悓姝ヤ俊鍙凤紱EXTSYNCIN1鍥哄畾鎺ュ埌XBAR_INPUT5锛孍XTSYNCIN2鍥哄畾鎺ュ埌XBAR_INPUT6
//        SysCtl_setSyncInputConfig(SYSCTL_SYNC_IN_EPWM1,SYSCTL_SYNC_IN_SRC_EXTSYNCIN1);
//
//        GPIO_setDirectionMode(28, GPIO_DIR_MODE_IN);
//        GPIO_setPadConfig(28, GPIO_PIN_TYPE_PULLUP);
//        GPIO_setQualificationMode(28, GPIO_QUAL_ASYNC);
//        GPIO_setPinConfig(GPIO_28_GPIO28);
//
//        XBAR_setInputPin(XBAR_INPUT5, 28);
//    }
    //
    // For EPWM1, the sync input is: SYSCTL_SYNC_IN_SRC_EXTSYNCIN1
    //
//    SysCtl_setSyncInputConfig(SYSCTL_SYNC_IN_EPWM4, SYSCTL_SYNC_IN_SRC_EPWM1SYNCOUT);
//    SysCtl_setSyncInputConfig(SYSCTL_SYNC_IN_EPWM7, SYSCTL_SYNC_IN_SRC_EPWM1SYNCOUT);
//    SysCtl_setSyncInputConfig(SYSCTL_SYNC_IN_ECAP1, SYSCTL_SYNC_IN_SRC_EPWM1SYNCOUT);
//    SysCtl_setSyncInputConfig(SYSCTL_SYNC_IN_ECAP4, SYSCTL_SYNC_IN_SRC_EPWM1SYNCOUT);
//    SysCtl_setSyncInputConfig(SYSCTL_SYNC_IN_ECAP6, SYSCTL_SYNC_IN_SRC_EPWM1SYNCOUT);
    //鍚屾杈撳嚭婧愭槸EPWM1SYNCOUT
    SysCtl_setSyncOutputConfig(SYSCTL_SYNC_OUT_SRC_EPWM1SYNCOUT);
    // SOCA
    SysCtl_enableExtADCSOCSource(SYSCTL_ADCSOC_SRC_PWM1SOCA);//璁剧疆SOCA鐨勬簮
    // SOCB
    SysCtl_enableExtADCSOCSource(0);
}
//*****************************************************************************
//
// EPWM Configurations
//
//*****************************************************************************

static void Drv_EPWMx_Init(uint32_t PwmXBase,uint32_t PwmPrd,uint32_t PhaseShift,uint32_t ComPareA,uint32_t ComPareB,int16_t DeadZEn,int16_t SyncEn)
{
    //EPWM閰嶇疆
    EPWM_setClockPrescaler(PwmXBase, EPWM_CLOCK_DIVIDER_1, EPWM_HSCLOCK_DIVIDER_1);
    EPWM_setTimeBasePeriod(PwmXBase, PwmPrd);   //璁剧疆瀹氭椂鍛ㄦ湡
    EPWM_setTimeBaseCounter(PwmXBase, 0);     //Clear counter
    EPWM_setTimeBaseCounterMode(PwmXBase, EPWM_COUNTER_MODE_UP_DOWN); //up down mode

//    EPWM_disablePhaseShiftLoad(PwmXBase); //鍏抽棴鐩镐綅鍋忕Щ鍔犺浇
    //褰撳悓姝ヤ俊鍙峰埌鏉ワ紝璁℃暟鍣↙oad鐩哥Щ鍊�
    EPWM_setPhaseShift(PwmXBase, PhaseShift);//鐩镐綅鍋忕Щ
    EPWM_enablePhaseShiftLoad(PwmXBase);
    // 璁剧疆SyncOut涓篠yncIn锛屽嵆PWM1浜х敓鐨勫悓姝ヤ俊鍙穊ypassP PWM2鐩存帴杈撳嚭
//    EPWM_setSyncOutPulseMode(PwmXBase, EPWM_SYNC_OUT_PULSE_ON_EPWMxSYNCIN);
//    EPWM_enableSyncOutPulseSource(PwmXBase, EPWM_SYNC_OUT_PULSE_ON_CNTR_ZERO);
    if(SyncEn)
    {
        //璁剧疆涓嶦PWM1鍚屾
        EPWM_setSyncInPulseSource(PwmXBase, EPWM_SYNC_IN_PULSE_SRC_SYNCOUT_EPWM1);
        // 璁剧疆鍚屾鍚庣殑Counter鏂瑰悜锛�
        EPWM_setCountModeAfterSync(PwmXBase, EPWM_COUNT_MODE_UP_AFTER_SYNC);
    }

    EPWM_setCounterCompareValue(PwmXBase, EPWM_COUNTER_COMPARE_A, ComPareA);
    EPWM_setCounterCompareValue(PwmXBase, EPWM_COUNTER_COMPARE_B, ComPareB);

    EPWM_setCounterCompareShadowLoadMode(PwmXBase, EPWM_COUNTER_COMPARE_A, EPWM_COMP_LOAD_ON_CNTR_ZERO_PERIOD);
    EPWM_setCounterCompareShadowLoadMode(PwmXBase, EPWM_COUNTER_COMPARE_B, EPWM_COMP_LOAD_ON_CNTR_ZERO_PERIOD);

    EPWM_setActionQualifierAction(PwmXBase, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
    EPWM_setActionQualifierAction(PwmXBase, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);
    EPWM_setActionQualifierAction(PwmXBase, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPB);
    EPWM_setActionQualifierAction(PwmXBase, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPB);
    //姝诲尯璁剧疆
    if(DeadZEn)
    {
        EPWM_setDeadBandCounterClock(PwmXBase,EPWM_DB_COUNTER_CLOCK_FULL_CYCLE);//鏃堕挓鍛ㄦ湡100M 10ns
        EPWM_setRisingEdgeDelayCount(PwmXBase,EPWM_DEAD_TIME);//璁剧疆姝诲尯鏃堕棿
        EPWM_setFallingEdgeDelayCount(PwmXBase,EPWM_DEAD_TIME);
        EPWM_setDeadBandDelayMode(PwmXBase,EPWM_DB_RED,true);//姝诲尯浣胯兘
        EPWM_setDeadBandDelayMode(PwmXBase,EPWM_DB_FED,true);
        EPWM_setRisingEdgeDeadBandDelayInput(PwmXBase,EPWM_DB_INPUT_EPWMA);//涓婂崌娌挎鍖鸿鏁板櫒杈撳叆閫変负EPWMA
        EPWM_setFallingEdgeDeadBandDelayInput(PwmXBase,EPWM_DB_INPUT_EPWMA);//涓嬮檷娌挎鍖鸿鏁板櫒杈撳叆閫変负EPWMA
        EPWM_setDeadBandDelayPolarity(PwmXBase,EPWM_DB_RED,EPWM_DB_POLARITY_ACTIVE_HIGH);//璁剧疆PWM涓婂崌娌挎鍖鸿鏁板悗涓嶅弽杞瀬鎬�
        EPWM_setDeadBandDelayPolarity(PwmXBase,EPWM_DB_FED,EPWM_DB_POLARITY_ACTIVE_LOW);//璁剧疆PWM涓嬮檷娌挎鍖鸿鏁板悗鍙嶈浆鏋佹��
    }
    //TZ灏侀攣PWM
//    EPWM_setTripZoneAction(PwmXBase, EPWM_TZ_ACTION_EVENT_TZA, EPWM_TZ_ACTION_LOW);
//    EPWM_enableTripZoneSignals(PwmXBase, EPWM_TZ_SIGNAL_OSHT1);

//    EPWM_setActionQualifierContSWForceShadowMode(PwmXBase,EPWM_AQ_SW_IMMEDIATE_LOAD);//鍏堝己鍒惰緭鍑轰綆锛屼笉杈撳嚭
//    EPWM_setActionQualifierContSWForceAction(PwmXBase,EPWM_AQ_OUTPUT_A,EPWM_AQ_SW_OUTPUT_LOW);
//    EPWM_setActionQualifierContSWForceAction(PwmXBase,EPWM_AQ_OUTPUT_B,EPWM_AQ_SW_OUTPUT_LOW);
    EPWM_setActionQualifierContSWForceShadowMode(PwmXBase,EPWM_AQ_SW_IMMEDIATE_LOAD);
    EPWM_setActionQualifierContSWForceAction(PwmXBase,EPWM_AQ_OUTPUT_A,EPWM_AQ_SW_DISABLED);
    EPWM_setActionQualifierContSWForceAction(PwmXBase,EPWM_AQ_OUTPUT_B,EPWM_AQ_SW_DISABLED);

    EPWM_disableChopper(PwmXBase);//鍏抽棴鏂╂尝鍔熻兘
}

static void Drv_EPWM1_Init(void)
{
/*    //褰揚WM1浜х敓鐨勫悓姝ヤ俊鍙峰埌鏉ワ紝璁℃暟鍣↙oad鐩哥Щ鍊�
//    EPWM_setPhaseShift(EPWM1_BASE, 0);//0搴�
//    EPWM_enablePhaseShiftLoad(EPWM1_BASE);
//    if(m_i16_MasterSlave == 1)//涓绘満妯″紡
    {
        EPWM_enableSyncOutPulseSource(EPWM1_BASE, EPWM_SYNC_OUT_PULSE_ON_CNTR_ZERO);
    }
//    else
//    {
//        // 璁剧疆鍚屾鍚嶤ounter鏂瑰悜
//        EPWM_setCountModeAfterSync(EPWM1_BASE, EPWM_COUNT_MODE_UP_AFTER_SYNC);
//        EPWM_setSyncOutPulseMode(EPWM1_BASE, EPWM_SYNC_OUT_PULSE_ON_EPWMxSYNCIN);
//        // 浣胯兘鐩镐綅瑁呰浇锛�
//        EPWM_setPhaseShift(EPWM1_BASE, 0);//0搴�
//        EPWM_enablePhaseShiftLoad(EPWM1_BASE);
//    }*/

    Drv_EPWMx_Init(EPWM1_BASE,EPWM1_TIMER_TBPRD,0,EPWM_TBPRD,EPWM_TBPRD,1,1);
    Drv_EPWM1_ReferenceOutputInit();

    // Disable SOCA
    EPWM_disableADCTrigger(EPWM1_BASE, EPWM_SOC_A);
    // Configure the SOC to occur on the ET_CTR_PRDZERO
    EPWM_setADCTriggerSource(EPWM1_BASE, EPWM_SOC_A, EPWM_SOC_TBCTR_ZERO_OR_PERIOD);//ADC鍑哄彂棰戠巼32KHZ
    EPWM_setADCTriggerEventPrescale(EPWM1_BASE, EPWM_SOC_A, 1);
    EPWM_enableADCTrigger(EPWM1_BASE, EPWM_SOC_A);
//    EPWM_enableInterrupt(EPWM1_BASE);
//    EPWM_setInterruptEventCount(EPWM1_BASE, 1);
}

static void Drv_EPWM1_ReferenceOutputInit(void)
{
    uint16_t cmpTicks = (uint16_t)(EPWM1_TIMER_TBPRD >> 1U);

    /*
     * EPWM1A/B are the visible reference PWM used by PPS sync verification.
     * This is normal PWM initialization, not scope-debug: PWM_SCOPE_DEBUG may
     * be disabled without losing the 16 kHz, 50% EPWM1A/B waveform.
     */
    GPIO_setPinConfig(GPIO_0_EPWM1A);
    GPIO_setPadConfig(0U, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(0U, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(0U, GPIO_QUAL_SYNC);
    GPIO_setPinConfig(GPIO_1_EPWM1B);
    GPIO_setPadConfig(1U, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(1U, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(1U, GPIO_QUAL_SYNC);

    EPWM_setClockPrescaler(EPWM1_BASE,
                           EPWM_CLOCK_DIVIDER_1,
                           EPWM_HSCLOCK_DIVIDER_1);
    EPWM_setTimeBasePeriod(EPWM1_BASE, EPWM1_TIMER_TBPRD);
    EPWM_setTimeBaseCounter(EPWM1_BASE, 0U);
    EPWM_setTimeBaseCounterMode(EPWM1_BASE, EPWM_COUNTER_MODE_UP_DOWN);
    EPWM_setPhaseShift(EPWM1_BASE, 0U);
    EPWM_disablePhaseShiftLoad(EPWM1_BASE);
    EPWM_setCountModeAfterSync(EPWM1_BASE, EPWM_COUNT_MODE_UP_AFTER_SYNC);
    EPWM_setCounterCompareShadowLoadMode(EPWM1_BASE,
                                         EPWM_COUNTER_COMPARE_A,
                                         EPWM_COMP_LOAD_ON_CNTR_ZERO_PERIOD);
    EPWM_setCounterCompareShadowLoadMode(EPWM1_BASE,
                                         EPWM_COUNTER_COMPARE_B,
                                         EPWM_COMP_LOAD_ON_CNTR_ZERO_PERIOD);
    EPWM_setCounterCompareValue(EPWM1_BASE, EPWM_COUNTER_COMPARE_A, cmpTicks);
    EPWM_setCounterCompareValue(EPWM1_BASE, EPWM_COUNTER_COMPARE_B, cmpTicks);

    EPWM_setAdditionalActionQualifierActionComplete(EPWM1_BASE,
                                                    EPWM_AQ_OUTPUT_A,
                                                    0U);
    EPWM_setAdditionalActionQualifierActionComplete(EPWM1_BASE,
                                                    EPWM_AQ_OUTPUT_B,
                                                    0U);
    EPWM_setActionQualifierActionComplete(EPWM1_BASE,
                                          EPWM_AQ_OUTPUT_A,
                                          EPWM_AQ_OUTPUT_HIGH_ZERO |
                                          EPWM_AQ_OUTPUT_LOW_PERIOD);
    EPWM_setActionQualifierActionComplete(EPWM1_BASE,
                                          EPWM_AQ_OUTPUT_B,
                                          EPWM_AQ_OUTPUT_HIGH_ZERO |
                                          EPWM_AQ_OUTPUT_LOW_PERIOD);
    EPWM_setActionQualifierSWAction(EPWM1_BASE,
                                    EPWM_AQ_OUTPUT_A,
                                    EPWM_AQ_OUTPUT_HIGH);
    EPWM_setActionQualifierSWAction(EPWM1_BASE,
                                    EPWM_AQ_OUTPUT_B,
                                    EPWM_AQ_OUTPUT_HIGH);
    EPWM_forceActionQualifierSWAction(EPWM1_BASE, EPWM_AQ_OUTPUT_A);
    EPWM_forceActionQualifierSWAction(EPWM1_BASE, EPWM_AQ_OUTPUT_B);

    EPWM_setActionQualifierContSWForceShadowMode(EPWM1_BASE,
                                                 EPWM_AQ_SW_IMMEDIATE_LOAD);
    EPWM_setActionQualifierContSWForceAction(EPWM1_BASE,
                                             EPWM_AQ_OUTPUT_A,
                                             EPWM_AQ_SW_DISABLED);
    EPWM_setActionQualifierContSWForceAction(EPWM1_BASE,
                                             EPWM_AQ_OUTPUT_B,
                                             EPWM_AQ_SW_DISABLED);

    EPWM_setDeadBandDelayMode(EPWM1_BASE, EPWM_DB_RED, false);
    EPWM_setDeadBandDelayMode(EPWM1_BASE, EPWM_DB_FED, false);
    EPWM_setDeadBandOutputSwapMode(EPWM1_BASE, EPWM_DB_OUTPUT_A, false);
    EPWM_setDeadBandOutputSwapMode(EPWM1_BASE, EPWM_DB_OUTPUT_B, false);
    EPWM_disableChopper(EPWM1_BASE);
    EPWM_disableTripZoneSignals(EPWM1_BASE, 0xFFFFU);
    EPWM_clearTripZoneFlag(EPWM1_BASE,
                           EPWM_TZ_INTERRUPT |
                           EPWM_TZ_FLAG_CBC |
                           EPWM_TZ_FLAG_OST |
                           EPWM_TZ_FLAG_DCAEVT1 |
                           EPWM_TZ_FLAG_DCAEVT2 |
                           EPWM_TZ_FLAG_DCBEVT1 |
                           EPWM_TZ_FLAG_DCBEVT2);
    EPWM_clearCycleByCycleTripZoneFlag(EPWM1_BASE,
                                       EPWM_TZ_CBC_FLAG_1 |
                                       EPWM_TZ_CBC_FLAG_2 |
                                       EPWM_TZ_CBC_FLAG_3 |
                                       EPWM_TZ_CBC_FLAG_4 |
                                       EPWM_TZ_CBC_FLAG_5 |
                                       EPWM_TZ_CBC_FLAG_6 |
                                       EPWM_TZ_CBC_FLAG_DCAEVT2 |
                                       EPWM_TZ_CBC_FLAG_DCBEVT2);
}

void Drv_PWM_ParamInit(void)
{
    Drv_EPWM1_Init();
    Drv_EPWMx_Init(EPWM2_BASE,EPWM2_TIMER_TBPRD,0,EPWM_TBPRD,EPWM_TBPRD,1,1);
    Drv_EPWMx_Init(EPWM3_BASE,EPWM3_TIMER_TBPRD,0,EPWM_TBPRD,EPWM_TBPRD,1,1);
    Drv_EPWMx_Init(EPWM4_BASE,EPWM4_TIMER_TBPRD,0,EPWM_TBPRD,EPWM_TBPRD,1,1);
    Drv_EPWMx_Init(EPWM5_BASE,EPWM5_TIMER_TBPRD,0,EPWM_TBPRD,EPWM_TBPRD,1,1);
    Drv_EPWMx_Init(EPWM6_BASE,EPWM6_TIMER_TBPRD,0,EPWM_TBPRD,EPWM_TBPRD,1,1);
    Drv_EPWMx_Init(EPWM7_BASE,EPWM6_TIMER_TBPRD,0,EPWM_TBPRD,EPWM_TBPRD,0,1);

//    EPWM_setActionQualifierContSWForceShadowMode(EPWM7_BASE,EPWM_AQ_SW_IMMEDIATE_LOAD);
//    EPWM_setActionQualifierContSWForceAction(EPWM7_BASE,EPWM_AQ_OUTPUT_A,EPWM_AQ_SW_OUTPUT_LOW); //寮哄埗杈撳嚭浣�
//    EPWM_setActionQualifierContSWForceAction(EPWM7_BASE,EPWM_AQ_OUTPUT_B,EPWM_AQ_SW_OUTPUT_LOW); //寮哄埗杈撳嚭浣�
}

void Drv_PwmOnset(void)
{
    bsp_clb_pwmEnCtrl(1);
    PWMEN_Open();
}

void Drv_PwmOffset(void)
{
    //关闭pwm
    bsp_clb_pwmEnCtrl(0);
    PWMEN_Close();
}


void FanPwm_Init(void)
{
    Drv_EPWMx_Init(EPWM8_BASE,EPWM8_TBPRD,0,EPWM8_TBPRD>>1,EPWM8_TBPRD>>1,0,0);
    FanPwm_SpeedCtrl(100);
    FanPwm_ON();
}

void FanPwm_ON(void)
{
    EPWM_setActionQualifierContSWForceShadowMode(EPWM9_BASE,EPWM_AQ_SW_IMMEDIATE_LOAD);
//    EPWM_setActionQualifierContSWForceAction(EPWM16_BASE,EPWM_AQ_OUTPUT_A,EPWM_AQ_SW_DISABLED);
    EPWM_setActionQualifierContSWForceAction(EPWM9_BASE,EPWM_AQ_OUTPUT_A,EPWM_AQ_SW_DISABLED);
//    EPWM_setActionQualifierContSWForceShadowMode(EPWM16_BASE,EPWM_AQ_SW_SH_LOAD_ON_CNTR_ZERO_PERIOD);
}

void FanPwm_OFF(void)
{
    EPWM_setActionQualifierContSWForceShadowMode(EPWM9_BASE,EPWM_AQ_SW_IMMEDIATE_LOAD);
//    EPWM_setActionQualifierContSWForceAction(EPWM16_BASE,EPWM_AQ_OUTPUT_A,EPWM_AQ_SW_OUTPUT_LOW); //寮哄埗杈撳嚭浣�
    EPWM_setActionQualifierContSWForceAction(EPWM9_BASE,EPWM_AQ_OUTPUT_A,EPWM_AQ_SW_OUTPUT_LOW); //寮哄埗杈撳嚭浣�
//    EPWM_setActionQualifierContSWForceShadowMode(EPWM16_BASE,EPWM_AQ_SW_SH_LOAD_ON_CNTR_ZERO_PERIOD);
}

void FanPwm_SpeedCtrl(int16_t duty)
{
    uint16_t PwmCmpDat;
    uint32_t PwmDutyBuf;

    PwmDutyBuf = (EPWM8_TBPRD*duty)/1000;
    PwmCmpDat = PwmDutyBuf;
    EPWM_setCounterCompareValue(EPWM8_BASE, EPWM_COUNTER_COMPARE_A, PwmCmpDat);
    EPWM_setCounterCompareValue(EPWM8_BASE, EPWM_COUNTER_COMPARE_B, PwmCmpDat);
}


