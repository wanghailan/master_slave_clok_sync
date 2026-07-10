/*
 * PCS_SETTING.h
 *
 *  Created on: 2025年02月14日
 *      Author: guowei680
 */

#ifndef PCS_SETTING_H_
#define PCS_SETTING_H_


#define DCDC_VBAT_READ_AD           ADC_readResult(ADCDRESULT_BASE, ADC_SOC_NUMBER1)   //DC高压端电压采样  电池侧  H1_VDC
#define DCDC_VBUS_READ_AD           ADC_readResult(ADCDRESULT_BASE, ADC_SOC_NUMBER0)   //DC低压端电压采样  母线侧  L1_VDC

#define DCDC_VBATS_READ_AD          ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER3)   //DC高压端 电容侧电压采样    H2_VDC
#define DCDC_VBUSS_READ_AD          ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER2)   //DC低压端 电容测电压采样    L2_VDC

//#define DCDC_VBATP_READ_AD          ADC_readResult(ADCCRESULT_BASE, ADC_SOC_NUMBER1)   //DC高压正半电压采样  电池侧  VDC_A5
//#define DCDC_VBATN_READ_AD          ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER2)   //DC高压负半电压采样  电池侧  VDC_A6

#define DCDC_VDC7_READ_AD           ADC_readResult(ADCDRESULT_BASE, ADC_SOC_NUMBER4)   //DC预留电压采样
#define DCDC_VDC8_READ_AD           ADC_readResult(ADCDRESULT_BASE, ADC_SOC_NUMBER3)   //DC预留电压采样
#define DCDC_VDC9_READ_AD           ADC_readResult(ADCDRESULT_BASE, ADC_SOC_NUMBER2)   //DC预留电压采样

#define DCDC_IBAT_READ_AD           ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER0)   //DC高压端电流采样  电池侧  H_IDC_Asig
#define DCDC_IBUS_READ_AD           ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER1)   //DC低压端电流采样  母线侧  L_IDC_Asig

#define DCDC_IL1_READ_AD            ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER0)   //DC单元1电感电流采样  母线侧  Idc_1
#define DCDC_IL2_READ_AD            ADC_readResult(ADCCRESULT_BASE, ADC_SOC_NUMBER0)   //DC单元2电感电流采样  母线侧  Idc_2
#define DCDC_IL3_READ_AD            ADC_readResult(ADCCRESULT_BASE, ADC_SOC_NUMBER1)   //DC单元3电感电流采样  母线侧  Idc_3

#define DCDC_IDC5_READ_AD           ADC_readResult(ADCCRESULT_BASE, ADC_SOC_NUMBER0)   //DC预留电流采样 IDC_A2
#define DCDC_IDC6_READ_AD           ADC_readResult(ADCCRESULT_BASE, ADC_SOC_NUMBER3)   //DC预留电流采样 IDC_A6

#define DCDC_IGBT_READ_AD           ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER3)   //IGBT温度采样
#define DCDC_NTC2_READ_AD           ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER2)   //底板NTC2温度采样


//#define DCDC_CPU_SYS_CLOCK_FREQ_HZ      ((float32_t)200*1000000)
//#define DCDC_PWMSYSCLOCK_FREQ_HZ        ((float32_t)100*1000000)
//
//#define PCS_PWM_SWITCHING_FREQ_HZ      ((float32_t)32*1000)
//#define PCS_CNTRL_ISR_FREQ_RATIO       1
//#define PCS_ISR1_FREQUENCY_HZ          ((float32_t)PCS_PWM_SWITCHING_FREQ_HZ / PCS_CNTRL_ISR_FREQ_RATIO)

// Control Loop Design
//
#define PI                             ((float32_t)3.141592653589f)

#define INV_SOFTSTART_STEP              5
//
// 1.0/(2.0^16)
//
#define DC_ADC_PU_SCALE_FACTOR        ((float32_t)0.0000152587890625f)

//ADC带偏置参考电压
#define DC_ADC_PNREF_VOL               ((float32_t)1.5f) //ADC参考电压-1.5V~+1.5V
//ADC不带偏置参考电压
#define DC_ADC_P0REF_VOL               ((float32_t)3.0f) //ADC参考电压0~3V
//偏置
#define DC_OFFSET_PU                   ((float32_t)0.5f) //量程0~3V，偏置1.5V
//SPWM MAX RATIO
#define DC_PWM_MAX_RATIO               ((float32_t)0.95f) //量程-1~1
//SPWM MIN RATIO
#define DC_PWM_MIN_RATIO               ((float32_t)0.05f) //量程-1~1

//// 1/3
//#define PCS_fK1div3Cnst                 ((float32_t)0.33333333f)
//
//#define PCS_ISR2_FREQUENCY_HZ           ((float32_t)8000)


#define DC_VDC_MAX_SENSE_RATIO            ((float32_t)800.0f*DC_ADC_P0REF_VOL)      //直流电压采样 变比 1200/1.5
#define DC_IDC300_MAX_SENSE_RATIO         ((float32_t)200.0f*DC_ADC_P0REF_VOL)        //直流电流采样比例 HIT300  300/1.5
#define DC_IDC200_MAX_SENSE_RATIO         ((float32_t)133.3333f*DC_ADC_P0REF_VOL)        //直流电流采样比例 HIT200  200/1.5
#define DC_IDC100_MAX_SENSE_RATIO         ((float32_t)66.6667f*DC_ADC_P0REF_VOL)         //直流电流采样比例 HIT100  100/1.5

#define     KMeanPointCntCnst       50


#endif /* PCS_setting.h */
