//#############################################################################
//
// FILE:   Drv_ADC.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

#include <Driver/device/device.h>
#include <Driver/device/driverlib.h>

#include "bsp.h"

#define GPIO_PIN_ASel1        17U  // GPIO number for SEL1
#define GPIO_PIN_ASel2        16U  // GPIO number for SEL2

#define  ADC_TriggerSet      ADC_TRIGGER_EPWM1_SOCA
#define  ADC_AcqpsSet        (74U)   //63+1 SYSCLK cycles(5ns) 320ns最小

static int16_t  adSel_index = 0;
static int16_t  adSel_Cnt = 0;
uint16_t ad_ntc1[4];//IGBT NTC数据 0:平衡管，1:A相管，2:B相管，3:C相管
uint16_t ad_ntc2[4];//扩展NTC
uint16_t ad_vol[4];//板上电压

//void Drv_ASYSCTL_init(void);
void Drv_ADCInterrupt_init(void);
__interrupt void Drv_adcA1ISR(void);
void Drv_ADCA_init(void);
void Drv_ADCB_init(void);
void Drv_ADCC_init(void);
void Drv_ADCD_init(void);


void Drv_ADC_Init(void)
{
    EALLOW;
//    Drv_ASYSCTL_init();
    Drv_ADCA_init();
    Drv_ADCB_init();
    Drv_ADCC_init();
    Drv_ADCD_init();
    Drv_ADCInterrupt_init();
    adSel_index = 0;
    GPIO_writePin(GPIO_PIN_ASel1, 0);
    GPIO_writePin(GPIO_PIN_ASel2, 0);

    EDIS;
}

//
////*****************************************************************************
////
//// ASYSCTL Configurations
////
////*****************************************************************************
//void Drv_ASYSCTL_init(void)
//{
//    //
//    // asysctl initialization
//    //
//    // Disables the temperature sensor output to the ADC.
//    //
////    ASysCtl_disableTemperatureSensor();
//    ASysCtl_enableTemperatureSensor();
//    DEVICE_DELAY_US(500);
////    //
////    // Set the analog voltage reference selection to External.
////    //
////    ASysCtl_setAnalogReferenceExternal( ASYSCTL_VREFHIA | ASYSCTL_VREFHIB | ASYSCTL_VREFHIC );
////    //
////    // Set the internal analog voltage reference selection to 1.65V.
////    //
////    ASysCtl_setAnalogReference1P65( ASYSCTL_VREFHIA | ASYSCTL_VREFHIB | ASYSCTL_VREFHIC );
//}

//int16_t adc_16B_Deal(uint16_t AD_Raw,float ratio)
//{
//    int16_t TempDat;
//
////    TempDat = ((float)AD_Raw*(3.0f/65535.0f)*100.0f);
//    TempDat = (int16_t)((float)AD_Raw*ratio*0.00457771f);
//
//    return TempDat;
//}

//*****************************************************************************
//
// INTERRUPT Configurations
//
//*****************************************************************************
void Drv_ADCInterrupt_init(void)
{

    // Interrupt Setings for INT_ADCA1
    Interrupt_register(INT_ADCA1, &Drv_adcA1ISR);
    Interrupt_enable(INT_ADCA1);
}

//
//复用AD
void Drv_AD_Switch_Read(void)
{
    adSel_Cnt++;
    if((adSel_Cnt == 1))
    {
        if(adSel_index == 0)
        {
            GPIO_writePin(GPIO_PIN_ASel1, 0);
            GPIO_writePin(GPIO_PIN_ASel2, 0);
        }
        else if(adSel_index == 1)
        {
            GPIO_writePin(GPIO_PIN_ASel1, 1);
            GPIO_writePin(GPIO_PIN_ASel2, 0);
        }
        else if(adSel_index == 2)
        {
            GPIO_writePin(GPIO_PIN_ASel1, 0);
            GPIO_writePin(GPIO_PIN_ASel2, 1);
        }
        else
        {
            GPIO_writePin(GPIO_PIN_ASel1, 1);
            GPIO_writePin(GPIO_PIN_ASel2, 1);
        }
    }

    if((adSel_Cnt > 58)&&(adSel_Cnt < 66))
    {
        ad_ntc1[adSel_index] = PCS_NTC1_READ_AD;//IGBT      NTC采样
        ad_ntc2[adSel_index] = PCS_NTC2_READ_AD;//扩展        NTC采样
        ad_vol[adSel_index]  = PCS_lVOL_READ_AD;
    }

    if(adSel_Cnt >= 100)
    {
        adSel_index++;
        if(adSel_index > 3)
            adSel_index = 0;
        adSel_Cnt = 0;
    }
}




