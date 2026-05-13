/*
 * OBJ_SETTING.h
 *
 *  Created on: 2025年02月14日
 *      Author: guowei680
 */

#ifndef OBJ_SETTING_H_
#define OBJ_SETTING_H_


#define PCS_VGRID_A_READ_AD    ADC_readResult(ADCDRESULT_BASE, ADC_SOC_NUMBER0)   //A相网侧相电压
#define PCS_VGRID_B_READ_AD    ADC_readResult(ADCCRESULT_BASE, ADC_SOC_NUMBER0)   //B相网侧相电压
#define PCS_VGRID_C_READ_AD    ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER0)   //C相网侧相电压

#define PCS_VINV_A_READ_AD     ADC_readResult(ADCDRESULT_BASE, ADC_SOC_NUMBER1)   //A相逆变相电压
#define PCS_VINV_B_READ_AD     ADC_readResult(ADCCRESULT_BASE, ADC_SOC_NUMBER1)   //B相逆变相电压
#define PCS_VINV_C_READ_AD     ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER1)   //C相逆变相电压

#define PCS_IGRID_A_READ_AD    ADC_readResult(ADCDRESULT_BASE, ADC_SOC_NUMBER2)   //A相网侧电流
#define PCS_IGRID_B_READ_AD    ADC_readResult(ADCCRESULT_BASE, ADC_SOC_NUMBER2)   //B相网侧电流
#define PCS_IGRID_C_READ_AD    ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER2)   //C相网侧电流

#define PCS_IINV_A_READ_AD     ADC_readResult(ADCDRESULT_BASE, ADC_SOC_NUMBER3)   //A相逆变电流
#define PCS_IINV_B_READ_AD     ADC_readResult(ADCCRESULT_BASE, ADC_SOC_NUMBER3)   //B相逆变电流
#define PCS_IINV_C_READ_AD     ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER3)   //C相逆变电流

#define PCS_VDCP_READ_AD       ADC_readResult(ADCDRESULT_BASE, ADC_SOC_NUMBER4)   //直流电压正采样
#define PCS_VDCN_READ_AD       ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER1)   //直流电压负采样
#define PCS_IDC_READ_AD        ADC_readResult(ADCCRESULT_BASE, ADC_SOC_NUMBER4)   //直流电流
#define PCS_VDC_READ_AD        ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER0)   //直流端口电压
#define PCS_IMN_READ_AD        ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER2)   //N线电流

#define PCS_NTC1_READ_AD       ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER3)   //IGBT NTC温度采样
#define PCS_NTC2_READ_AD       ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER4)   //机箱NTC温度采样
#define PCS_lVOL_READ_AD       ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER4)   //板上电压


#define PCS_CPU_SYS_CLOCK_FREQ_HZ      ((float32_t)200*1000000)
#define PCS_PWMSYSCLOCK_FREQ_HZ        ((float32_t)100*1000000)

#define PCS_PWM_SWITCHING_FREQ_HZ      ((float32_t)32*1000)
#define PCS_CNTRL_ISR_FREQ_RATIO       1
#define PCS_ISR1_FREQUENCY_HZ          ((float32_t)PCS_PWM_SWITCHING_FREQ_HZ / PCS_CNTRL_ISR_FREQ_RATIO)
#define PCS_PWM_PERIOD_TICKS           (PCS_PWMSYSCLOCK_FREQ_HZ / PCS_PWM_SWITCHING_FREQ_HZ)

#define PCS_AC_FREQ_HZ                 ((float32_t)50)
// Control Loop Design
//
#define PI                             ((float32_t)3.141592653589f)

#define INV_SOFTSTART_STEP              5
//
// 1.0/(2.0^16)
//
#define PCS_ADC_PU_SCALE_FACTOR        ((float32_t)0.0000152587890625f)

//ADC带偏置参考电压
#define PCS_ADC_PNREF_VOL               ((float32_t)1.5f) //ADC参考电压-1.5V~+1.5V
//ADC不带偏置参考电压
#define PCS_ADC_P0REF_VOL               ((float32_t)3.0f) //ADC参考电压0~3V
//偏置
#define PCS_OFFSET_PU                   ((float32_t)0.5f) //量程0~3V，偏置1.5V
//SPWM MAX RATIO
#define PCS_PWM_MAX_RATIO               ((float32_t)0.998f) //量程-1~1
//SPWM MIN RATIO
#define PCS_PWM_MIN_RATIO               ((float32_t)-0.998f) //量程-1~1



// Default Ref Values
//
#define PCS_VREF_DEFAULT                ((float32_t)0.835f)
#define PCS_IREF_DEFAULT                ((float32_t)0.005f)

// 1/3
#define PCS_fK1div3Cnst                 ((float32_t)0.33333333f)

#define PCSUNIVERSAL_GRID_MAX_FREQ      65
#define PCS_UNIVERSAL_GRID_MIN_FREQ     45
#define PCS_ISR2_FREQUENCY_HZ           ((float32_t)8000)

#define PCS_IGRID_MAX_SENSE_AMPS        ((float32_t)205.875f*PCS_ADC_PNREF_VOL)      //输出电流采样比例 300A~0.97561V
#define PCS_IINV_MAX_SENSE_AMPS         ((float32_t)205.875f*PCS_ADC_PNREF_VOL)      //逆变电流采样比例 300A~0.97561V
#define PCS_IBUSN_MAX_SENSE_AMPS        ((float32_t)137.25f*PCS_ADC_P0REF_VOL)      //N线电流采样比例 300A~0.97561V

#define PCS_VGRID_MAX_SENSE_VOLTS       ((float32_t)491.563f*PCS_ADC_PNREF_VOL)   //交流电网电压采样比例 200*10+62/8.06
#define PCS_VINV_MAX_SENSE_VOLTS        ((float32_t)491.563f*PCS_ADC_PNREF_VOL)   //交流逆变电压采样比例 200*10+62/8.06

#define PCS_VBATT_MAX_SENSE_VOLTS       ((float32_t)828.11245f*PCS_ADC_P0REF_VOL)  //直流电压采样比例
#define PCS_VDC_MAX_SENSE_VOLTS         ((float32_t)207.43f*PCS_ADC_P0REF_VOL)      //直流正负电压采样比例
#define PCS_IDC_MAX_SENSE_VOLTS         ((float32_t)205.875f*PCS_ADC_P0REF_VOL)      //直流电流采样比例 300A~0.97561V PCS_ADC_PNREF_VOL

#define     K50HzPointCntCnst       160            // fpwm/50 ISR=32KHz/4=8
//#define     K60HzPointCntCnst       133            // fpwm/60 ISR=32KHz/4=8



#endif /* PCS_setting.h */
