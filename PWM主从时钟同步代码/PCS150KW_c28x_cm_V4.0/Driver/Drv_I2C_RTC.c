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
//
// I2C
//
#define RTC_RegSize                   19 //

// 定义ISL1208I的I2C地址
#define ISL1208I_ADDR       0xDE

// 定义寄存器地址
#define SECONDS_REG     0x00
#define MINUTES_REG     0x01
#define HOURS_REG       0x02
#define DAY_REG         0x03
#define MONTH_REG       0x04
#define YEAR_REG        0x05
#define DW_REG          0x06
#define SR_REG          0x07

RtcTime Str_RtcTime;

static int RTC_WriteBuf(uint8_t *_pWriteBuf, uint16_t _usAddress, uint16_t _usSize,uint16_t WaitUs)
{
    uint16_t i,m;
    uint16_t usAddr;
    uint8_t AddrH,AddrL;

    do
    {
        Drv_Timer_usDelay(10);
    }while(I2CA_Busy == 1);

    if((_usAddress+_usSize) > RTC_RegSize)//超过器件容量，退出
        goto cmd_fail;

    usAddr = _usAddress;

    for (i = 0; i < _usSize; i++)
    {
        if ((i == 0) || ((usAddr & (RTC_RegSize - 1)) == 0))
        {
            AddrH = (uint8_t)((usAddr >> 8)&0xff);
            AddrL = (uint8_t)(usAddr&0xff);
            if(AddrH > 0)
                goto cmd_fail;

            i2cA_Stop();//发停止信号，启动内部写操作
            /* 通过检查器件应答的方式，判断内部写操作是否完成, 一般小于 10ms
                CLK频率为200KHz时，查询次数为30次左右 */
            for (m = 0; m < WaitUs; m++)
            {
                i2cA_Start();// 发起I2C总线启动信号
                i2cA_SendByte(ISL1208I_ADDR | I2C_WR); // 高7bit是地址，bit0是读写控制位，0表示写，1表示读

                if (i2cA_WaitAck() == 0)
                    break;
            }
            if (m  >= WaitUs)
                goto cmd_fail;

            i2cA_SendByte(AddrL);
            if (i2cA_WaitAck() != 0)
                goto cmd_fail;
        }

        i2cA_SendByte(_pWriteBuf[i]);//开始写入数据
        if (i2cA_WaitAck() != 0)
            goto cmd_fail;

        usAddr++;   // 地址增1
    }

    // 命令执行成功，发送I2C总线停止信号
    i2cA_Stop();
    return 1;

cmd_fail: // 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备
    i2cA_Stop();
    return 0;
}



static int RTC_ReadBuf(uint8_t *_pReadBuf, uint16_t _usAddress, uint16_t _usSize,uint16_t WaitUs)
{
    uint16_t i,m;
    uint8_t AddrH,AddrL;

    do
    {
        Drv_Timer_usDelay(10);
    }while(I2CA_Busy == 1);

    if((_usAddress+_usSize) > RTC_RegSize)//超过器件容量，退出
        goto cmd_fail;

    AddrH = (uint8_t)((_usAddress >> 8)&0xff);
    AddrL = (uint8_t)(_usAddress&0xff);
    if(AddrH > 0)
        goto cmd_fail;
    i2cA_Stop();//发停止信号，启动内部写操作
    for (m = 0; m < WaitUs; m++)
    {
        i2cA_Start();// 发起I2C总线启动信号
        i2cA_SendByte(ISL1208I_ADDR | I2C_WR); // 高7bit是地址，bit0是读写控制位，0表示写，1表示读

        if (i2cA_WaitAck() == 0)
            break;
    }
    if (m  >= WaitUs)
        goto cmd_fail;

    // 发送字节地址
    i2cA_SendByte(AddrL);
    if (i2cA_WaitAck() != 0)
        goto cmd_fail;

    i2cA_Start();
    i2cA_SendByte(ISL1208I_ADDR | I2C_RD); // 高7bit是地址，bit0是读写控制位，0表示写，1表示读

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

static int rtc_write_byte(uint16_t RegAddr,uint8_t pData)
{
    return  RTC_WriteBuf(&pData, RegAddr, 1,500);
}

static int rtc_read_byte(uint16_t RegAddr)
{
    uint8_t pdata;

    RTC_ReadBuf(&pdata, RegAddr, 1,500);

    return pdata;
}

static int rtc_read_bytes(uint16_t RegAddr,uint8_t *pdata,int16_t len)
{
    RTC_ReadBuf(pdata, RegAddr, len,500);

    return 1;
}


// BCD码转换为十进制数
static uint8_t bcd_to_dec(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

// 十进制数转换为BCD码
static uint8_t dec_to_bcd(uint8_t dec) {
    return ((dec / 10) << 4) | (dec % 10);
}

// 初始化ISL1208I
void rtc_init(void)
{
    rtc_write_byte(0x07,0x91);
//    rtc_write_byte(0x08,0x20);
}

// 设置时间
void rtc_write_time(int16_t regAddr,int16_t regData)
{
    rtc_write_byte(0x07,0x90);
    if(regAddr < 7)
    {
        if(regAddr == HOURS_REG)
            rtc_write_byte(regAddr, dec_to_bcd(regData)+0x80);//24小时
        else if(regAddr == YEAR_REG)
            rtc_write_byte(regAddr, dec_to_bcd(regData-2000));
        else
            rtc_write_byte(regAddr, dec_to_bcd(regData));
    }
    else
        rtc_write_byte(regAddr, regData);
}

// 读取时间
void rtc_read_time(void)
{
    uint8_t TimeBuf[7];
    rtc_read_bytes(0,TimeBuf,7);
    Str_RtcTime.seconds    = bcd_to_dec(TimeBuf[SECONDS_REG]);
    Str_RtcTime.minutes    = bcd_to_dec(TimeBuf[MINUTES_REG]);
    Str_RtcTime.hours      = bcd_to_dec(TimeBuf[HOURS_REG]&0x7f);
    Str_RtcTime.day        = bcd_to_dec(TimeBuf[DAY_REG]);
    Str_RtcTime.month      = bcd_to_dec(TimeBuf[MONTH_REG]);
    Str_RtcTime.year       = 2000+bcd_to_dec(TimeBuf[YEAR_REG]);
    Str_RtcTime.dweek      = bcd_to_dec(TimeBuf[DW_REG]);
}