__interrupt void Drv_adcA1ISR(void)
{
    Pcs_runISR1();
//    EPWM_CBCTripZoneInt_Counter();
    Drv_AD_Switch_Read();
    Drv_Timer_IntMaintain();
    // Clear the interrupt flag
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);

    // Check if overflow has occurred
    if(true == ADC_getInterruptOverflowStatus(ADCA_BASE, ADC_INT_NUMBER1))
    {
        ADC_clearInterruptOverflowStatus(ADCA_BASE, ADC_INT_NUMBER1);
        ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);
    }

    // Acknowledge the interrupt
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}



//*****************************************************************************
//
// ADCA Configurations
//
//*****************************************************************************
void Drv_ADCA_init(void)
{
    // ADC Initialization: Write ADC configurations and power up the ADC
    // Configures the ADC module's offset trim
//    ADC_setVREF(ADCA_BASE,ADC_REFERENCE_EXTERNAL,ADC_REFERENCE_3_3V);
    // Configures the analog-to-digital converter module prescaler.
    ADC_setPrescaler(ADCA_BASE, ADC_CLK_DIV_4_0);//ADCCLK 200M/4 = 50MHZ
    // Configures the analog-to-digital converter resolution and signal mode.
    ADC_setMode(ADCA_BASE, ADC_RESOLUTION_16BIT, ADC_MODE_SINGLE_ENDED);
    // Sets the timing of the end-of-conversion pulse
    ADC_setInterruptPulseMode(ADCA_BASE, ADC_PULSE_END_OF_CONV);
    // Powers up the analog-to-digital converter core.
    ADC_enableConverter(ADCA_BASE);
    // Delay for 1ms to allow ADC time to power up
    DEVICE_DELAY_US(1000);
    //
    // SOC Configuration: Setup ADC EPWM channel and trigger settings
    // Disables SOC burst mode.
    ADC_disableBurstMode(ADCA_BASE);
    // Sets the priority mode of the SOCs.
    ADC_setSOCPriority(ADCA_BASE, ADC_PRI_THRU_SOC5_HIPRI);
    // Start of Conversion 0 Configuration

    // Configures a start-of-conversion (SOC) in the ADC and its interrupt SOC trigger.
    //      SOC number      : 6
    //      Trigger         : ADC_TriggerSet
    //      Channel         : ADC_CH_ADCIN0
    //      Sample Window   : 26 SYSCLK cycles
    //      Interrupt Trigger: ADC_INT_SOC_TRIGGER_NONE
    //
    //Select the channels to convert and end of conversion flag
/* *
 *  ADCA_SOC0  ------------ 直流母线电压采样(ADC_A4)
 *  ADCA_SOC1  ------------ 直流电压负采样(ADC_A5)
 *  ADCA_SOC2  ------------ N线电流采样(ADC_A2)
 *  ADCA_SOC3  ------------ IGBT温度四选一采样(ADC_A1)
 *  ADCA_SOC4  ------------ 备用四选一模拟采样(ADC_A0)
 *  */
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER0, ADC_TriggerSet, ADC_CH_ADCIN4, ADC_AcqpsSet);//采样保持窗 cycles
    ADC_setInterruptSOCTrigger(ADCA_BASE, ADC_SOC_NUMBER0, ADC_INT_SOC_TRIGGER_NONE);
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER1, ADC_TriggerSet, ADC_CH_ADCIN5, ADC_AcqpsSet);
    ADC_setInterruptSOCTrigger(ADCA_BASE, ADC_SOC_NUMBER1, ADC_INT_SOC_TRIGGER_NONE);
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER2, ADC_TriggerSet, ADC_CH_ADCIN2, ADC_AcqpsSet);
    ADC_setInterruptSOCTrigger(ADCA_BASE, ADC_SOC_NUMBER2, ADC_INT_SOC_TRIGGER_NONE);
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER3, ADC_TriggerSet, ADC_CH_ADCIN1, ADC_AcqpsSet);
    ADC_setInterruptSOCTrigger(ADCA_BASE, ADC_SOC_NUMBER3, ADC_INT_SOC_TRIGGER_NONE);
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER4, ADC_TriggerSet, ADC_CH_ADCIN3, ADC_AcqpsSet);
    ADC_setInterruptSOCTrigger(ADCA_BASE, ADC_SOC_NUMBER4, ADC_INT_SOC_TRIGGER_NONE);
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER5, ADC_TriggerSet, ADC_CH_ADCIN4, ADC_AcqpsSet);//采样保持窗 cycles
    ADC_setInterruptSOCTrigger(ADCA_BASE, ADC_SOC_NUMBER5, ADC_INT_SOC_TRIGGER_NONE);

    //
    // ADC Interrupt 1 Configuration
    //      SOC/EOC number  : 6
    //      Interrupt Source: enabled
    //      Continuous Mode : disabled
    ADC_setInterruptSource(ADCA_BASE, ADC_INT_NUMBER1, ADC_SOC_NUMBER5);
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);
    ADC_disableContinuousMode(ADCA_BASE, ADC_INT_NUMBER1);
    ADC_enableInterrupt(ADCA_BASE, ADC_INT_NUMBER1);
}

