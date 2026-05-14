/*
 * Drv_DO.h
 *
 *  Created on: 2024年9月11日
 *      Author: guowei680
 */

#ifndef DRV_DO_H_
#define DRV_DO_H_


enum
{
    Relay_SoftAC = 1,
    Relay_MastAC,
    Relay_SoftDC,
    Relay_MastDC,
    Relay_FANPwr,
    Led_Work,
    Led_Fault,
    Do1_Res,
    Do2_Res,
    Do3_Res,
    Do_Asel1,
    Do_Asel2
};


void Drv_DO_Init(void);
int16_t Drv_RelaySt_Get(int16_t ReIndex);
void Drv_RelaySet(int16_t RelayIndex,int16_t SetValue);
#endif /* DRV_DO_H_ */
