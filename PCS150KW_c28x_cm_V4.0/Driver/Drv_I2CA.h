/*
 * Drv_I2CA.h
 *
 *  Created on: 2024年9月11日
 *      Author: guowei680
 */

#ifndef DRV_I2CA_H_
#define DRV_I2CA_H_


#define I2C_WR  0       // 写控制bit
#define I2C_RD  1       // 读控制bit

extern int8_t  I2CA_Busy;

void i2cA_Delay(void);
void i2cA_Start(void);
void i2cA_Stop(void);
void i2cA_SendByte(uint8_t _ucByte);
uint8_t i2cA_ReadByte(void);
uint8_t i2cA_WaitAck(void);
void i2cA_Ack(void);
void i2cA_NAck(void);
uint8_t i2cA_CheckDevice(uint8_t _Address);
void Drv_I2CA_Init(void);

#endif /* DRV_I2CA_H_ */
