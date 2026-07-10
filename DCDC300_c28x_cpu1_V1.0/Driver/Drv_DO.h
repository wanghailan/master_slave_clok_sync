/*
 * Drv_DO.h
 *
 *  Created on: 2024Äê9ÔÂ11ÈÕ
 *      Author: guowei680
 */

#ifndef DRV_DO_H_
#define DRV_DO_H_

enum
{
    Relay_SoftBat = 1,
    Relay_MastBat,
    Relay_SoftBus,
    Relay_MastBus,
    Relay_FAN,
    Relay_WorkLed,
    Relay_FaultLed
};


void Drv_DO_Init(void);
int16_t Drv_RelaySt_Get(int16_t ReIndex);
void Drv_RelaySet(int16_t RelayIndex,int16_t SetValue);
void Drv_UpExtDo(void);
#endif /* DRV_DO_H_ */
