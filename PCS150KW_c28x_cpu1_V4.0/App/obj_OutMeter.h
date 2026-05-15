/*
 * obj_OutMeter.h
 *
 *  Created on: 2025年02月14日
 *      Author: guowei680
 */

#ifndef OBJ_OUTMETER_H_
#define OBJ_OUTMETER_H_

#include "bsp.h"


typedef struct _PCS_OUTPUT_DATA //
{
  float32_t   PCS_AC_VRms_PhaseA;        //A相电压，0.1V
  float32_t   PCS_AC_VRms_PhaseB;        //B相电压，0.1V
  float32_t   PCS_AC_VRms_PhaseC;        //C相电压，0.1V

  float32_t   PCS_AC_VRms_LineAB;        //AB线电压，0.1V
  float32_t   PCS_AC_VRms_LineBC;        //BC线电压，0.1V
  float32_t   PCS_AC_VRms_LineCA;        //CA线电压，0.1V

  float32_t   PCS_AC_VLRms_PhaseA;       //A相电压，0.1V
  float32_t   PCS_AC_VLRms_PhaseB;       //B相电压，0.1V
  float32_t   PCS_AC_VLRms_PhaseC;       //C相电压，0.1V

  float32_t   PCS_AC_IRms_A;             //A相电流，0.1A
  float32_t   PCS_AC_IRms_B;             //B相电流，0.1A
  float32_t   PCS_AC_IRms_C;             //C相电流，0.1A

  float32_t   PCS_AC_ILRms_A;             //A1相电流，0.1A
  float32_t   PCS_AC_ILRms_B;             //B1相电流，0.1A
  float32_t   PCS_AC_ILRms_C;             //C1相电流，0.1A

  float32_t   PCS_AC_PowerP_A;            //A相有功功率，0.01KW
  float32_t   PCS_AC_PowerP_B;            //A相有功功率，0.01KW
  float32_t   PCS_AC_PowerP_C;            //A相有功功率，0.01KW

  float32_t   PCS_AC_PowerQ_A;            //A相无功功率，0.01KVar
  float32_t   PCS_AC_PowerQ_B;            //B相无功功率，0.01KVar
  float32_t   PCS_AC_PowerQ_C;            //C相无功功率，0.01KVar

  float32_t   PCS_AC_PowerS_A;            //A相视在功率，0.01KVA
  float32_t   PCS_AC_PowerS_B;            //B相视在功率，0.01KVA
  float32_t   PCS_AC_PowerS_C;            //C相视在功率，0.01KVA

  float32_t   PCS_AC_PowerP;              //三相总有功功率，0.01KW
  float32_t   PCS_AC_PowerQ;              //三相总无功功率，0.01KVar
  float32_t   PCS_AC_PowerS;              //三相总视在功率，0.01KVA

  float32_t   PCS_AC_Freq_A;              //A相频率，0.01HZ
  float32_t   PCS_AC_Freq_B;              //B相频率，0.01HZ
  float32_t   PCS_AC_Freq_C;              //C相频率，0.01HZ

  float32_t   PCS_AC_PF_A;                //A相功率因数， 0.01
  float32_t   PCS_AC_PF_B;                //B相功率因数， 0.01
  float32_t   PCS_AC_PF_C;                //C相功率因数， 0.01

  float32_t   PCS_DC_BusVol;              //母线侧直流电压
  float32_t   PCS_DC_BusCurr;             //母线侧直流电流，0.01A
  float32_t   PCS_DC_Power;               //母线侧直流功率，0.01KW

  float32_t   PCS_DC_PBusVol;             //正母线电压，0.1V
  float32_t   PCS_DC_NBusVol;             //负母线电压，0.1V
  float32_t   PCS_DC_IBUS_N;              //N线电流，0.1A
}PCS_OUTPUT_DATA;

extern PCS_OUTPUT_DATA     Pcs_Output_Meter;

void  obj_PcsOutput_Init(void);
void  obj_PcsOutput_RmsCalc(void);
void  obj_PcsOutput_PowerMeasAC(void);
void  obj_PcsOutput_PowerMeas(void);
void  obj_PcsOutput_KWHDeal(void);
void  obj_PcsOutput_TempCalc(void);

#endif /* obj_Pcs_Output.h */
