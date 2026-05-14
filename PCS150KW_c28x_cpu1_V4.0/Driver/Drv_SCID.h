/*
 * Drv_SCI.h
 *
 *  Created on: 2024年9月11日
 *      Author: guowei680
 */

#ifndef DRIVER_DRV_SCID_H_
#define DRIVER_DRV_SCID_H_

//SCID_RX
#define SCID_SCIRX_GPIO             46
#define SCID_SCIRX_PIN_CONFIG       GPIO_46_SCID_RX
//SCID_TX - GPIO Settings
#define SCID_SCITX_GPIO             47
#define SCID_SCITX_PIN_CONFIG       GPIO_47_SCID_TX
// SCID_485_OE
#define SCID_GPIO_PIN_OE            45U  // GPIO number for OEA
#define SCID_OE_PIN_CONFIG          GPIO_45_GPIO45  // "pinConfig" for OE

#define SCID_TxEn()     GPIO_writePin(SCID_GPIO_PIN_OE, 1)
#define SCID_RxEn()     GPIO_writePin(SCID_GPIO_PIN_OE, 0)

#define  SCID_BufSize     256


extern uint16_t    SCID_RxBuf[SCID_BufSize];
extern int16_t     SCID_RxCnt;
extern int32_t     SCID_RxTimeOut;

extern uint16_t    SCID_TxBuf[SCID_BufSize];
extern int16_t     SCID_TxCnt;
extern int16_t     SCID_TxFlag;

void    Drv_SCIDPin_Init(void);
void    Drv_SCIDInit(void);
void    Drv_SCID_WriteByte(uint16_t dat);
int16_t    Drv_SCID_WriteDataBuf(int16_t dLen,uint16_t *dBuf);
void    SciD_Poll(void);


#endif /* DRIVER_DRV_SCIC_H_ */
