//#############################################################################
//
// FILE:   Drv_LED.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

#include <Driver/device/device.h>
#include <Driver/device/driverlib.h>
#include "bsp.h"
//
// LEDs
//
#define DEVICE_GPIO_PIN_LED1        28U  // GPIO number for LD1
#define DEVICE_GPIO_CFG_LED1        GPIO_28_GPIO28  // "pinConfig" for LD1
#define DEVICE_GPIO_PIN_LED2        29U  // GPIO number for LD2
#define DEVICE_GPIO_CFG_LED2        GPIO_29_GPIO29  // "pinConfig" for LD2
#define DEVICE_GPIO_PIN_LED3        33U  // GPIO number for LD3
#define DEVICE_GPIO_CFG_LED3        GPIO_33_GPIO33  // "pinConfig" for LD3


void Drv_LedPin_Init(void)
{
    GPIO_setPinConfig(DEVICE_GPIO_CFG_LED1);
    GPIO_setPadConfig(DEVICE_GPIO_PIN_LED1, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(DEVICE_GPIO_PIN_LED1, GPIO_DIR_MODE_OUT);

    GPIO_setPinConfig(DEVICE_GPIO_CFG_LED2);
    GPIO_setPadConfig(DEVICE_GPIO_PIN_LED2, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(DEVICE_GPIO_PIN_LED2, GPIO_DIR_MODE_OUT);

    GPIO_setPinConfig(DEVICE_GPIO_CFG_LED3);
    GPIO_setPadConfig(DEVICE_GPIO_PIN_LED3, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(DEVICE_GPIO_PIN_LED3, GPIO_DIR_MODE_OUT);

    GPIO_writePin(DEVICE_GPIO_PIN_LED1, 1);//上电熄灭LED1灯
    GPIO_writePin(DEVICE_GPIO_PIN_LED2, 1);//上电熄灭LED2灯
    GPIO_writePin(DEVICE_GPIO_PIN_LED3, 1);//上电熄灭LED3灯

    GPIO_setMasterCore(DEVICE_GPIO_PIN_LED1, GPIO_CORE_CPU1);
    GPIO_setMasterCore(DEVICE_GPIO_PIN_LED2, GPIO_CORE_CPU1);
    GPIO_setMasterCore(DEVICE_GPIO_PIN_LED3, GPIO_CORE_CM);
}

void Drv_Led_On(int16_t _ch)
{
    if(_ch == 1)
        GPIO_writePin(DEVICE_GPIO_PIN_LED1, 0);
    else
        GPIO_writePin(DEVICE_GPIO_PIN_LED2, 0);
}

void Drv_Led_Off(int16_t _ch)
{
    if(_ch == 1)
        GPIO_writePin(DEVICE_GPIO_PIN_LED1, 1);
    else
        GPIO_writePin(DEVICE_GPIO_PIN_LED2, 1);
}

void Drv_Led_toggle(int16_t _ch)
{
    if(_ch == 1)
        GPIO_togglePin(DEVICE_GPIO_PIN_LED1);
    else
        GPIO_togglePin(DEVICE_GPIO_PIN_LED2);
}



