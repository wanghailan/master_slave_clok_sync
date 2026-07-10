/*
 * obj_Protect.h
 *
 *  Created on: 2025年02月14日
 *      Author: guowei680
 */

#ifndef OBJ_PROTECT_H_
#define OBJ_PROTECT_H_

/* CODEMAP_PROTECT_H
 * Protection thresholds and protection API.
 * Threshold Modbus registers 208..218 map mainly to DCDC_Protect_Value fields.
 */
#include "Pcs_setting.h"

typedef struct _DCDC_Protect_Value //
{
    int16_t   Bat_OverVol_Value;        //高压侧过压保护阈值
    int16_t   Bat_UnderVol_Value;       //高压侧欠压保护阈值
    int16_t   Bus_OverVol_Value;        //低压侧过压保护阈值
    int16_t   Bus_UnderVol_Value;       //低压侧欠压保护阈值
    int16_t   Bat_OverCurr_Value;       //高压过流保护阈值
    int16_t   Bus_OverCurr_Value;       //低压过流保护阈值
    int16_t   Vol_Unbance_Value;        //正负电压不平衡报护阈值
    int16_t   Curr_Unbance_Value;       //电流不平衡报护阈值
    int16_t   Temp_Over_Value;          //IGBT过温保护阈值
    int16_t   Bat_VShort_Value;        //高压侧短路保护阈值
    int16_t   Bus_VShort_Value;        //低压侧短路保护阈值
}DCDC_Protect_Value;

extern DCDC_Protect_Value     DcDc_Protect_Threshold;

extern uint64_t  epwm1TZIntCount;

void PCS_HAL_setupCBCProtection();
void EPWM_CBCTripZoneInt_Counter(void);
int16_t App_AllFault_Deal(void);


#endif /* obj_Protect.h */
