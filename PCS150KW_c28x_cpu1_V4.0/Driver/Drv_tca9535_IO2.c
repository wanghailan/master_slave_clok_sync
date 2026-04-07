//#############################################################################
//
// FILE:   Drv_tca9535_IO1.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

#include <Driver/device/device.h>
#include <Driver/device/driverlib.h>
#include "bsp.h"

typedef enum {
    TCA9535_PIN0 = 0x01,
    TCA9535_PIN1 = 0x02,
    TCA9535_PIN2 = 0x04,
    TCA9535_PIN3 = 0x08,
    TCA9535_PIN4 = 0x10,
    TCA9535_PIN5 = 0x20,
    TCA9535_PIN6 = 0x40,
    TCA9535_PIN7 = 0x80,
    TCA9535_PIN_ALL = 0xFF
} TCA9535_Pin;

#define     UInt8       uint16_t
#define     UInt32      uint32_t

#define I2C_WR  0       // 写控制bit
#define I2C_RD  1       // 读控制bit

#define TCA9535_I2C_ADDR            (0x20 << 1)  //扩展芯片1 I2C地址

/************************** TCA9535 寄存器定义 **************************/
#define TCA9535_INPUT_PORT0         0x00  // 输入端口0寄存器（只读）
#define TCA9535_INPUT_PORT1         0x01  // 输入端口1寄存器（只读）
#define TCA9535_OUTPUT_PORT0        0x02  // 输出端口0寄存器（读写）
#define TCA9535_OUTPUT_PORT1        0x03  // 输出端口1寄存器（读写）
#define TCA9535_POLARITY_PORT0      0x04  // 极性反转端口0（0：原极性，1：反转）
#define TCA9535_POLARITY_PORT1      0x05  // 极性反转端口1
#define TCA9535_CONFIG_PORT0        0x06  // 配置端口0（0：输出，1：输入）
#define TCA9535_CONFIG_PORT1        0x07  // 配置端口1

// TCA9535PWR IO扩展控制接口1，通过IO模拟I2C实现
#define GPIO_PIN_IO2_SDA                    51U
#define GPIO_CFG_IO2_SDA                    GPIO_51_GPIO51
#define GPIO_PIN_IO2_SCL                    50U
#define GPIO_CFG_IO2_SCL                    GPIO_50_GPIO50
#define GPIO_PIN_IO2_INT                    49U
#define GPIO_CFG_IO2_INT                    GPIO_49_GPIO49


static __inline void I2C_SCL_0(void)        {GPIO_writePin(GPIO_PIN_IO2_SCL, 0);}
static __inline void I2C_SCL_1(void)        {GPIO_writePin(GPIO_PIN_IO2_SCL, 1);}
static __inline void I2C_SDA_0(void)        {GPIO_writePin(GPIO_PIN_IO2_SDA, 0);}
static __inline void I2C_SDA_1(void)        {GPIO_writePin(GPIO_PIN_IO2_SDA, 1);}

static __inline void I2C_SCL_in(void)       {GPIO_setDirectionMode(GPIO_PIN_IO2_SCL, GPIO_DIR_MODE_IN);}
static __inline void I2C_SCL_out(void)      {GPIO_setDirectionMode(GPIO_PIN_IO2_SCL, GPIO_DIR_MODE_OUT);}
static __inline void I2C_SDA_in(void)       {GPIO_setDirectionMode(GPIO_PIN_IO2_SDA, GPIO_DIR_MODE_IN);}
static __inline void I2C_SDA_out(void)      {GPIO_setDirectionMode(GPIO_PIN_IO2_SDA, GPIO_DIR_MODE_OUT);}

#define I2C_SDA_READ()                      (GPIO_readPin(GPIO_PIN_IO2_SDA))   // 读SDA口线状态
#define I2C_SCL_READ()                      (GPIO_readPin(GPIO_PIN_IO2_SCL))   // 读SCL口线状态

static void i2c_Delay(void);
static void i2c_Start(void);
static void i2c_Stop(void);
static void i2c_SendByte(UInt8 _ucByte);
static UInt8 i2c_ReadByte(void);
static UInt8 i2c_WaitAck(void);
static void i2c_Ack(void);
static void i2c_NAck(void);
static UInt8 i2c_CheckDevice(UInt8 _Address);
static uint8_t tca9535_WriteReg(uint8_t baseAddr, uint8_t _ucRegAddr, uint16_t _usValue);
static uint8_t tca9535_ReadReg(uint8_t baseAddr,uint8_t ReadAddr);


