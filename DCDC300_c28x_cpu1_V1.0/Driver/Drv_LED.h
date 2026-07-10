/*
 * Drv_LED.h
 *
 *  Created on: 2024Äê9ÔÂ11ÈÕ
 *      Author: guowei680
 */

#ifndef DRIVER_DRV_LED_H_
#define DRIVER_DRV_LED_H_


void Drv_LedPin_Init(void);
void Drv_Led_On(void);
void Drv_Led_Off(void);
void Drv_Led_toggle(void);
void Drv_WorkLed_On(void);
void Drv_WorkLed_Off(void);
void Drv_WorkLed_toggle(void);
void Drv_FaultLed_On(void);
void Drv_FaultLed_Off(void);
void Drv_FaultLed_toggle(void);


#endif /* DRIVER_DRV_LED_H_ */
