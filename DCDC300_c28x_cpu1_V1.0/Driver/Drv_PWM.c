//#############################################################################
//
// FILE:   Drv_PWM.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

/* CODEMAP_PWM_C
 * Role: low-level ePWM pin mux, synchronization, dead-time, phase shift and PWM enable/disable.
 * CLA calculates duty; this driver configures the hardware that outputs the duty.
 */
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
//#define GPIO_PIN_EPWM3_A            4
//#define EPWM3_EPWMA_GPIO            4
//#define EPWM3_EPWMA_PIN_CONFIG      GPIO_4_EPWM3A
// EPWM3_B - GPIO Settings
//#define GPIO_PIN_EPWM3_B            5
//#define EPWM3_EPWMB_GPIO            5
//#define EPWM3_EPWMB_PIN_CONFIG      GPIO_5_EPWM3B
// EPWM4_A - GPIO Settings
#define GPIO_PIN_EPWM4_A            151
#define EPWM4_EPWMA_GPIO            151
#define EPWM4_EPWMA_PIN_CONFIG      GPIO_151_EPWM4A
// EPWM4_B - GPIO Settings
#define GPIO_PIN_EPWM4_B            152
#define EPWM4_EPWMB_GPIO            152
#define EPWM4_EPWMB_PIN_CONFIG      GPIO_152_EPWM4B
// EPWM5_A - GPIO Settings
//#define GPIO_PIN_EPWM5_A            8
//#define EPWM5_EPWMA_GPIO            8
//#define EPWM5_EPWMA_PIN_CONFIG      GPIO_8_EPWM5A
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
// EPWM7_A - GPIO Settings
//#define GPIO_PIN_EPWM7_A            12
//#define EPWM7_EPWMA_GPIO            12
//#define EPWM7_EPWMA_PIN_CONFIG      GPIO_12_EPWM7A
// EPWM7_B - GPIO Settings
#define GPIO_PIN_EPWM7_B            13
#define EPWM7_EPWMB_GPIO            13
#define EPWM7_EPWMB_PIN_CONFIG      GPIO_13_EPWM7B
// EPWM8_A - GPIO Settings
#define GPIO_PIN_EPWM8_A            14
#define EPWM8_EPWMA_GPIO            14
#define EPWM8_EPWMA_PIN_CONFIG      GPIO_14_EPWM8A
// EPWM8_B - GPIO Settings
#define GPIO_PIN_EPWM8_B            15
#define EPWM8_EPWMB_GPIO            15
#define EPWM8_EPWMB_PIN_CONFIG      GPIO_15_EPWM8B
/*
// EPWM9_A - GPIO Settings
#define GPIO_PIN_EPWM9_A            16
#define EPWM9_EPWMA_GPIO            16
#define EPWM9_EPWMA_PIN_CONFIG      GPIO_16_EPWM9A
// EPWM9_B - GPIO Settings
#define GPIO_PIN_EPWM9_B            17
#define EPWM9_EPWMB_GPIO            17
#define EPWM9_EPWMB_PIN_CONFIG      GPIO_17_EPWM9B
// EPWM10_A - GPIO Settings
#define GPIO_PIN_EPWM10_A            18
#define EPWM10_EPWMA_GPIO            18
#define EPWM10_EPWMA_PIN_CONFIG      GPIO_18_EPWM10A
// EPWM10_B - GPIO Settings
#define GPIO_PIN_EPWM10_B            19
#define EPWM10_EPWMB_GPIO            19
#define EPWM10_EPWMB_PIN_CONFIG      GPIO_19_EPWM10B
// EPWM11_A - GPIO Settings
#define GPIO_PIN_EPWM11_A            20
#define EPWM11_EPWMA_GPIO            20
#define EPWM11_EPWMA_PIN_CONFIG      GPIO_20_EPWM11A
// EPWM11_B - GPIO Settings
#define GPIO_PIN_EPWM11_B            21
#define EPWM11_EPWMB_GPIO            21
#define EPWM11_EPWMB_PIN_CONFIG      GPIO_21_EPWM11B
// EPWM12_A - GPIO Settings
#define GPIO_PIN_EPWM12_A            22
#define EPWM12_EPWMA_GPIO            22
#define EPWM12_EPWMA_PIN_CONFIG      GPIO_22_EPWM12A
// EPWM12_B - GPIO Settings
#define GPIO_PIN_EPWM12_B            23
#define EPWM12_EPWMB_GPIO            23
#define EPWM12_EPWMB_PIN_CONFIG      GPIO_23_EPWM12B
*/

