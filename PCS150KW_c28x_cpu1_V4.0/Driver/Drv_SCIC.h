/*
 * Drv_SCI.h
 *
 *  Created on: 2024Äê9ÔÂ11ÈÕ
 *      Author: guowei680
 */

#ifndef DRIVER_DRV_SCIC_H_
#define DRIVER_DRV_SCIC_H_

//SCIC_RX
#define SCIC_SCIRX_GPIO             62U
#define SCIC_SCIRX_PIN_CONFIG       GPIO_62_SCIC_RX
//SCIC_TX - GPIO Settings
#define SCIC_SCITX_GPIO             63U
#define SCIC_SCITX_PIN_CONFIG       GPIO_63_SCIC_TX
// SCIC_485_OE
#define SCIC_GPIO_PIN_OE            44U  // GPIO number for OEA
#define SCIC_OE_PIN_CONFIG          GPIO_44_GPIO44  // "pinConfig" for OE

#define SCIC_TxEn()                 GPIO_writePin(SCIC_GPIO_PIN_OE, 1)
#define SCIC_RxEn()                 GPIO_writePin(SCIC_GPIO_PIN_OE, 0)

#define  SCIC_BufSize     256

extern uint16_t    SCIC_RxBuf[SCIC_BufSize];
extern int16_t     SCIC_RxCnt;
extern int32_t     SCIC_RxTimeOut;

extern uint16_t    SCIC_TxBuf[SCIC_BufSize];
extern int16_t     SCIC_TxCnt;
extern int16_t     SCIC_TxFlag;

void    Drv_SCICPin_Init(void);
void    Drv_SCICInit(void);
void    Drv_SCIC_WriteByte(uint16_t dat);
int16_t    Drv_SCIC_WriteDataBuf(int16_t dLen,uint16_t *dBuf);
__interrupt void SCICRxISR(void);
void    SciC_Poll(void);


#endif /* DRIVER_DRV_SCIC_H_ */
