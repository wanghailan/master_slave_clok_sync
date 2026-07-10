/*
 * Drv_I2CA.h
 *
 *  Created on: 2024Äê9ÔÂ11ÈÕ
 *      Author: guowei680
 */

#ifndef DRIVER_DRV_I2CA_H_
#define DRIVER_DRV_I2CA_H_

void Drv_I2CAPin_Init(void);
void Drv_I2CA_Init(void);
int I2CA_WriteBuf(uint16_t DevAddr,uint16_t RegAddr, uint16_t len, uint16_t *pdata,uint16_t WaitUs);
int I2CA_ReadBuf(uint16_t DevAddr,uint16_t RegAddr, uint16_t len, uint16_t *pdata,uint16_t WaitUs);

#endif /* DRIVER_DRV_I2C_H_ */
