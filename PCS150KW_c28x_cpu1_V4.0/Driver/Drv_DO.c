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
#define GPIO_PIN_DO01              23U
#define GPIO_CFG_DO01              GPIO_23_GPIO23

#define GPIO_PIN_DO02              22U
#define GPIO_CFG_DO02              GPIO_22_GPIO22
//
#define GPIO_PIN_DO03              21U
#define GPIO_CFG_DO03              GPIO_21_GPIO21
//
#define GPIO_PIN_DO04              20U
#define GPIO_CFG_DO04              GPIO_20_GPIO20
//
#define GPIO_PIN_DO05              19U
#define GPIO_CFG_DO05              GPIO_19_GPIO19
//
#define GPIO_PIN_DO06              18U
#define GPIO_CFG_DO06              GPIO_18_GPIO18
//
#define GPIO_PIN_DO07              17U
#define GPIO_CFG_DO07              GPIO_17_GPIO17
//
#define GPIO_PIN_DO08              16U
#define GPIO_CFG_DO08              GPIO_16_GPIO16
//PWM_EN2
#define GPIO_PIN_PWMEN2            153U  //PWMEN2
#define GPIO_CFG_PWMEN2            GPIO_153_GPIO153

#define DO_17_REG                  6    //EXIO2_P05     工作指示灯
#define DO_18_REG                  5    //EXIO2_P04     故障指示灯

#define DO_09_REG                  1    //EXIO1_P00     扩展输出1
#define DO_10_REG                  2    //EXIO1_P01     扩展输出2
#define DO_11_REG                  3    //EXIO1_P02     扩展输出3


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
    //DO05
    GPIO_setPinConfig(GPIO_CFG_DO05);
    GPIO_setPadConfig(GPIO_PIN_DO05, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_DO05, GPIO_DIR_MODE_OUT);
    //DO06
    GPIO_setPinConfig(GPIO_CFG_DO06);
    GPIO_setPadConfig(GPIO_PIN_DO06, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_DO06, GPIO_DIR_MODE_OUT);
    //DO07
    GPIO_setPinConfig(GPIO_CFG_DO07);
    GPIO_setPadConfig(GPIO_PIN_DO07, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_DO07, GPIO_DIR_MODE_OUT);
    //DO08
    GPIO_setPinConfig(GPIO_CFG_DO08);
    GPIO_setPadConfig(GPIO_PIN_DO08, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_DO08, GPIO_DIR_MODE_OUT);

    GPIO_writePin(GPIO_PIN_PWMEN2, 1);
    GPIO_writePin(GPIO_PIN_DO01, 1);
    GPIO_writePin(GPIO_PIN_DO02, 1);
    GPIO_writePin(GPIO_PIN_DO03, 1);
    GPIO_writePin(GPIO_PIN_DO04, 1);
    GPIO_writePin(GPIO_PIN_DO05, 1);
    GPIO_writePin(GPIO_PIN_DO06, 1);
    GPIO_writePin(GPIO_PIN_DO07, 0);
    GPIO_writePin(GPIO_PIN_DO08, 0);
    GPIO_writePin(GPIO_PIN_PWMEN2, 0);
}


int16_t Drv_RelayGet(int16_t RelayIndex)
{
    uint32_t PinStatus;
    switch(RelayIndex)
    {
        case Relay_SoftAC:
            PinStatus = GPIO_readPin(GPIO_PIN_DO01);
            break;
        case Relay_MastAC:
            PinStatus = GPIO_readPin(GPIO_PIN_DO02);
            break;
        case Relay_SoftDC:
            PinStatus = GPIO_readPin(GPIO_PIN_DO03);
            break;
        case Relay_MastDC:
            PinStatus = GPIO_readPin(GPIO_PIN_DO04);
            break;
        case Relay_FANPwr:
            PinStatus = GPIO_readPin(GPIO_PIN_DO05);
            break;
        case Led_Work:
            PinStatus = tca9535_io2_p0Get(DO_17_REG);
            break;
        case Led_Fault:
            PinStatus = tca9535_io2_p0Get(DO_18_REG);
            break;
        case Do1_Res:
            PinStatus = tca9535_io1_p0Get(DO_09_REG);
            break;
        case Do2_Res:
            PinStatus = tca9535_io1_p0Get(DO_10_REG);
            break;
        case Do3_Res:
            PinStatus = tca9535_io1_p0Get(DO_11_REG);
            break;
        default:PinStatus = 0;break;
    }

    return (int16_t)(PinStatus);
}

void Drv_RelaySet(int16_t RelayIndex,int16_t SetValue)
{
    switch(RelayIndex)
    {
        case Relay_SoftAC:
            if(SetValue == 1)
                GPIO_writePin(GPIO_PIN_DO01, 1);
            else
                GPIO_writePin(GPIO_PIN_DO01, 0);
            break;
        case Relay_MastAC:
            if(SetValue == 1)
                GPIO_writePin(GPIO_PIN_DO02, 1);
            else
                GPIO_writePin(GPIO_PIN_DO02, 0);
            break;
        case Relay_SoftDC:
            if(SetValue == 1)
                GPIO_writePin(GPIO_PIN_DO03, 1);
            else
                GPIO_writePin(GPIO_PIN_DO03, 0);
            break;
        case Relay_MastDC:
            if(SetValue == 1)
                GPIO_writePin(GPIO_PIN_DO04, 1);
            else
                GPIO_writePin(GPIO_PIN_DO04, 0);
            break;
        case Relay_FANPwr:
            if(SetValue == 1)
                GPIO_writePin(GPIO_PIN_DO05, 1);
            else
                GPIO_writePin(GPIO_PIN_DO05, 1);
            break;
        case Led_Work:
            if(SetValue == 1)
                tca9535_io2_p0Set(DO_17_REG,1);
            else
                tca9535_io2_p0Set(DO_17_REG,0);
            break;
        case Led_Fault:
            if(SetValue == 1)
                tca9535_io2_p0Set(DO_18_REG,1);
            else
                tca9535_io2_p0Set(DO_18_REG,0);
            break;
        case Do1_Res:
            if(SetValue == 1)
                tca9535_io1_p0Set(DO_09_REG,1);
            else
                tca9535_io1_p0Set(DO_09_REG,0);
            break;
        case Do2_Res:
            if(SetValue == 1)
                tca9535_io1_p0Set(DO_10_REG,1);
            else
                tca9535_io1_p0Set(DO_10_REG,0);
            break;
        case Do3_Res:
            if(SetValue == 1)
                tca9535_io1_p0Set(DO_11_REG,1);
            else
                tca9535_io1_p0Set(DO_11_REG,0);
            break;
        default:break;
    }
}





