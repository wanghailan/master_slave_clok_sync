/*
 * modbus_Param.h
 *
 *  Created on: 2025年6月24日
 *      Author: guowei
 */

#ifndef __modbbus_PARAM_H__
#define __modbbus_PARAM_H__

#include <stdint.h>

/* ModBus 应答代码 */
#define RSP_OK              0       /* 成功 */
#define RSP_ERR_CMD         0x01    /* 不支持的功能码 */
#define RSP_ERR_REG_ADDR    0x02    /* 寄存器地址错误 */
#define RSP_ERR_VALUE       0x03    /* 数据值域错误 */
#define RSP_ERR_WRITE       0x04    /* 写入失败 */

#define REG_STATUS_HOLDING_START        0
#define REG_CONTROL_HOLDING_START       100
#define REG_FAULT_HOLDING_START         200
#define REG_PARAM_HOLDING_START         300
#define REG_DEBUG_HOLDING_START         400

#define REG_STATUS_HOLDING_LEN          60
#define REG_CONTROL_HOLDING_LEN         30
#define REG_FAULT_HOLDING_LEN           20
#define REG_PARAM_HOLDING_LEN           50
#define REG_DEBUG_HOLDING_LEN           50

extern uint16_t   status_RegHoldingBuf_w[REG_STATUS_HOLDING_LEN];//状态寄存器
extern uint16_t   control_RegHoldingBuf_w[REG_CONTROL_HOLDING_LEN];//控制寄存器
extern uint16_t   fault_RegHoldingBuf_w[REG_FAULT_HOLDING_LEN];//故障寄存器
extern uint16_t   param_RegHoldingBuf_w[REG_PARAM_HOLDING_LEN];//参数寄存器
extern uint16_t   debug_RegHoldingBuf_w[REG_DEBUG_HOLDING_LEN];//功能使能寄存器

int16_t upDataHoldingCBReg(void);
int16_t Param_WriteDataHoldingReg(uint16_t reg,int16_t value);
int16_t  Mb03_RegAddrLookUp(uint16_t regAddr,uint16_t regNum,uint32_t *regAddrBegin);

#endif
