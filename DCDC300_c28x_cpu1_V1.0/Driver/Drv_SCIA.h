/*
 * Drv_SCI.h
 *
 *  Created on: 2024Äê9ÔÂ11ÈÕ
 *      Author: guowei680
 */

#ifndef DRIVER_DRV_SCIA_H_
#define DRIVER_DRV_SCIA_H_

//SCIA_RX
#define SCIA_SCIRX_GPIO             43
#define SCIA_SCIRX_PIN_CONFIG       GPIO_43_SCIA_RX
//SCIA_TX - GPIO Settings
#define SCIA_SCITX_GPIO             42
#define SCIA_SCITX_PIN_CONFIG       GPIO_42_SCIA_TX


#define  SCIA_BufSize     128


extern uint16_t    SCIA_RxBuf[SCIA_BufSize];
extern int16_t     SCIA_RxCnt;
extern int16_t     SCIA_RxTimeOut;

void   Drv_SCIAPin_Init(void);
void   Drv_SCIAInit(void);
void   Drv_SCIA_WriteByte(uint16_t dat);
void   Drv_SCIA_WriteDataBuf(int16_t dLen,uint16_t *dBuf);
__interrupt void SCIARxISR(void);


#endif /* DRIVER_DRV_SCIA_H_ */