//*****************************************************************************
//
// ADCB Configurations
//
//*****************************************************************************
void Drv_ADCB_init(void)
{
//    ADC_setVREF(ADCB_BASE,ADC_REFERENCE_EXTERNAL,ADC_REFERENCE_3_3V);
    // Configures the analog-to-digital converter module prescaler.
    ADC_setPrescaler(ADCB_BASE, ADC_CLK_DIV_4_0);
    ADC_setMode(ADCB_BASE, ADC_RESOLUTION_16BIT, ADC_MODE_SINGLE_ENDED);
    // Sets the timing of the end-of-conversion pulse
    ADC_setInterruptPulseMode(ADCB_BASE, ADC_PULSE_END_OF_CONV);
    // Powers up the analog-to-digital converter core.
    ADC_enableConverter(ADCB_BASE);
    // Delay for 1ms to allow ADC time to power up
    DEVICE_DELAY_US(1000);
    //
    // SOC Configuration: Setup ADC EPWM channel and trigger settings
    // Disables SOC burst mode.
    ADC_disableBurstMode(ADCB_BASE);
    // Sets the priority mode of the SOCs.
    ADC_setSOCPriority(ADCB_BASE, ADC_PRI_THRU_SOC5_HIPRI);
    //
    // Start of Conversion 0 Configuration
    //Select the channels to convert and end of conversion flag
/* *
 *  ADCB_SOC0  ------------ C相网侧电压(ADC_B3)
 *  ADCB_SOC1  ------------ C相逆变电压(ADC_B1)
 *  ADCB_SOC2  ------------ 网侧C相电流(ADC_B2)
 *  ADCB_SOC3  ------------ 逆变C相电流(ADC_B5)
 *  ADCB_SOC4  ------------ 板上电压采样(ADC_B4)
 *  */
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER0, ADC_TriggerSet, ADC_CH_ADCIN3, ADC_AcqpsSet);
    ADC_setInterruptSOCTrigger(ADCB_BASE, ADC_SOC_NUMBER0, ADC_INT_SOC_TRIGGER_NONE);
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER1, ADC_TriggerSet, ADC_CH_ADCIN1, ADC_AcqpsSet);
    ADC_setInterruptSOCTrigger(ADCB_BASE, ADC_SOC_NUMBER1, ADC_INT_SOC_TRIGGER_NONE);
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER2, ADC_TriggerSet, ADC_CH_ADCIN2, ADC_AcqpsSet);
    ADC_setInterruptSOCTrigger(ADCB_BASE, ADC_SOC_NUMBER2, ADC_INT_SOC_TRIGGER_NONE);
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER3, ADC_TriggerSet, ADC_CH_ADCIN5, ADC_AcqpsSet);
    ADC_setInterruptSOCTrigger(ADCB_BASE, ADC_SOC_NUMBER3, ADC_INT_SOC_TRIGGER_NONE);
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER4, ADC_TriggerSet, ADC_CH_ADCIN4, ADC_AcqpsSet);
    ADC_setInterruptSOCTrigger(ADCB_BASE, ADC_SOC_NUMBER4, ADC_INT_SOC_TRIGGER_NONE);
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER5, ADC_TriggerSet, ADC_CH_ADCIN3, ADC_AcqpsSet);
    ADC_setInterruptSOCTrigger(ADCB_BASE, ADC_SOC_NUMBER5, ADC_INT_SOC_TRIGGER_NONE);
}

