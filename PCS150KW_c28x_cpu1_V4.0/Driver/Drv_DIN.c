//#############################################################################
//
// FILE:   Drv_IO.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

#include <Driver/device/device.h>
#include <Driver/device/driverlib.h>
#include "bsp.h"
//
// GPIO

//急停输入低电平有效 DIN1输入
#define GPIO_PIN_DI01            145U  // GPIO number for
#define GPIO_CFG_DI01            GPIO_145_GPIO145  // "pinConfig" for
//远程输入
#define GPIO_PIN_DI02            146U
#define GPIO_CFG_DI02            GPIO_146_GPIO146
//
#define GPIO_PIN_DI03            147U
#define GPIO_CFG_DI03            GPIO_147_GPIO147
//模拟片选2
#define GPIO_PIN_DI04            74U
#define GPIO_CFG_DI04            GPIO_74_GPIO74
//直流过流硬件比较
#define GPIO_PIN_DI05            76U
#define GPIO_CFG_DI05            GPIO_76_GPIO76
//模拟片选1
#define GPIO_PIN_DI06            68U
#define GPIO_CFG_DI06            GPIO_68_GPIO68
//
#define GPIO_PIN_DI07            67U
#define GPIO_CFG_DI07            GPIO_67_GPIO67
//
#define GPIO_PIN_DI08            69U
#define GPIO_CFG_DI08            GPIO_69_GPIO69

#define DIN_01_REG               8    //EXIO1_P17     扩展输入1
#define DIN_02_REG               7    //EXIO1_P16     扩展输入2
#define DIN_03_REG               6    //EXIO1_P15     扩展输入3
#define DI_ADDR1_REG             1    //EXIO2_10     拨码地址位1
#define DI_ADDR2_REG             3    //EXIO2_12     拨码地址位2
#define DI_ADDR3_REG             2    //EXIO2_11     拨码地址位3
#define DI_ADDR4_REG             4    //EXIO2_13     拨码地址位4


void Drv_DinPin_Init(void)
{
    //DO01
    GPIO_setPinConfig(GPIO_CFG_DI01);
    GPIO_setPadConfig(GPIO_PIN_DI01, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_DI01, GPIO_DIR_MODE_IN);
    //DO02
    GPIO_setPinConfig(GPIO_CFG_DI02);
    GPIO_setPadConfig(GPIO_PIN_DI02, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_DI02, GPIO_DIR_MODE_IN);
    //DO03
    GPIO_setPinConfig(GPIO_CFG_DI03);
    GPIO_setPadConfig(GPIO_PIN_DI03, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_DI03, GPIO_DIR_MODE_IN);
    //DO04
    GPIO_setPinConfig(GPIO_CFG_DI04);
    GPIO_setPadConfig(GPIO_PIN_DI04, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_DI04, GPIO_DIR_MODE_IN);
    //DO05
    GPIO_setPinConfig(GPIO_CFG_DI05);
    GPIO_setPadConfig(GPIO_PIN_DI05, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_DI05, GPIO_DIR_MODE_IN);
    //DO06
    GPIO_setPinConfig(GPIO_CFG_DI06);
    GPIO_setPadConfig(GPIO_PIN_DI06, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_DI06, GPIO_DIR_MODE_IN);
    //DO07
    GPIO_setPinConfig(GPIO_CFG_DI07);
    GPIO_setPadConfig(GPIO_PIN_DI07, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_DI07, GPIO_DIR_MODE_IN);
    //DO08
    GPIO_setPinConfig(GPIO_CFG_DI08);
    GPIO_setPadConfig(GPIO_PIN_DI08, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_DI08, GPIO_DIR_MODE_IN);
}


//输入状态
int16_t Drv_DinStatusGet(int16_t RelayIndex)
{
    uint32_t PinStatus;
    switch(RelayIndex)
    {
        case Din_EStop:
            PinStatus = GPIO_readPin(GPIO_PIN_DI01);
            break;
        case Din_ReMCtrl:
            PinStatus = GPIO_readPin(GPIO_PIN_DI02);
            break;
        case Din_IdcCmp:
            PinStatus = GPIO_readPin(GPIO_PIN_DI05);
            break;
        case Din_in1:
            PinStatus = tca9535_io1_p1Get(DIN_01_REG);
            break;
        case Din_in2:
            PinStatus = tca9535_io1_p1Get(DIN_02_REG);
            break;
        case Din_in3:
            PinStatus = tca9535_io1_p1Get(DIN_03_REG);
            break;
        case Din_Addr1:
            PinStatus = tca9535_io2_p1Get(DI_ADDR1_REG);
            break;
        case Din_Addr2:
            PinStatus = tca9535_io2_p1Get(DI_ADDR2_REG);
            break;
        case Din_Addr3:
            PinStatus = tca9535_io2_p1Get(DI_ADDR3_REG);
            break;
        case Din_Addr4:
            PinStatus = tca9535_io2_p1Get(DI_ADDR4_REG);
            break;
        default:PinStatus = 0;break;
    }

    return (int16_t)(PinStatus);
}