static void Drv_TCA9535_IO2_PinInit(void)
{
    //控制IO设置
    GPIO_setPinConfig(GPIO_CFG_IO2_SDA);
    GPIO_setPadConfig(GPIO_PIN_IO2_SDA, GPIO_PIN_TYPE_OD);
    GPIO_setDirectionMode(GPIO_PIN_IO2_SDA, GPIO_DIR_MODE_OUT);
    GPIO_setPinConfig(GPIO_CFG_IO2_SCL);
    GPIO_setPadConfig(GPIO_PIN_IO2_SCL, GPIO_PIN_TYPE_OD);
    GPIO_setDirectionMode(GPIO_PIN_IO2_SCL, GPIO_DIR_MODE_OUT);
    //中断控制IO设置
    GPIO_setPinConfig(GPIO_CFG_IO2_INT);
    GPIO_setPadConfig(GPIO_PIN_IO2_INT, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_IO2_INT, GPIO_DIR_MODE_IN);

}

static void i2c_Init(void)
{
    // 初始化I2C0引脚PINMUX
    Drv_TCA9535_IO2_PinInit();

    Drv_Timer_usDelay(1000);
    I2C_SDA_out();
    I2C_SCL_out();

    i2c_Stop();
}

/*
*********************************************************************************************************
*   函 数 名: bsp_InitI2C
*   功能说明: 配置I2C总线的GPIO，采用模拟IO的方式实现
*   形    参:  无
*   返 回 值: 无
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*   函 数 名: i2c_Delay
*   功能说明: I2C总线位延迟，最快400KHz
*   形    参:  无
*   返 回 值: 无
*********************************************************************************************************
*/
//static void Delay(volatile unsigned int count)
//{
//    while(count--);
//}
//
//static void delay_us(UInt32 num)
//{
//    while(num--)
//    {
//        Delay(0x1d);//约1us
//    }
//}

//100khz是5us,
static void i2c_Delay(void)
{
//    delay_us(2);
    Drv_Timer_usDelay(5);
}