//*****************************************************************************
//
// ADCC Configurations
//
//*****************************************************************************
void Drv_ADCC_init(void)
{
//    ADC_setVREF(ADCC_BASE,ADC_REFERENCE_EXTERNAL,ADC_REFERENCE_3_3V);
    // Configures the analog-to-digital converter module prescaler.
    ADC_setPrescaler(ADCC_BASE, ADC_CLK_DIV_4_0); //ADCCLK 200M/4 = 50MHZ
    ADC_setMode(ADCC_BASE, ADC_RESOLUTION_16BIT, ADC_MODE_SINGLE_ENDED);
    // Sets the timing of the end-of-conversion pulse
    ADC_setInterruptPulseMode(ADCC_BASE, ADC_PULSE_END_OF_CONV);
    // Powers up the analog-to-digital converter core.
    ADC_enableConverter(ADCC_BASE);
    // Delay for 1ms to allow ADC time to power up
    DEVICE_DELAY_US(1000);
    //
    // SOC Configuration: Setup ADC EPWM channel and trigger settings
    // Disables SOC burst mode.
    ADC_disableBurstMode(ADCC_BASE);
    // Sets the priority mode of the SOCs.
    ADC_setSOCPriority(ADCC_BASE, ADC_PRI_THRU_SOC5_HIPRI);
    //
    // Start of Conversion 0 Configuration
    // Configures a start-of-conversion (SOC) in the ADC and its interrupt SOC trigger.
    //Select the channels to convert and end of conversion flag
/* *
 *  ADCC_SOC0  ------------ B相网侧电压(ADC_C3)
 *  ADCC_SOC1  ------------ B相逆变电压(ADC_15)
 *  ADCC_SOC2  ------------ 网侧B相电流(ADC_C2)
 *  ADCC_SOC3  ------------ 逆变B相电流(ADC_C5)
 *  ADCC_SOC4  ------------ 直流电流(ADC_C4)
 *  */
    ADC_setupSOC(ADCC_BASE, ADC_SOC_NUMBER0, ADC_TriggerSet, ADC_CH_ADCIN3, ADC_AcqpsSet);
    ADC_setInterruptSOCTrigger(ADCC_BASE, ADC_SOC_NUMBER0, ADC_INT_SOC_TRIGGER_NONE);
    ADC_setupSOC(ADCC_BASE, ADC_SOC_NUMBER1, ADC_TriggerSet, ADC_CH_ADCIN15, ADC_AcqpsSet);
    ADC_setInterruptSOCTrigger(ADCC_BASE, ADC_SOC_NUMBER1, ADC_INT_SOC_TRIGGER_NONE);
    ADC_setupSOC(ADCC_BASE, ADC_SOC_NUMBER2, ADC_TriggerSet, ADC_CH_ADCIN2, ADC_AcqpsSet);
    ADC_setInterruptSOCTrigger(ADCC_BASE, ADC_SOC_NUMBER2, ADC_INT_SOC_TRIGGER_NONE);
    ADC_setupSOC(ADCC_BASE, ADC_SOC_NUMBER3, ADC_TriggerSet, ADC_CH_ADCIN5, ADC_AcqpsSet);
    ADC_setInterruptSOCTrigger(ADCC_BASE, ADC_SOC_NUMBER3, ADC_INT_SOC_TRIGGER_NONE);
    ADC_setupSOC(ADCC_BASE, ADC_SOC_NUMBER4, ADC_TriggerSet, ADC_CH_ADCIN4, ADC_AcqpsSet);
    ADC_setInterruptSOCTrigger(ADCC_BASE, ADC_SOC_NUMBER4, ADC_INT_SOC_TRIGGER_NONE);
    ADC_setupSOC(ADCC_BASE, ADC_SOC_NUMBER5, ADC_TriggerSet, ADC_CH_ADCIN3, ADC_AcqpsSet);
    ADC_setInterruptSOCTrigger(ADCC_BASE, ADC_SOC_NUMBER5, ADC_INT_SOC_TRIGGER_NONE);
}

