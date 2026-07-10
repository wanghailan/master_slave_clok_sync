//#############################################################################
//
// FILE:   Drv_DO.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

#include <Driver/device/device.h>
#include <Driver/device/driverlib.h>
#include "bsp.h"
//
// GPIO
//
#define GPIO_PIN_DO01              23U  // GPIO number for DO01
#define GPIO_CFG_DO01              GPIO_23_GPIO23  // "pinConfig" for DO01

#define GPIO_PIN_DO02              22U  // GPIO number for DO02
#define GPIO_CFG_DO02              GPIO_22_GPIO22  // "pinConfig" for DO02
//
#define GPIO_PIN_DO03              21U  // GPIO number for DO03
#define GPIO_CFG_DO03              GPIO_21_GPIO21  // "pinConfig" for DO03
//
#define GPIO_PIN_DO04              20U  // GPIO number for DO04
#define GPIO_CFG_DO04              GPIO_20_GPIO20  // "pinConfig" for DO04


#define GPIO_PIN_RDYCA             5U  // GPIO number for RDYC
#define GPIO_CFG_RDYCA             GPIO_5_GPIO5  // "pinConfig" for RDYCA
#define GPIO_PIN_RDYCB             8U  // GPIO number for RDYC
#define GPIO_CFG_RDYCB             GPIO_8_GPIO8  // "pinConfig" for RDYCB
#define GPIO_PIN_RDYCC             4U  // GPIO number for RDYC
#define GPIO_CFG_RDYCC             GPIO_4_GPIO4  // "pinConfig" for RDYCC

#define GPIO_PIN_FAN               18U  // GPIO number for FAN
#define GPIO_CFG_FAN               GPIO_18_GPIO18  // "pinConfig" for FAN

#define GPIO_PIN_ASel1             96U  // GPIO number for ASel1
#define GPIO_CFG_ASel1             GPIO_96_GPIO96  // "pinConfig" for ASel1
#define GPIO_PIN_ASel2             61U  // GPIO number for ASel2
#define GPIO_CFG_ASel2             GPIO_61_GPIO61  // "pinConfig" for ASel2

#define GPIO_PIN_DISCHARGE         19U  // GPIO number for DISCHARGE
#define GPIO_CFG_DISCHARGE         GPIO_19_GPIO19  // "pinConfig" for DISCHARGE

//PWM_EN2
#define GPIO_PIN_PWMEN2            153U  // GPIO number for PWMEN2
#define GPIO_CFG_PWMEN2            GPIO_153_GPIO153  // "pinConfig" for PWMEN2
#define GPIO_PIN_PWMEN0            97U  // GPIO number for PWMEN2
#define GPIO_CFG_PWMEN0            GPIO_97_GPIO97  // "pinConfig" for PWMEN2
//#define GPIO_PIN_PWMEN1            153U  // GPIO number for PWMEN2
//#define GPIO_CFG_PWMEN1            GPIO_153_GPIO153  // "pinConfig" for PWMEN2


static int16_t  tca9535_io2P0_reg = 0xff;//1�ر�DO���, 0��DO���

void Drv_DO_Init(void)
{
    //PWM_EN2
    GPIO_setPinConfig(GPIO_CFG_PWMEN2);
    GPIO_setPadConfig(GPIO_PIN_PWMEN2, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_PWMEN2, GPIO_DIR_MODE_OUT);
    //DO01
    GPIO_setPinConfig(GPIO_CFG_DO01);
    GPIO_setPadConfig(GPIO_PIN_DO01, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_DO01, GPIO_DIR_MODE_OUT);
    //DO02
    GPIO_setPinConfig(GPIO_CFG_DO02);
    GPIO_setPadConfig(GPIO_PIN_DO02, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_DO02, GPIO_DIR_MODE_OUT);
    //DO03
    GPIO_setPinConfig(GPIO_CFG_DO03);
    GPIO_setPadConfig(GPIO_PIN_DO03, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_DO03, GPIO_DIR_MODE_OUT);
    //DO04
    GPIO_setPinConfig(GPIO_CFG_DO04);
    GPIO_setPadConfig(GPIO_PIN_DO04, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_DO04, GPIO_DIR_MODE_OUT);

    GPIO_setPinConfig(GPIO_CFG_RDYCC);
    GPIO_setPadConfig(GPIO_PIN_RDYCC, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_RDYCC, GPIO_DIR_MODE_OUT);

    GPIO_setPinConfig(GPIO_CFG_RDYCB);
    GPIO_setPadConfig(GPIO_PIN_RDYCB, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_RDYCB, GPIO_DIR_MODE_OUT);

    GPIO_setPinConfig(GPIO_CFG_RDYCA);
    GPIO_setPadConfig(GPIO_PIN_RDYCA, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_RDYCA, GPIO_DIR_MODE_OUT);

    GPIO_setPinConfig(GPIO_CFG_ASel1);
    GPIO_setPadConfig(GPIO_PIN_ASel1, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_ASel1, GPIO_DIR_MODE_OUT);

    GPIO_setPinConfig(GPIO_CFG_ASel2);
    GPIO_setPadConfig(GPIO_PIN_ASel2, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_ASel2, GPIO_DIR_MODE_OUT);

    GPIO_setPinConfig(GPIO_CFG_DISCHARGE);
    GPIO_setPadConfig(GPIO_PIN_DISCHARGE, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_DISCHARGE, GPIO_DIR_MODE_OUT);

    GPIO_setPinConfig(GPIO_CFG_FAN);
    GPIO_setPadConfig(GPIO_PIN_FAN, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_FAN, GPIO_DIR_MODE_OUT);

    GPIO_writePin(GPIO_PIN_PWMEN2, 1);
    GPIO_writePin(GPIO_PIN_PWMEN0, 0);
    GPIO_writePin(GPIO_PIN_DO01, 0);
    GPIO_writePin(GPIO_PIN_DO02, 0);
    GPIO_writePin(GPIO_PIN_DO03, 0);
    GPIO_writePin(GPIO_PIN_DO04, 0);
    GPIO_writePin(GPIO_PIN_DISCHARGE, 0);
    GPIO_writePin(GPIO_PIN_RDYCA, 1);
    GPIO_writePin(GPIO_PIN_RDYCB, 1);
    GPIO_writePin(GPIO_PIN_RDYCC, 1);
    GPIO_writePin(GPIO_PIN_FAN, 0);
    GPIO_writePin(GPIO_PIN_PWMEN2, 0);

    tca9535_io2P0_reg = 0xff;
}


int16_t Drv_RelayGet(int16_t RelayIndex)
{
    uint32_t PinStatus;
//    switch(RelayIndex)
//    {
//        case Relay_SoftBat:
//            PinStatus = GPIO_readPin(GPIO_PIN_DO03);
//            break;
//        case Relay_MastBat:
////            PinStatus = GPIO_readPin(GPIO_PIN_DCMCTRL);
//            break;
//        case Relay_SoftBus:
//            PinStatus = GPIO_readPin(GPIO_PIN_DO02);
//            break;
//        case Relay_MastBus:
//            PinStatus = GPIO_readPin(GPIO_PIN_DO01);
//            break;
//        case Relay_FAN:
////            PinStatus = GPIO_readPin(GPIO_PIN_FANCTRL);
//            break;
//        default:PinStatus = 0;break;
//    }

    return (int16_t)(PinStatus);
}

void Drv_RelaySet(int16_t RelayIndex,int16_t SetValue)
{
    switch(RelayIndex)
    {
        case Relay_SoftBat:
            if(SetValue == 1)
                GPIO_writePin(GPIO_PIN_DO04, 1);
            else
                GPIO_writePin(GPIO_PIN_DO04, 0);
            break;
        case Relay_MastBat:
            if(SetValue == 1)
                GPIO_writePin(GPIO_PIN_DO01, 1);
            else
                GPIO_writePin(GPIO_PIN_DO01, 0);
            break;
        case Relay_SoftBus:
            if(SetValue == 1)
                GPIO_writePin(GPIO_PIN_DO02, 1);
            else
                GPIO_writePin(GPIO_PIN_DO02, 0);
            break;
        case Relay_MastBus:
            if(SetValue == 1) //P06
                GPIO_writePin(GPIO_PIN_DO03, 1);
            else
                GPIO_writePin(GPIO_PIN_DO03, 0);
            break;
        case Relay_FAN:
            if(SetValue == 1) //P03
                GPIO_writePin(GPIO_PIN_FAN, 1);
            else
                GPIO_writePin(GPIO_PIN_FAN, 0);
            break;
        case Relay_WorkLed:
            if(SetValue == 1) //P04
                tca9535_io2P0_reg &= 0xEF;
            else
                tca9535_io2P0_reg |= 0x10;
            break;
        case Relay_FaultLed:
            if(SetValue == 1) //P05
                tca9535_io2P0_reg &= 0xDF;
            else
                tca9535_io2P0_reg |= 0x20;
            break;
        default:break;
    }
}

//void  Drv_UpExtDo(void)
//{
//     tca9535_io2_p0_set(tca9535_io2P0_reg);
//}