#define EPWM_DEAD_TIME              200U //����ʱ��1us 200/200

//#define GPIO_PIN_PWMEN2            153U  // GPIO number for PWMEN2

#define GPIO_PIN_PWMEN              97U  // GPIO number for PWMEN
#define GPIO_CFG_PWMEN              GPIO_97_GPIO97  // "pinConfig" for PWMEN


void Drv_Pwm_PinMux_Init(void);
/* CODEMAP_PWM_SYNC: configure ePWM synchronization and phase relationships. Verify 120-degree interleaving here. */
void Drv_PWM_SYNC_Init(void);
/* CODEMAP_PWM_PARAM: initialize the active ePWM modules, initial phase shifts and compare values. */
void Drv_PWM_ParamInit(void);

static void Drv_EPWMx_Init(uint32_t PwmXBase,uint32_t PwmPrd,int32_t PhaseShift,uint32_t ComPareA,uint32_t ComPareB,int16_t DeadZEn,int16_t SyncEn,int16_t Enable);
//
// Globals to hold the ePWM information used in this example
//

/* CODEMAP_PWMEN_OPEN: enable external PWM gate/driver enable pin path. */
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
    GPIO_setPinConfig(GPIO_CFG_PWMEN);
    GPIO_setPadConfig(GPIO_PIN_PWMEN, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_PWMEN, GPIO_DIR_MODE_OUT);
    PWMEN_Open();
    // EPWM1A -> EPWM1A Pinmux
    GPIO_setPinConfig(EPWM1_EPWMA_PIN_CONFIG);//����PIN����ģʽ
    GPIO_setPadConfig(EPWM1_EPWMA_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(EPWM1_EPWMA_GPIO, GPIO_QUAL_SYNC);
    // EPWM1B
    GPIO_setPinConfig(EPWM1_EPWMB_PIN_CONFIG);
    GPIO_setPadConfig(EPWM1_EPWMB_GPIO, GPIO_PIN_TYPE_STD);
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
//    GPIO_setPinConfig(EPWM3_EPWMA_PIN_CONFIG);
//    GPIO_setPadConfig(EPWM3_EPWMA_GPIO, GPIO_PIN_TYPE_STD);
//    GPIO_setQualificationMode(EPWM3_EPWMA_GPIO, GPIO_QUAL_SYNC);
    // EPWM3B
//    GPIO_setPinConfig(EPWM3_EPWMB_PIN_CONFIG);
//    GPIO_setPadConfig(EPWM3_EPWMB_GPIO, GPIO_PIN_TYPE_STD);
//    GPIO_setQualificationMode(EPWM3_EPWMB_GPIO, GPIO_QUAL_SYNC);

    // EPWM4 -> myEPWM4 Pinmux
    GPIO_setPinConfig(EPWM4_EPWMA_PIN_CONFIG);
    GPIO_setPadConfig(EPWM4_EPWMA_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(EPWM4_EPWMA_GPIO, GPIO_QUAL_SYNC);
    // EPWM4B
    GPIO_setPinConfig(EPWM4_EPWMB_PIN_CONFIG);
    GPIO_setPadConfig(EPWM4_EPWMB_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(EPWM4_EPWMB_GPIO, GPIO_QUAL_SYNC);

    // EPWM5 -> myEPWM5 Pinmux
//    GPIO_setPinConfig(EPWM5_EPWMA_PIN_CONFIG);
//    GPIO_setPadConfig(EPWM5_EPWMA_GPIO, GPIO_PIN_TYPE_STD);
//    GPIO_setQualificationMode(EPWM5_EPWMA_GPIO, GPIO_QUAL_SYNC);
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
//    GPIO_setPinConfig(EPWM7_EPWMA_PIN_CONFIG);
//    GPIO_setPadConfig(EPWM7_EPWMA_GPIO, GPIO_PIN_TYPE_STD);
//    GPIO_setQualificationMode(EPWM7_EPWMA_GPIO, GPIO_QUAL_SYNC);
    // EPWM7B
    GPIO_setPinConfig(EPWM7_EPWMB_PIN_CONFIG);
    GPIO_setPadConfig(EPWM7_EPWMB_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(EPWM7_EPWMB_GPIO, GPIO_QUAL_SYNC);

    // EPWM8A
    GPIO_setPinConfig(EPWM8_EPWMA_PIN_CONFIG);
    GPIO_setPadConfig(EPWM8_EPWMA_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(EPWM8_EPWMA_GPIO, GPIO_QUAL_SYNC);
    // EPWM8B
    GPIO_setPinConfig(EPWM8_EPWMB_PIN_CONFIG);
    GPIO_setPadConfig(EPWM8_EPWMB_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(EPWM8_EPWMB_GPIO, GPIO_QUAL_SYNC);

//    // EPWM9A
//    GPIO_setPinConfig(EPWM9_EPWMA_PIN_CONFIG);
//    GPIO_setPadConfig(EPWM9_EPWMA_GPIO, GPIO_PIN_TYPE_STD);
//    GPIO_setQualificationMode(EPWM9_EPWMA_GPIO, GPIO_QUAL_SYNC);
}

/* CODEMAP_PWM_INIT: initialize PWM hardware, then force outputs off as the safe default. */
void Drv_Pwm_Init(void)
{
    EALLOW;
    PWMEN_Open();//�ߵ�ƽ�����,PWMӲ����
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
/* CODEMAP_PWM_SYNC: configure ePWM synchronization and phase relationships. Verify 120-degree interleaving here. */
void Drv_PWM_SYNC_Init(void)
{
//    if(m_i16_MasterSlave == 1)//����ģʽ
//    {
//        SysCtl_setSyncOutputConfig(SYSCTL_SYNC_OUT_SRC_EPWM1SYNCOUT);
//        //����GPIO5Ϊ��׼�������
//        GPIO_setDirectionMode(28, GPIO_DIR_MODE_OUT);
//        GPIO_setPadConfig(28, GPIO_PIN_TYPE_STD);
//        GPIO_setQualificationMode(28, GPIO_QUAL_ASYNC);
//        //����GPIO28ΪXBAR_OUTPUT5
//        GPIO_setPinConfig(GPIO_28_OUTPUTXBAR5);
//
//        //����XBAR_OUTPUT5���ź�ԴΪMUX14��EXTSYNCOUT
//        XBAR_setOutputMuxConfig(XBAR_OUTPUT5, XBAR_OUT_MUX14_EXTSYNCOUT);
//        XBAR_enableOutputMux(XBAR_OUTPUT5,XBAR_MUX14);
//
//    }
//    else
//    {
//        // ����EXTSYNCIN1�ź���ΪEPWM1��ͬ���źţ�EXTSYNCIN1�̶��ӵ�XBAR_INPUT5��EXTSYNCIN2�̶��ӵ�XBAR_INPUT6
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
    //ͬ�����Դ��EPWM1SYNCOUT
    SysCtl_setSyncOutputConfig(SYSCTL_SYNC_OUT_SRC_EPWM1SYNCOUT);
    // SOCA
    SysCtl_enableExtADCSOCSource(SYSCTL_ADCSOC_SRC_PWM1SOCA);//����SOCA��Դ
    // SOCB
    SysCtl_enableExtADCSOCSource(0);
}
//*****************************************************************************
//
// EPWM Configurations
//
//*****************************************************************************

static void Drv_EPWMx_Init(uint32_t PwmXBase,uint32_t PwmPrd,int32_t PhaseShift,uint32_t ComPareA,uint32_t ComPareB,int16_t DeadZEn,int16_t SyncEn,int16_t Enable)
{
    //EPWM����
    EPWM_setClockPrescaler(PwmXBase, EPWM_CLOCK_DIVIDER_1, EPWM_HSCLOCK_DIVIDER_1);
    EPWM_setTimeBasePeriod(PwmXBase, PwmPrd);   //���ö�ʱ����
    EPWM_setTimeBaseCounter(PwmXBase, 0);     //Clear counter
    EPWM_setTimeBaseCounterMode(PwmXBase, EPWM_COUNTER_MODE_UP_DOWN); //up down mode

    if(PwmXBase == EPWM1_BASE)
    {
//        EPWM_setPhaseShift(PwmXBase, 0);
//        EPWM_disablePhaseShiftLoad(PwmXBase);
        EPWM_enableSyncOutPulseSource(PwmXBase, EPWM_SYNC_OUT_PULSE_ON_CNTR_ZERO);
    }
    else
    {
        if(PwmXBase == EPWM2_BASE)
            EPWM_enableSyncOutPulseSource(PwmXBase, EPWM_SYNC_OUT_PULSE_ON_CNTR_ZERO);
        //��ͬ���źŵ�����������Load����ֵ
        EPWM_setPhaseShift(PwmXBase, PhaseShift);//��λƫ��
        EPWM_enablePhaseShiftLoad(PwmXBase);
    }

    if(SyncEn)
    {
        if(PwmXBase == EPWM2_BASE)
            //������EPWM1ͬ��
            EPWM_setSyncInPulseSource(PwmXBase, EPWM_SYNC_IN_PULSE_SRC_SYNCOUT_EPWM1);
        if(PwmXBase == EPWM4_BASE)
//            //������EPWM2ͬ��
            EPWM_setSyncInPulseSource(PwmXBase, EPWM_SYNC_IN_PULSE_SRC_SYNCOUT_EPWM2);
//             ����ͬ�����Counter����;
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
    //��������
    if(DeadZEn)
    {
        EPWM_setDeadBandCounterClock(PwmXBase,EPWM_DB_COUNTER_CLOCK_FULL_CYCLE);//ʱ������200M 5ns
        EPWM_setRisingEdgeDelayCount(PwmXBase,EPWM_DEAD_TIME);//��������ʱ��
        EPWM_setFallingEdgeDelayCount(PwmXBase,EPWM_DEAD_TIME);
        EPWM_setDeadBandDelayMode(PwmXBase,EPWM_DB_RED,true);//����ʹ��
        EPWM_setDeadBandDelayMode(PwmXBase,EPWM_DB_FED,true);
        EPWM_setRisingEdgeDeadBandDelayInput(PwmXBase,EPWM_DB_INPUT_EPWMA);//��������������������ѡΪEPWMA
        EPWM_setFallingEdgeDeadBandDelayInput(PwmXBase,EPWM_DB_INPUT_EPWMA);//�½�����������������ѡΪEPWMA
        EPWM_setDeadBandDelayPolarity(PwmXBase,EPWM_DB_RED,EPWM_DB_POLARITY_ACTIVE_HIGH);//����PWM���������������󲻷�ת����
        EPWM_setDeadBandDelayPolarity(PwmXBase,EPWM_DB_FED,EPWM_DB_POLARITY_ACTIVE_LOW);//����PWM�½�������������ת����
    }
    //TZ����PWM
//    EPWM_setTripZoneAction(PwmXBase, EPWM_TZ_ACTION_EVENT_TZA, EPWM_TZ_ACTION_LOW);
//    EPWM_enableTripZoneSignals(PwmXBase, EPWM_TZ_SIGNAL_OSHT1);

    if(Enable)
    {
        EPWM_setActionQualifierContSWForceShadowMode(PwmXBase,EPWM_AQ_SW_IMMEDIATE_LOAD);
        EPWM_setActionQualifierContSWForceAction(PwmXBase,EPWM_AQ_OUTPUT_A,EPWM_AQ_SW_DISABLED);
        EPWM_setActionQualifierContSWForceAction(PwmXBase,EPWM_AQ_OUTPUT_B,EPWM_AQ_SW_DISABLED);
    }
//    else
//    {
//        EPWM_setActionQualifierContSWForceShadowMode(PwmXBase,EPWM_AQ_SW_IMMEDIATE_LOAD);//ǿ������ͣ������
//        EPWM_setActionQualifierContSWForceAction(PwmXBase,EPWM_AQ_OUTPUT_A,EPWM_AQ_SW_OUTPUT_LOW);
//        EPWM_setActionQualifierContSWForceAction(PwmXBase,EPWM_AQ_OUTPUT_B,EPWM_AQ_SW_OUTPUT_LOW);
//    }

    EPWM_disableChopper(PwmXBase);//�ر�ն������
}

static void Drv_EPWM1_Init(void)
{
/*    //��PWM1������ͬ���źŵ�����������Load����ֵ
//    if(m_i16_MasterSlave == 1)//����ģʽ
    {
        EPWM_enableSyncOutPulseSource(EPWM1_BASE, EPWM_SYNC_OUT_PULSE_ON_CNTR_ZERO);
    }
//    else
//    {
//    }*/

    Drv_EPWMx_Init(EPWM1_BASE,EPWM_TBPRD,0,EPWM_TBPRD,EPWM_TBPRD,1,0,1);

    // Disable SOCA
    EPWM_disableADCTrigger(EPWM1_BASE, EPWM_SOC_A);
    // Configure the SOC to occur on the ET_CTR_PRDZERO
    EPWM_setADCTriggerSource(EPWM1_BASE, EPWM_SOC_A, EPWM_SOC_TBCTR_ZERO);//ADC����Ƶ��32KHZ
    EPWM_setADCTriggerEventPrescale(EPWM1_BASE, EPWM_SOC_A, 1);
    EPWM_enableADCTrigger(EPWM1_BASE, EPWM_SOC_A);
//    EPWM_enableInterrupt(EPWM1_BASE);
//    EPWM_setInterruptEventCount(EPWM1_BASE, 1);
}

/* CODEMAP_PWM_PARAM: initialize the active ePWM modules, initial phase shifts and compare values. */
void Drv_PWM_ParamInit(void)
{
    //EPWM1~4��δʹ�ã�����Ϊ��ʹ�����
    Drv_EPWM1_Init();
    Drv_EPWMx_Init(EPWM2_BASE,EPWM_TBPRD,(int32_t)EPWM_TBPRD ,EPWM_TBPRD,EPWM_TBPRD,1,1,1);// * 2 / 3(int32_t)EPWM_TBPRD
//    Drv_EPWMx_Init(EPWM3_BASE,EPWM_TBPRD,0,EPWM_TBPRD,EPWM_TBPRD,0,1,0);
    Drv_EPWMx_Init(EPWM4_BASE,EPWM_TBPRD,(int32_t)(EPWM_TBPRD*2/3) ,EPWM_TBPRD,EPWM_TBPRD,1,1,1);//*1/2

//    Drv_EPWMx_Init(EPWM5_BASE,EPWM_TBPRD,EPWM_TBPRD,EPWM_TBPRD,EPWM_TBPRD,0,1,0);
//    Drv_EPWMx_Init(EPWM6_BASE,EPWM_TBPRD,EPWM_TBPRD,EPWM_TBPRD,EPWM_TBPRD,0,1,0);
//    Drv_EPWMx_Init(EPWM7_BASE,EPWM_TBPRD,0,EPWM_TBPRD,EPWM_TBPRD,0,1,0);
//    Drv_EPWMx_Init(EPWM8_BASE,EPWM_TBPRD,0,EPWM_TBPRD,EPWM_TBPRD,0,1,0);
}

/* CODEMAP_PWM_ON: final software call that enables PWM output/gate path. */
void Drv_PwmOnset(void)
{
    bsp_clb_pwmEnCtrl(1);
//    PWMEN_Open();
}

/* CODEMAP_PWM_OFF: final software call that disables PWM output/gate path. Fault and stop paths must reach here. */
void Drv_PwmOffset(void)
{
    //�ر�PWM���
    bsp_clb_pwmEnCtrl(0);
//    PWMEN_Close();
}


//void FanPwm_Init(void)
//{
//    Drv_EPWMx_Init(EPWM9_BASE,EPWM9_TBPRD,0,EPWM9_TBPRD>>1,EPWM9_TBPRD>>1,0,0);
//    FanPwm_SpeedCtrl(100);
//    FanPwm_ON();
//}
//
//void FanPwm_ON(void)
//{
//    EPWM_setActionQualifierContSWForceShadowMode(EPWM9_BASE,EPWM_AQ_SW_IMMEDIATE_LOAD);
////    EPWM_setActionQualifierContSWForceAction(EPWM16_BASE,EPWM_AQ_OUTPUT_A,EPWM_AQ_SW_DISABLED);
//    EPWM_setActionQualifierContSWForceAction(EPWM9_BASE,EPWM_AQ_OUTPUT_A,EPWM_AQ_SW_DISABLED);
////    EPWM_setActionQualifierContSWForceShadowMode(EPWM16_BASE,EPWM_AQ_SW_SH_LOAD_ON_CNTR_ZERO_PERIOD);
//}
//
//void FanPwm_OFF(void)
//{
//    EPWM_setActionQualifierContSWForceShadowMode(EPWM9_BASE,EPWM_AQ_SW_IMMEDIATE_LOAD);
////    EPWM_setActionQualifierContSWForceAction(EPWM16_BASE,EPWM_AQ_OUTPUT_A,EPWM_AQ_SW_OUTPUT_LOW); //ǿ�������
//    EPWM_setActionQualifierContSWForceAction(EPWM9_BASE,EPWM_AQ_OUTPUT_A,EPWM_AQ_SW_OUTPUT_LOW); //ǿ�������
////    EPWM_setActionQualifierContSWForceShadowMode(EPWM16_BASE,EPWM_AQ_SW_SH_LOAD_ON_CNTR_ZERO_PERIOD);
//}

//void FanPwm_SpeedCtrl(int16_t duty)
//{
//    uint16_t PwmCmpDat;
//    uint32_t PwmDutyBuf;
//
//    PwmDutyBuf = (EPWM9_TBPRD*duty)/1000;
//    PwmCmpDat = PwmDutyBuf;
//    EPWM_setCounterCompareValue(EPWM9_BASE, EPWM_COUNTER_COMPARE_A, PwmCmpDat);
//    EPWM_setCounterCompareValue(EPWM9_BASE, EPWM_COUNTER_COMPARE_B, PwmCmpDat);
//}







