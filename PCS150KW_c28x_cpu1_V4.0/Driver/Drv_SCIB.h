/*
 * Drv_SCI.h
 *
 *  Created on: 2024Äê9ÔÂ11ÈÕ
 *      Author: guowei680
 */

#ifndef DRIVER_DRV_SCIB_H_
#define DRIVER_DRV_SCIB_H_

//SCIB_RX
#define SCIB_SCIRX_GPIO             71
#define SCIB_SCIRX_PIN_CONFIG       GPIO_71_SCIB_RX
//SCIB_TX - GPIO Settings
#define SCIB_SCITX_GPIO             70
#define SCIB_SCITX_PIN_CONFIG       GPIO_70_SCIB_TX

// SCIB_DE
#define SCIB_GPIO_PIN_DE            66
#define SCIB_DE_PIN_CONFIG          GPIO_66_GPIO66
// SCIB_RE
#define SCIB_GPIO_PIN_RE            64
#define SCIB_RE_PIN_CONFIG          GPIO_64_GPIO64

#define SCIB_TxEn()          GPIO_writePin(SCIB_GPIO_PIN_DE, 1)
#define SCIB_RxEn()          GPIO_writePin(SCIB_GPIO_PIN_RE, 0)

#define SCIB_TxDisable()     GPIO_writePin(SCIB_GPIO_PIN_DE, 0)
#define SCIB_RxDisable()     GPIO_writePin(SCIB_GPIO_PIN_RE, 1)

#define  SCIB_BufSize     128


extern uint16_t    SCIB_RxBuf[SCIB_BufSize];
extern int16_t     SCIB_RxCnt;
extern int32_t     SCIB_RxTimeOut;



void   Drv_SCIBPin_Init(void);
void   Drv_SCIBInit(void);
void   Drv_SCIB_WriteByte(uint16_t dat);
void   Drv_SCIB_WriteDataBuf(int16_t dLen,uint16_t *dBuf);
__interrupt void SCIBRxISR(void);



#endif /* DRIVER_DRV_SCIB_H_ */
