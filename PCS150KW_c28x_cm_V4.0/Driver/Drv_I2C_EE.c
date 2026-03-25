//#############################################################################
//
// FILE:   Drv_LED.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

#include <Driver/device/cm.h>
#include <Driver/device/driverlib_cm.h>
#include "bsp.h"

int EE_WriteBuf(uint8_t *_pWriteBuf, uint16_t _usAddress, uint16_t _usSize)
{
    uint16_t i,m;
    uint16_t usAddr;
    uint8_t AddrH,AddrL;

    do
    {
        Drv_Timer_usDelay(10);
    }while(I2CA_Busy == 1);
//  /*
//      写串行EEPROM不像读操作可以连续读取很多字节，每次写操作只能在同一个page。
//      对于24xx02，page size = 8
//      简单的处理方法为：按字节写操作模式，每写1个字节，都发送地址
//      为了提高连续写的效率: 本函数采用page wirte操作。
//  */
    if((_usAddress+_usSize) > EE_SIZE)//超过器件容量，退出
        goto cmd_fail;

    usAddr = _usAddress;

    for (i = 0; i < _usSize; i++)
    {
        // 当发送第1个字节或是页面首地址时，需要重新发起启动信号和地址 ,1个page是64个byte,所以数据地址低6位是一个page周期
        if ((i == 0) || ((usAddr & (EE_PAGE_SIZE - 1)) == 0))
        {
            AddrH = (uint8_t)((usAddr >> 8)&0xff);
            AddrL = (uint8_t)(usAddr&0xff);

            i2cA_Stop();//发停止信号，启动内部写操作
            /* 通过检查器件应答的方式，判断内部写操作是否完成, 一般小于 10ms
                CLK频率为200KHz时，查询次数为30次左右 */
            for (m = 0; m < 1000; m++)
            {
                i2cA_Start();// 发起I2C总线启动信号
                if (EE_ADDR_BYTES == 1)
                    i2cA_SendByte(EE_DEV_ADDR |(AddrH << 1)| I2C_WR);    // 高7bit是地址，bit0是读写控制位，0表示写，1表示读
                else
                    i2cA_SendByte(EE_DEV_ADDR | I2C_WR); // 高7bit是地址，bit0是读写控制位，0表示写，1表示读

                if (i2cA_WaitAck() == 0)
                    break;
            }
            if (m  >= 1000)
                goto cmd_fail;  // EEPROM器件写超时

            if (EE_ADDR_BYTES == 2)
            {
                i2cA_SendByte(AddrH);
                if (i2cA_WaitAck() != 0)
                    goto cmd_fail;  // EEPROM器件无应答
            }
                i2cA_SendByte(AddrL);
                if (i2cA_WaitAck() != 0)
                    goto cmd_fail;  // EEPROM器件无应答
        }

        i2cA_SendByte(_pWriteBuf[i]);//开始写入数据
        if (i2cA_WaitAck() != 0)
            goto cmd_fail;  // EEPROM器件无应答

        usAddr++;   // 地址增1
    }

    // 命令执行成功，发送I2C总线停止信号
    i2cA_Stop();
    return 1;

cmd_fail: // 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备
    i2cA_Stop();
    return 0;
}



int EE_ReadBuf(uint8_t *_pReadBuf, uint16_t _usAddress, uint16_t _usSize)
{
    uint16_t i,m;
    uint8_t AddrH,AddrL;

    do
    {
        Drv_Timer_usDelay(10);
    }while(I2CA_Busy == 1);

    if((_usAddress+_usSize) > EE_SIZE)//超过器件容量，退出
        goto cmd_fail;

    AddrH = (uint8_t)((_usAddress >> 8)&0xff);
    AddrL = (uint8_t)(_usAddress&0xff);
    i2cA_Stop();//发停止信号，启动内部写操作
// 采用串行EEPROM随即读取指令序列，连续读取若干字节
    for (m = 0; m < 1000; m++)
    {
        i2cA_Start();// 发起I2C总线启动信号
        if (EE_ADDR_BYTES == 1)
            i2cA_SendByte(EE_DEV_ADDR |(AddrH << 1)| I2C_WR);    // 高7bit是地址，bit0是读写控制位，0表示写，1表示读
        else
            i2cA_SendByte(EE_DEV_ADDR | I2C_WR); // 高7bit是地址，bit0是读写控制位，0表示写，1表示读

        if (i2cA_WaitAck() == 0)
            break;
    }
    if (m  >= 1000)
        goto cmd_fail;  // EEPROM器件写超时

    // 发送字节地址，24C16地址低8位在这里
    if (EE_ADDR_BYTES == 2)
    {
        i2cA_SendByte(AddrH);
        if (i2cA_WaitAck() != 0)
            goto cmd_fail;  // EEPROM器件无应答
    }
        i2cA_SendByte(AddrL);
        if (i2cA_WaitAck() != 0)
            goto cmd_fail;  // EEPROM器件无应答

    i2cA_Start();
    if (EE_ADDR_BYTES == 1)
       i2cA_SendByte(EE_DEV_ADDR |(AddrH << 1)| I2C_RD);    // 高7bit是地址，bit0是读写控制位，0表示写，1表示读
    else
       i2cA_SendByte(EE_DEV_ADDR | I2C_RD); // 高7bit是地址，bit0是读写控制位，0表示写，1表示读

    if (i2cA_WaitAck() != 0)
        goto cmd_fail;  //EEPROM器件无应答

    for (i = 0; i < _usSize; i++) //循环读取数据
    {
        _pReadBuf[i] = i2cA_ReadByte();  // 读1个字节

        // 每读完1个字节后，需要发送Ack， 最后一个字节不需要Ack，发Nack
        if (i != _usSize - 1)
            i2cA_Ack();  // 中间字节读完后，CPU产生ACK信号(驱动SDA = 0)
        else
            i2cA_NAck(); // 最后1个字节读完后，CPU产生NACK信号(驱动SDA = 1)
    }

    i2cA_Stop(); // 发送I2C总线停止信号
    return 1;   // 执行成功

cmd_fail: // 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备
    i2cA_Stop();// 发送I2C总线停止信号
    return 0;
}





