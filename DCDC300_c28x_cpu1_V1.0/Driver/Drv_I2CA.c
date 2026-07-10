//#############################################################################
//
// FILE:   Drv_I2CA.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

#include <Driver/device/device.h>
#include <Driver/device/driverlib.h>
#include "i2cLib_FIFO_polling.h"
#include "Drv_Timer.h"
//
// I2C
//
#define I2CA_GPIO_PIN_SDA           31U  // GPIO number for I2C SDAA
#define I2CA_GPIO_PIN_SCL           32U  // GPIO number for I2C SCLA
#define I2CA_GPIO_SDA_CONFIG        GPIO_31_I2CA_SDA  // "pinConfig" for I2C SDAA
#define I2CA_GPIO_SCL_CONFIG        GPIO_32_I2CA_SCL  // "pinConfig" for I2C SCLA

#define I2CA_SCLK                   100000U //I2CA ±÷”∆µ¬ 
#define I2CA_EE_ADDR                0x6F    //EE–æ∆¨µÿ÷∑

#define I2CEEBase                   I2CA_BASE

void Drv_I2CAPin_Init(void)
{
    // I2CA pins (SDAA / SCLA)
    GPIO_setDirectionMode(I2CA_GPIO_PIN_SDA, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(I2CA_GPIO_PIN_SDA, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(I2CA_GPIO_PIN_SDA, GPIO_QUAL_ASYNC);

    GPIO_setDirectionMode(I2CA_GPIO_PIN_SCL, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(I2CA_GPIO_PIN_SCL, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(I2CA_GPIO_PIN_SCL, GPIO_QUAL_ASYNC);

    GPIO_setPinConfig(I2CA_GPIO_SDA_CONFIG);
    GPIO_setPinConfig(I2CA_GPIO_SCL_CONFIG);
}

static void Drv_I2C_Param_Init(void)
{
    //I2CA initialization
    I2C_disableModule(I2CEEBase);
    I2C_initController(I2CEEBase, DEVICE_SYSCLK_FREQ, I2CA_SCLK, I2C_DUTYCYCLE_50);
    I2C_setConfig(I2CEEBase, I2C_CONTROLLER_SEND_MODE);
    I2C_setTargetAddress(I2CEEBase, I2CA_EE_ADDR);
    I2C_setOwnAddress(I2CEEBase, 0); //I2CA address
    I2C_disableLoopback(I2CEEBase);
    I2C_setBitCount(I2CEEBase, I2C_BITCOUNT_8);
    I2C_setDataCount(I2CEEBase, 1); //
    I2C_setAddressMode(I2CEEBase, I2C_ADDR_MODE_7BITS);
    I2C_enableFIFO(I2CEEBase);
    I2C_clearInterruptStatus(I2CEEBase, I2C_INT_ARB_LOST | I2C_INT_NO_ACK);
    I2C_setFIFOInterruptLevel(I2CEEBase, I2C_FIFO_TXEMPTY, I2C_FIFO_RX2);
    //I2C_enableInterrupt(I2CEEBase, I2C_INT_ADDR_SLAVE | I2C_INT_ARB_LOST | I2C_INT_NO_ACK | I2C_INT_STOP_CONDITION);
    I2C_setEmulationMode(I2CEEBase, I2C_EMULATION_FREE_RUN);
    I2C_enableModule(I2CEEBase);
}

void Drv_I2CA_Init(void)
{
//    Drv_I2CAPin_Init();
    Drv_I2C_Param_Init();
}

int I2CA_WriteBuf(uint16_t DevAddr,uint16_t RegAddr, uint16_t len, uint16_t *pdata,uint16_t WaitUs)
{
    struct I2CHandle I2CA_WHandle;
    uint32_t ControlAddr;

    if(checkBusStatus(I2CEEBase)==SUCCESS)
    {
        ControlAddr = RegAddr;
        I2CA_WHandle.targetAddr     = DevAddr;
        I2CA_WHandle.base           = I2CEEBase;
        I2CA_WHandle.pControlAddr   = &ControlAddr;
        I2CA_WHandle.NumOfAddrBytes = 1;
        I2CA_WHandle.NumOfAttempts  = 5;
        I2CA_WHandle.Delay_us       = 10;

        I2CA_WHandle.NumOfDataBytes = len;
        I2CA_WHandle.pTX_MsgBuffer  = pdata;
        I2C_ControllerTransmitter(&I2CA_WHandle);
        Drv_Timer_usDelay(WaitUs);
    }

    return 0;
}

int I2CA_ReadBuf(uint16_t DevAddr,uint16_t RegAddr, uint16_t len, uint16_t *pdata,uint16_t WaitUs)
{
    struct I2CHandle I2CA_RHandle;
    uint32_t ControlAddr;
    ControlAddr = RegAddr;

    if(checkBusStatus(I2CEEBase)==SUCCESS)
    {
        I2CA_RHandle.targetAddr     = DevAddr;
        I2CA_RHandle.base           = I2CEEBase;
        I2CA_RHandle.pControlAddr   = &ControlAddr;
        I2CA_RHandle.NumOfAddrBytes = 1;
        I2CA_RHandle.NumOfAttempts  = 5;
        I2CA_RHandle.Delay_us       = 10;

        I2CA_RHandle.NumOfDataBytes = len;
        I2CA_RHandle.pRX_MsgBuffer  = pdata;
        I2C_ControllerReceiver(&I2CA_RHandle);
        Drv_Timer_usDelay(WaitUs);
    }

    return 0;
}



