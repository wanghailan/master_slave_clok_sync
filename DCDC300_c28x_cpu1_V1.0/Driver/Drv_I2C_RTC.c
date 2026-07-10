//#############################################################################
//
// FILE:   Drv_LED.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

#include <Driver/device/device.h>
#include <Driver/device/driverlib.h>
#include "Drv_I2CA.h"
#include "Drv_I2C_RTC.h"
//
// I2C
//
#define RTC_ADDR_MIN                  0
#define RTC_ADDR_MAX                 (19-1) //

// 定义ISL1208I的I2C地址
#define ISL1208I_ADDR       0x6F

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

static int rtc_write_byte(uint16_t RegAddr,uint16_t pData)
{
    return I2CA_WriteBuf(ISL1208I_ADDR,RegAddr, 1, &pData,500);
}

static int rtc_read_byte(uint16_t RegAddr)
{
    uint16_t pdata;

    I2CA_ReadBuf(ISL1208I_ADDR,RegAddr, 1, &pdata,500);

    return pdata;
}

static int rtc_read_bytes(uint16_t RegAddr,uint16_t *pdata,int16_t len)
{
    I2CA_ReadBuf(ISL1208I_ADDR,RegAddr, len, pdata,500);

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
    uint16_t TimeBuf[7];
    rtc_read_bytes(0,TimeBuf,7);
    Str_RtcTime.seconds    = bcd_to_dec(TimeBuf[SECONDS_REG]);
    Str_RtcTime.minutes    = bcd_to_dec(TimeBuf[MINUTES_REG]);
    Str_RtcTime.hours      = bcd_to_dec(TimeBuf[HOURS_REG]&0x7f);
    Str_RtcTime.day        = bcd_to_dec(TimeBuf[DAY_REG]);
    Str_RtcTime.month      = bcd_to_dec(TimeBuf[MONTH_REG]);
    Str_RtcTime.year       = 2000+bcd_to_dec(TimeBuf[YEAR_REG]);
    Str_RtcTime.dweek      = bcd_to_dec(TimeBuf[DW_REG]);
}



