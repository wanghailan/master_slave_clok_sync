/*
 * Drv_DIN.h
 *
 *  Created on: 2024Äê9ÔÂ11ÈÕ
 *      Author: guowei680
 */

#ifndef DRIVER_DRV_DIN_H_
#define DRIVER_DRV_DIN_H_

//enum
//{
//    Relay_SoftDC = 1,
//    Relay_MastDC,
//    Relay_SoftAC,
//    Relay_MastAC,
//    Relay_FAN
//};


void Drv_DinPin_Init(void);
int16_t Drv_EstopStatusGet(void);
int16_t Drv_FigbtStatusGet(void);

#endif /* DRIVER_DRV_DIN_H_ */
