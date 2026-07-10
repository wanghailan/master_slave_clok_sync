/*
 * obj_Pcs_Output.h
 *
 *  Created on: 2025年02月14日
 *      Author: guowei680
 */

#ifndef OBJ_PCS_OUTPUT_H_
#define OBJ_PCS_OUTPUT_H_

/* CODEMAP_OUTMETER_H
 * Output-meter data structure.
 * CLA produces per-unit sensed values; obj_OutMeter.c averages/scales them into engineering units.
 */
#include "bsp.h"


typedef struct _DCDC_OUTPUT_DATA //
{
  float32_t   DC_VBat_Mean;       //电池电压平均值，0.1V
  float32_t   DC_VBatS_Mean;      //电池软起电压平均值，0.1V

  float32_t   DC_VBus_Mean;       //母线电压平均值，0.1V
  float32_t   DC_VBusS_Mean;      //母线软起电压平均值，0.1V

  float32_t   DC_VBatP_Mean;      //电池电压P平均值，0.1V
  float32_t   DC_VBatN_Mean;      //电池电压N平均值，0.1V

  float32_t   DC_Vdc7_Mean;       //预留电压7平均值，0.1V
  float32_t   DC_Vdc8_Mean;       //预留电压8平均值，0.1V
  float32_t   DC_Vdc9_Mean;       //预留电压9平均值，0.1V

  float32_t   DC_IBat_Mean;       //电池电流平均值，0.1A
  float32_t   DC_IBus_Mean;       //网侧电流平均值，0.1A

  float32_t   DC_IL1_Mean;        //单元1电流平均值，0.1A
  float32_t   DC_IL2_Mean;        //单元2电流平均值，0.1A
  float32_t   DC_IL3_Mean;        //单元3电流平均值，0.1A

  float32_t   DC_Idc5_Mean;       //预留电流5平均值，0.1A
  float32_t   DC_Idc6_Mean;       //预留电流6平均值，0.1A

  float32_t   DC_PowerBat;        //电池侧功率，0.01KW
  float32_t   DC_PowerBus;        //母线侧功率，0.01KW

  float32_t   DC_IGBT_Temp1;      //IGBT温度1
  float32_t   DC_IGBT_Temp2;      //IGBT温度2
  float32_t   DC_IGBT_Temp3;      //IGBT温度3
  float32_t   DC_IGBT_Temp4;      //IGBT温度4
  float32_t   DC_IGBT_TempMax;    //IGBT温度最大值

  float32_t   DC_transfEffi;      //转换效率

  uint16_t    DC_HChargeKWH;      //充电电量，万度(KWH)
  uint16_t    DC_LChargeKWH;      //充电电量，度(KWH)
  uint16_t    DC_HdisChargeKWH;   //放电电量，万度(KWH)
  uint16_t    DC_LdisChargeKWH;   //放电电量，度(KWH)

  uint16_t    DC_RUN_TimeM;       //累计运行时间，分钟(S)
}DCDC_OUTPUT_DATA;

extern DCDC_OUTPUT_DATA     DC_OutMeter;

void  obj_OutputParam_Init(void);
void  obj_Power_MeanCalc(void);
void  obj_VolCurr_MeanCalc(void);
void  obj_Output_KWHDeal(void);

#endif /* obj_Pcs_Output.h */
