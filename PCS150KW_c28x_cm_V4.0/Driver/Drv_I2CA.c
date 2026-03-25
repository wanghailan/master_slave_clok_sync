//#############################################################################
//
// FILE:   Drv_I2CA.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

#include <Driver/device/cm.h>
#include <Driver/device/driverlib_cm.h>
#include "bsp.h"

//CM 通过IO模拟I2C实现
#define I2CA_GPIO_PIN_SDA           31U  // GPIO number for I2C SDAA
#define I2CA_GPIO_PIN_SCL           32U  // GPIO number for I2C SCLA

static __inline void I2C_SCL_0(void)        {GPIO_writePin(I2CA_GPIO_PIN_SCL, 0);}
static __inline void I2C_SCL_1(void)        {GPIO_writePin(I2CA_GPIO_PIN_SCL, 1);}
static __inline void I2C_SDA_0(void)        {GPIO_writePin(I2CA_GPIO_PIN_SDA, 0);}
static __inline void I2C_SDA_1(void)        {GPIO_writePin(I2CA_GPIO_PIN_SDA, 1);}

static __inline void I2C_SCL_in(void)       {;}
static __inline void I2C_SCL_out(void)      {;}
static __inline void I2C_SDA_in(void)       {;}
static __inline void I2C_SDA_out(void)      {;}

#define I2C_SDA_READ()                      (GPIO_readPin(I2CA_GPIO_PIN_SDA))   // 读SDA口线状态
#define I2C_SCL_READ()                      (GPIO_readPin(I2CA_GPIO_PIN_SCL))   // 读SCL口线状态


int8_t  I2CA_Busy = 0;
//
// Function to configure I2C0.
//
void Drv_I2CA_Init(void)
{
 /*   // Enable the Master module.
    I2C_enableMaster(I2C0_BASE);

    // I2C configuration. Set up to transfer data at 100 Kbps.
    I2C_initMaster(I2C0_BASE,I2C_CLK_FREQ,false);

    // Enable the Slave module.
    I2C_enableSlave(I2C0_BASE);

    // Configure for internal loopback mode
    I2C_setSlaveAddress(I2C0_BASE,I2CA_EE_ADDR,I2C_MASTER_WRITE);
    I2C_setOwnSlaveAddress(I2C0_BASE,I2C_SLAVE_ADDR_PRIMARY,I2CA_EE_ADDR);
    I2C_disableLoopback(I2C0_BASE);

    // Configure and turn on the I2C0 slave interrupt.  The I2C_enableSlaveInt()
    // gives you the ability to only enable specific interrupts.  For this case
    // we are only interrupting when the slave device receives data.
//    I2C_enableSlaveInt(I2C0_BASE);
 * */

    Drv_Timer_usDelay(1000);

    i2cA_Stop();
}

//100khz是5us,
static void i2c_Delay(void)
{
    Drv_Timer_usDelay(2);
}

/*
*********************************************************************************************************
*   函 数 名: i2c_Start
*   功能说明: CPU发起I2C总线启动信号
*   形    参:  无
*   返 回 值: 无
*********************************************************************************************************
*/
void i2cA_Start(void)
{
    // 当SCL高电平时，SDA出现一个下跳沿表示I2C总线启动信号
    I2CA_Busy = 1;
    I2C_SDA_1();
    I2C_SCL_1();
    i2c_Delay();
    I2C_SDA_0();
    i2c_Delay();
    I2C_SCL_0();
    i2c_Delay();
}

/*
*********************************************************************************************************
*   函 数 名: i2c_Stop
*   功能说明: CPU发起I2C总线停止信号
*   形    参:  无
*   返 回 值: 无
*********************************************************************************************************
*/
void i2cA_Stop(void)
{
    // 当SCL高电平时，SDA出现一个上跳沿表示I2C总线停止信号
    I2C_SCL_0();
    I2C_SDA_0();//时钟为高时，数据从低到高变化时关闭总线
    i2c_Delay();
    I2C_SCL_1();
    i2c_Delay();
    I2C_SDA_1();//发送I2C总线结束信号
    i2c_Delay();
    I2CA_Busy = 0;
}

