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

//º±Õ£ ‰»ÎµÕµÁ∆Ω”––ß
#define GPIO_PIN_ESTOP            67U  // GPIO number for
#define GPIO_CFG_ESTOP            GPIO_67_GPIO67  // "pinConfig" for

//IGBTπ ’œ ‰»ÎµÕµÁ∆Ω”––ß
#define GPIO_PIN_FIGBT            76U  // GPIO number for
#define GPIO_CFG_FIGBT            GPIO_76_GPIO76  // "pinConfig" for


void Drv_DinPin_Init(void)
{
    //º±Õ£ ‰»Î
    GPIO_setPinConfig(GPIO_CFG_ESTOP);
    GPIO_setPadConfig(GPIO_PIN_ESTOP, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_ESTOP, GPIO_DIR_MODE_IN);

    //IGBTπ ’œ ‰»Î
    GPIO_setPinConfig(GPIO_CFG_FIGBT);
    GPIO_setPadConfig(GPIO_PIN_FIGBT, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_FIGBT, GPIO_DIR_MODE_IN);
}

//º±Õ£ ‰»Î◊¥Ã¨
int16_t Drv_EstopStatusGet(void)
{
    uint32_t PinStatus;

    PinStatus = GPIO_readPin(GPIO_PIN_ESTOP);

    return (int16_t)(PinStatus);
}

//IGBTπ ’œ◊¥Ã¨
int16_t Drv_FigbtStatusGet(void)
{
    uint32_t PinStatus;

    PinStatus = GPIO_readPin(GPIO_PIN_FIGBT);

    return (int16_t)(PinStatus);
}



