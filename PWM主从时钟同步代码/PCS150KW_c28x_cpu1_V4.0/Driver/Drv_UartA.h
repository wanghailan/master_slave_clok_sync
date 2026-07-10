/*
 * Drv_UARTA.h
 *
 *  Created on: 2024年9月11日
 *      Author: guowei680
 */

#ifndef DRIVER_DRV_UARTA_H_
#define DRIVER_DRV_UARTA_H_

//UARTA_RX
#define UARTA_RX_GPIO             43U
#define UARTA_RX_PIN_CONFIG       GPIO_43_UARTA_RX
//UARTA_TX - GPIO Settings
#define UARTA_TX_GPIO             42U
#define UARTA_TX_PIN_CONFIG       GPIO_42_UARTA_TX

void    Drv_UartAPin_Init(void);


#endif /* DRV_UARTA_H_ */
