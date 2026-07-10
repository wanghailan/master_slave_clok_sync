/*
 * Modbus_RTU.h
 *
 *  Created on: 2024年10月11日
 *      Author: guowei680
 */

#ifndef MODBUS_RTU_H_
#define MODBUS_RTU_H_

/*
Baud rate   Bit rate     Bit time    Character time   3.5 character times
  2400      2400 bits/s   417 us          4.6 ms          16 ms
  4800      4800 bits/s   208 us          2.3 ms          8.0 ms
  9600      9600 bits/s   104 us          1.2 ms          4.0 ms
 19200     19200 bits/s    52 us          573 us          2.0 ms
 38400     38400 bits/s    26 us          286 us          1.75 ms(1.0 ms)
 115200    115200 bit/s   8.7 us           95 us          1.75 ms(0.33 ms) 后面固定都为1750us
*/

#define  ModRxTimeOut         56 //PWM中断31.25us 1.75ms是35
#define  Mod_TXBUF_SIZE       210

#define RTU_ADDR    0x01

#define REG_STATUS_HOLDING_START        0
#define REG_CONTROL_HOLDING_START       100
#define REG_FAULT_HOLDING_START         200
#define REG_PARAM_HOLDING_START         300
#define REG_DEBUG_HOLDING_START         400
#define REG_KEY_HOLDING_START           500

#define REG_STATUS_HOLDING_LEN          40
#define REG_CONTROL_HOLDING_LEN         24
#define REG_FAULT_HOLDING_LEN           20
#define REG_PARAM_HOLDING_LEN           50
#define REG_DEBUG_HOLDING_LEN           32
#define REG_Key_HOLDING_LEN             1

void ModbusRtu1_Init(void);
int16_t ModbusRtu1_Poll(void);

void ModbusRtu2_Init(void);
int16_t ModbusRtu2_Poll(void);

int16_t upDataHoldingCBReg(void);

#endif /* Modbus_RTU.h */
