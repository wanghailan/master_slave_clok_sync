/*
 * Drv_I2C_EE.h
 *
 *  Created on: 2024Äê9ÔÂ11ÈÕ
 *      Author: guowei680
 */

#ifndef DRIVER_DRV_I2C_EE_H_
#define DRIVER_DRV_I2C_EE_H_

void Drv_I2CAPin_Init(void);
void Drv_I2C_EE_Init(void);
int I2C_EE_WriteBuf(uint16_t addr, uint16_t len, uint16_t *pdata);
int I2C_EE_ReadBuf(uint16_t addr, uint16_t len, uint16_t *pdata);

#endif /* DRIVER_DRV_I2C_FRAM_H_ */
