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
//
// I2C
//
#define I2CA_EE_ADDR                0x50    //EE芯片地址
#define EE_ADDR_MIN                 0
#define EE_ADDR_MAX                 (2048-1) //2K*8bits  每次传输2bytes


int I2C_EE_WriteBuf(uint16_t addr, uint16_t len, uint16_t *pdata)
{
    uint8_t dataAddrL,dataAddrH;
    uint16_t DevAddr,RegAddr;

    if(addr%2 || addr<EE_ADDR_MIN ||(addr+len)>(EE_ADDR_MAX+1))
        return 1;

    if(len==0 || !pdata)
        return 2;

    dataAddrL = (uint8_t)(addr&0xff);
    dataAddrH = (uint8_t)((addr>>8)&0x07);

    DevAddr  = I2CA_EE_ADDR | dataAddrH;
    RegAddr  = dataAddrL;

    return I2CA_WriteBuf(DevAddr,RegAddr, len, pdata,2000);
}

int I2C_EE_ReadBuf(uint16_t addr, uint16_t len, uint16_t *pdata)
{
    uint8_t dataAddrL,dataAddrH;
    uint16_t DevAddr,RegAddr;

    if(addr<EE_ADDR_MIN ||(addr+len)>(EE_ADDR_MAX+1))
        return 1;

    if(len==0 || !pdata)
        return 2;

    dataAddrL = (uint8_t)(addr&0xff);
    dataAddrH = (uint8_t)((addr>>8)&0x07);

    DevAddr  = I2CA_EE_ADDR | dataAddrH;
    RegAddr  = dataAddrL;

    return I2CA_ReadBuf(DevAddr,RegAddr, len, pdata,2000);
}