/*
*********************************************************************************************************
*   函 数 名: i2c_Start
*   功能说明: CPU发起I2C总线启动信号
*   形    参:  无
*   返 回 值: 无
*********************************************************************************************************
*/
static void i2c_Start(void)
{
    // 当SCL高电平时，SDA出现一个下跳沿表示I2C总线启动信号
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
static void i2c_Stop(void)
{
    // 当SCL高电平时，SDA出现一个上跳沿表示I2C总线停止信号
    I2C_SCL_0();
    I2C_SDA_0();//时钟为高时，数据从低到高变化时关闭总线
    i2c_Delay();
    I2C_SCL_1();
    i2c_Delay();
    I2C_SDA_1();//发送I2C总线结束信号
    i2c_Delay();
}

/*
*********************************************************************************************************
*   函 数 名: i2c_SendByte
*   功能说明: CPU向I2C总线设备发送8bit数据
*   形    参:  _ucByte ： 等待发送的字节
*   返 回 值: 无
*********************************************************************************************************
*/
static void i2c_SendByte(UInt8 _ucByte)
{
    UInt8 i;

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
static UInt8 i2c_ReadByte(void)
{
    UInt8 i;
    UInt8 value = 0;

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
static UInt8 i2c_WaitAck(void)
{
    UInt8 re;

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
static void i2c_Ack(void)
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
static void i2c_NAck(void)
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
static UInt8 i2c2_CheckDevice(UInt8 _Address)
{
    UInt8 ucAck;
    I2C_SDA_in();
    I2C_SCL_in();
    if (I2C_SDA_READ() && I2C_SCL_READ())
    {
        I2C_SCL_out();
        I2C_SDA_out();
        i2c_Start();        // 发送启动信号

        // 发送设备地址+读写控制bit（0 = w， 1 = r) bit7 先传
        i2c_SendByte(_Address | I2C_WR);
        ucAck = i2c_WaitAck();  // 检测设备的ACK应答

        i2c_Stop();         // 发送停止信号

        return ucAck;
    }
    I2C_SCL_out();
    I2C_SDA_out();
    return 1;   // I2C总线异常
}

/*
*********************************************************************************************************
*   函 数 名: aic3106_WriteReg
*   功能说明: 写aic3106寄存器
*   形    参:  _ucRegAddr ： 寄存器地址
*             _usValue ：寄存器值
*   返 回 值: 无
*********************************************************************************************************
*/
static uint8_t tca9535_WriteReg(uint8_t baseAddr, uint8_t _ucRegAddr, uint16_t _usValue)
{
    // 发送起始位
    i2c_Start();
    // 发送设备地址+读写控制bit（0 = w， 1 = r) bit7 先传
    i2c_SendByte(baseAddr | I2C_WR);
    // 检测ACK
    if (i2c_WaitAck() == 1)
        return 0;
    // 发送控制字节1
    i2c_SendByte(_ucRegAddr);
    if (i2c_WaitAck() == 1)
        return 0;
    // 发送控制字节2
    i2c_SendByte(_usValue & 0xFF);
    if (i2c_WaitAck() == 1)
        return 0;
    // 发送STOP
    i2c_Stop();
    return 1;
}


static uint8_t tca9535_ReadReg(uint8_t baseAddr,uint8_t ReadAddr)
{
    uint8_t data_buf;

    i2c_Start();
    i2c_SendByte(baseAddr | I2C_WR);    // 高7bit是地址，bit0是读写控制位，0表示写，1表示读
    if (i2c_WaitAck() != 0) //等待应答
        return 0;   //无应答

    i2c_SendByte(ReadAddr);
    if (i2c_WaitAck() != 0)
        return 0;   //无应答

    i2c_Start();
    i2c_SendByte(baseAddr | I2C_RD);    // 高7bit是地址，bit0是读写控制位，0表示写，1表示读
    if (i2c_WaitAck() != 0)
        return 0;   //无应答

    data_buf = i2c_ReadByte();  // 读1个字节
    i2c_NAck();             // 最后1个字节读完后，CPU产生NACK信号(驱动SDA = 1)

    i2c_Stop(); // 发送I2C总线停止信号
    return data_buf;    // 执行成功
}

int16_t  tca9535_io2_init(void)
{
    i2c_Init();

    tca9535_WriteReg(TCA9535_I2C_ADDR,TCA9535_CONFIG_PORT0,0x00);//配置P00~P07为输出
    tca9535_WriteReg(TCA9535_I2C_ADDR,TCA9535_CONFIG_PORT1,0xff);//配置P10~P17为输入
    tca9535_WriteReg(TCA9535_I2C_ADDR,TCA9535_OUTPUT_PORT0,0xff);//配置P10~P17输出高电平

    return 0;
}

static int16_t DO0_Reg = 0xff;
static int16_t DO0_Reg_old = 0x00;
int16_t  tca9535_io2_p0Set(int16_t _reg,int16_t _set)
{
    switch(_reg)
    {
        case 1:
            if(_set)
                DO0_Reg |= TCA9535_PIN0;
            else
                DO0_Reg &= (~TCA9535_PIN0);
            break;
        case 2:
            if(_set)
                DO0_Reg |= TCA9535_PIN1;
            else
                DO0_Reg &= (~TCA9535_PIN1);
            break;
        case 3:
            if(_set)
                DO0_Reg |= TCA9535_PIN2;
            else
                DO0_Reg &= (~TCA9535_PIN2);
            break;
        case 4:
            if(_set)
                DO0_Reg |= TCA9535_PIN3;
            else
                DO0_Reg &= (~TCA9535_PIN3);
            break;
        case 5:
            if(_set)
                DO0_Reg |= TCA9535_PIN4;
            else
                DO0_Reg &= (~TCA9535_PIN4);
            break;
        case 6:
            if(_set)
                DO0_Reg |= TCA9535_PIN5;
            else
                DO0_Reg &= (~TCA9535_PIN5);
            break;
        case 7:
            if(_set)
                DO0_Reg |= TCA9535_PIN6;
            else
                DO0_Reg &= (~TCA9535_PIN6);
            break;
        case 8:
            if(_set)
                DO0_Reg |= TCA9535_PIN7;
            else
                DO0_Reg &= (~TCA9535_PIN7);
            break;
        default:break;
    }
    if(DO0_Reg_old != DO0_Reg)
    {
        DO0_Reg_old = DO0_Reg;
        tca9535_WriteReg(TCA9535_I2C_ADDR,TCA9535_OUTPUT_PORT0,DO0_Reg);
    }

    return 0;
}

static int16_t DO1_Reg = 0xff;
static int16_t DO1_Reg_old = 0x00;
int16_t  tca9535_io2_p1Set(int16_t _reg,int16_t _set)
{
    switch(_reg)
    {
        case 1:
            if(_set)
                DO1_Reg |= TCA9535_PIN0;
            else
                DO1_Reg &= (~TCA9535_PIN0);
            break;
        case 2:
            if(_set)
                DO1_Reg |= TCA9535_PIN1;
            else
                DO1_Reg &= (~TCA9535_PIN1);
            break;
        case 3:
            if(_set)
                DO1_Reg |= TCA9535_PIN2;
            else
                DO1_Reg &= (~TCA9535_PIN2);
            break;
        case 4:
            if(_set)
                DO1_Reg |= TCA9535_PIN3;
            else
                DO1_Reg &= (~TCA9535_PIN3);
            break;
        case 5:
            if(_set)
                DO1_Reg |= TCA9535_PIN4;
            else
                DO1_Reg &= (~TCA9535_PIN4);
            break;
        case 6:
            if(_set)
                DO1_Reg |= TCA9535_PIN5;
            else
                DO1_Reg &= (~TCA9535_PIN5);
            break;
        case 7:
            if(_set)
                DO1_Reg |= TCA9535_PIN6;
            else
                DO1_Reg &= (~TCA9535_PIN6);
            break;
        case 8:
            if(_set)
                DO1_Reg |= TCA9535_PIN7;
            else
                DO1_Reg &= (~TCA9535_PIN7);
            break;
        default:break;
    }
    if(DO1_Reg_old != DO1_Reg)
    {
        DO1_Reg_old = DO1_Reg;
        tca9535_WriteReg(TCA9535_I2C_ADDR,TCA9535_OUTPUT_PORT1,DO1_Reg);
    }

    return 0;
}

int16_t  tca9535_io2_p0Get(int16_t _reg)
{
    int16_t regGet = 0x00;
    int16_t ret;

    regGet = tca9535_ReadReg(TCA9535_I2C_ADDR,TCA9535_INPUT_PORT0);
    switch(_reg)
    {
        case 1:
            if(regGet & TCA9535_PIN0)
                ret = 1;
            else
                ret = 0;
            break;
        case 2:
            if(regGet & TCA9535_PIN1)
                ret = 1;
            else
                ret = 0;
            break;
        case 3:
            if(regGet & TCA9535_PIN2)
                ret = 1;
            else
                ret = 0;
            break;
        case 4:
            if(regGet & TCA9535_PIN3)
                ret = 1;
            else
                ret = 0;
            break;
        case 5:
            if(regGet & TCA9535_PIN4)
                ret = 1;
            else
                ret = 0;
            break;
        case 6:
            if(regGet & TCA9535_PIN5)
                ret = 1;
            else
                ret = 0;
            break;
        case 7:
            if(regGet & TCA9535_PIN6)
                ret = 1;
            else
                ret = 0;
            break;
        case 8:
            if(regGet & TCA9535_PIN7)
                ret = 1;
            else
                ret = 0;
            break;
        default:ret = 0;break;
    }

    return ret;
}

int16_t  tca9535_io2_p1Get(int16_t _reg)
{
    int16_t regGet = 0x00;
    int16_t ret;

    regGet = tca9535_ReadReg(TCA9535_I2C_ADDR,TCA9535_INPUT_PORT1);
    switch(_reg)
    {
        case 1:
            if(regGet & TCA9535_PIN0)
                ret = 1;
            else
                ret = 0;
            break;
        case 2:
            if(regGet & TCA9535_PIN1)
                ret = 1;
            else
                ret = 0;
            break;
        case 3:
            if(regGet & TCA9535_PIN2)
                ret = 1;
            else
                ret = 0;
            break;
        case 4:
            if(regGet & TCA9535_PIN3)
                ret = 1;
            else
                ret = 0;
            break;
        case 5:
            if(regGet & TCA9535_PIN4)
                ret = 1;
            else
                ret = 0;
            break;
        case 6:
            if(regGet & TCA9535_PIN5)
                ret = 1;
            else
                ret = 0;
            break;
        case 7:
            if(regGet & TCA9535_PIN6)
                ret = 1;
            else
                ret = 0;
            break;
        case 8:
            if(regGet & TCA9535_PIN7)
                ret = 1;
            else
                ret = 0;
            break;
        default:ret = 0;break;
    }

    return ret;
}

/***************************** End Of File ***********************************/