/*
*********************************************************************************************************
*   函 数 名: i2c_SendByte
*   功能说明: CPU向I2C总线设备发送8bit数据
*   形    参:  _ucByte ： 等待发送的字节
*   返 回 值: 无
*********************************************************************************************************
*/
void i2cA_SendByte(uint8_t _ucByte)
{
    uint8_t i;

    // 先发送字节的高位bit7
    for (i = 0; i < 8; i++)
    {
        if (_ucByte & 0x80)
        {
            I2C_SDA_1();
        }
        else
        {
            I2C_SDA_0();
        }
        i2c_Delay();
        I2C_SCL_1();
        i2c_Delay();
        I2C_SCL_0();
        if (i == 7)
        {
             I2C_SDA_1(); // 释放总线
        }
        _ucByte <<= 1;  // 左移一个bit
        i2c_Delay();
    }
}


/*
*********************************************************************************************************
*   函 数 名: i2c_ReadByte
*   功能说明: CPU从I2C总线设备读取8bit数据
*   形    参:  无
*   返 回 值: 读到的数据
*********************************************************************************************************
*/
uint8_t i2cA_ReadByte(void)
{
    uint8_t i;
    uint8_t value = 0;

    // 读到第1个bit为数据的bit7
//  I2C_SDA_1();//读取时先把SDA拉高
    I2C_SDA_in();
    for (i = 0; i < 8; i++)
    {
        value <<= 1;
        I2C_SCL_1();
        i2c_Delay();
        if (I2C_SDA_READ())
        {
            value++;
        }
        I2C_SCL_0();
        i2c_Delay();
    }
    I2C_SDA_out();
    return value;
}

/*
*********************************************************************************************************
*   函 数 名: i2c_WaitAck
*   功能说明: CPU产生一个时钟，并读取器件的ACK应答信号
*   形    参:  无
*   返 回 值: 返回0表示正确应答，1表示无器件响应
*********************************************************************************************************
*/
uint8_t i2cA_WaitAck(void)
{
    uint8_t re;

    I2C_SDA_1();    // CPU释放SDA总线
    i2c_Delay();
    I2C_SCL_1();    // CPU驱动SCL = 1, 此时器件会返回ACK应答
    i2c_Delay();
    I2C_SDA_in();
    if (I2C_SDA_READ()) // CPU读取SDA口线状态
    {
        re = 1;
    }
    else
    {
        re = 0;
    }
    I2C_SCL_0();
    i2c_Delay();
    I2C_SDA_out();
    return re;
}

/*
*********************************************************************************************************
*   函 数 名: i2c_Ack
*   功能说明: CPU产生一个ACK信号
*   形    参:  无
*   返 回 值: 无
*********************************************************************************************************
*/
void i2cA_Ack(void)
{
    I2C_SDA_0();    // CPU驱动SDA = 0
    i2c_Delay();
    I2C_SCL_1();    // CPU产生1个时钟
    i2c_Delay();
    I2C_SCL_0();
    i2c_Delay();
    I2C_SDA_1();    // CPU释放SDA总线
}

/*
*********************************************************************************************************
*   函 数 名: i2c_NAck
*   功能说明: CPU产生1个NACK信号
*   形    参:  无
*   返 回 值: 无
*********************************************************************************************************
*/
void i2cA_NAck(void)
{
    I2C_SDA_1();    // CPU驱动SDA = 1
    i2c_Delay();
    I2C_SCL_1();    // CPU产生1个时钟
    i2c_Delay();
    I2C_SCL_0();
    i2c_Delay();
}

/*
*********************************************************************************************************
*   函 数 名: i2c_CheckDevice
*   功能说明: 检测I2C总线设备，CPU向发送设备地址，然后读取设备应答来判断该设备是否存在
*   形    参:  _Address：设备的I2C总线地址
*   返 回 值: 返回值 0 表示正确， 返回1表示未探测到
*********************************************************************************************************
*/
uint8_t i2c_CheckDevice(uint8_t _Address)
{
    uint8_t ucAck;
    I2C_SDA_in();
    I2C_SCL_in();
    if (I2C_SDA_READ() && I2C_SCL_READ())
    {
        I2C_SCL_out();
        I2C_SDA_out();
        i2cA_Start();        // 发送启动信号

        // 发送设备地址+读写控制bit（0 = w， 1 = r) bit7 先传
        i2cA_SendByte(_Address | I2C_WR);
        ucAck = i2cA_WaitAck();  // 检测设备的ACK应答

        i2cA_Stop();         // 发送停止信号

        return ucAck;
    }
    I2C_SCL_out();
    I2C_SDA_out();
    return 1;   // I2C总线异常
}