//*****************************************************************************
//
// ADCD Configurations
//
//*****************************************************************************
void Drv_ADCD_init(void)
{
//    ADC_setVREF(ADCC_BASE,ADC_REFERENCE_EXTERNAL,ADC_REFERENCE_3_3V);
    // Configures the analog-to-digital converter module prescaler.
    ADC_setPrescaler(ADCD_BASE, ADC_CLK_DIV_4_0);
    ADC_setMode(ADCD_BASE, ADC_RESOLUTION_16BIT, ADC_MODE_SINGLE_ENDED);
    // Sets the timing of the end-of-conversion pulse
    ADC_setInterruptPulseMode(ADCD_BASE, ADC_PULSE_END_OF_CONV);
    // Powers up the analog-to-digital converter core.
    ADC_enableConverter(ADCD_BASE);
    // Delay for 1ms to allow ADC time to power up
    DEVICE_DELAY_US(1000);
    //
    // SOC Configuration: Setup ADC EPWM channel and trigger settings
    // Disables SOC burst mode.
    ADC_disableBurstMode(ADCD_BASE);
    // Sets the priority mode of the SOCs.
    ADC_setSOCPriority(ADCD_BASE, ADC_PRI_THRU_SOC5_HIPRI);
    //
    // Start of Conversion 0 Configuration
    // Configures a start-of-conversion (SOC) in the ADC and its interrupt SOC trigger.
    //Select the channels to convert and end of conversion flag
/* *
 *  ADCD_SOC0  ------------ A相网侧电压(ADC_D3)
 *  ADCD_SOC1  ------------ A相逆变电压(ADC_D1)
 *  ADCD_SOC2  ------------ A相网侧电流(ADC_D2)
 *  ADCD_SOC3  ------------ A相逆变电流(ADC_D5)
 *  ADCD_SOC4  ------------ 直流电压正(ADC_D0)
 *  */
    ADC_setupSOC(ADCD_BASE, ADC_SOC_NUMBER0, ADC_TriggerSet, ADC_CH_ADCIN3, ADC_AcqpsSet);
    ADC_setInterruptSOCTrigger(ADCD_BASE, ADC_SOC_NUMBER0, ADC_INT_SOC_TRIGGER_NONE);
    ADC_setupSOC(ADCD_BASE, ADC_SOC_NUMBER1, ADC_TriggerSet, ADC_CH_ADCIN1, ADC_AcqpsSet);
    ADC_setInterruptSOCTrigger(ADCD_BASE, ADC_SOC_NUMBER1, ADC_INT_SOC_TRIGGER_NONE);
    ADC_setupSOC(ADCD_BASE, ADC_SOC_NUMBER2, ADC_TriggerSet, ADC_CH_ADCIN2, ADC_AcqpsSet);
    ADC_setInterruptSOCTrigger(ADCD_BASE, ADC_SOC_NUMBER2, ADC_INT_SOC_TRIGGER_NONE);
    ADC_setupSOC(ADCD_BASE, ADC_SOC_NUMBER3, ADC_TriggerSet, ADC_CH_ADCIN5, ADC_AcqpsSet);
    ADC_setInterruptSOCTrigger(ADCD_BASE, ADC_SOC_NUMBER3, ADC_INT_SOC_TRIGGER_NONE);
    ADC_setupSOC(ADCD_BASE, ADC_SOC_NUMBER4, ADC_TriggerSet, ADC_CH_ADCIN0, ADC_AcqpsSet);
    ADC_setInterruptSOCTrigger(ADCD_BASE, ADC_SOC_NUMBER4, ADC_INT_SOC_TRIGGER_NONE);
    ADC_setupSOC(ADCD_BASE, ADC_SOC_NUMBER5, ADC_TriggerSet,ADC_CH_ADCIN3 , ADC_AcqpsSet);
    ADC_setInterruptSOCTrigger(ADCD_BASE, ADC_SOC_NUMBER5, ADC_INT_SOC_TRIGGER_NONE);
}


