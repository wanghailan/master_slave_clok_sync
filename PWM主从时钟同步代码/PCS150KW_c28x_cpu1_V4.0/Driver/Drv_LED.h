/*
 * Drv_LED.h
 *
 *  Created on: 2024年9月11日
 *      Author: guowei680
 */

#ifndef DRIVER_DRV_LED_H_
#define DRIVER_DRV_LED_H_


void Drv_LedPin_Init(void);
void Drv_Led_On(int16_t _ch);
void Drv_Led_Off(int16_t _ch);
void Drv_Led_toggle(int16_t _ch);

#endif /* DRIVER_DRV_LED_H_ */
