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

#define DEVICE_GPIO_PIN_LEDWork     33U  // GPIO number for work led
#define DEVICE_GPIO_CFG_LEDWork     GPIO_33_GPIO33  // "pinConfig" for work led

#define DEVICE_GPIO_PIN_LEDFault    30U  // GPIO number for fault led
#define DEVICE_GPIO_CFG_LEDFault    GPIO_30_GPIO30  // "pinConfig" for fault led

void Drv_LedPin_Init(void)
{
    GPIO_setPinConfig(DEVICE_GPIO_CFG_LED1);
    GPIO_setPadConfig(DEVICE_GPIO_PIN_LED1, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(DEVICE_GPIO_PIN_LED1, GPIO_DIR_MODE_OUT);

    GPIO_setPinConfig(DEVICE_GPIO_CFG_LED2);
    GPIO_setPadConfig(DEVICE_GPIO_PIN_LED2, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(DEVICE_GPIO_PIN_LED2, GPIO_DIR_MODE_OUT);
    // Configure CM to control the LED2
    //GPIO_setMasterCore(DEVICE_GPIO_PIN_LED2, GPIO_CORE_CPU2);

    GPIO_setPinConfig(DEVICE_GPIO_CFG_LEDWork);
    GPIO_setPadConfig(DEVICE_GPIO_PIN_LEDWork, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(DEVICE_GPIO_PIN_LEDWork, GPIO_DIR_MODE_OUT);

    GPIO_setPinConfig(DEVICE_GPIO_CFG_LEDFault);
    GPIO_setPadConfig(DEVICE_GPIO_PIN_LEDFault, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(DEVICE_GPIO_PIN_LEDFault, GPIO_DIR_MODE_OUT);

    GPIO_setMasterCore(DEVICE_GPIO_PIN_LED1, GPIO_CORE_CPU1);
    GPIO_setMasterCore(DEVICE_GPIO_PIN_LED2, GPIO_CORE_CPU1);

    Drv_Led_Off();
    Drv_WorkLed_Off();
    Drv_FaultLed_Off();
}

void Drv_Led_On(void)
{
    GPIO_writePin(DEVICE_GPIO_PIN_LED1, 1);
}

void Drv_Led_Off(void)
{
    GPIO_writePin(DEVICE_GPIO_PIN_LED1, 0);
}

void Drv_Led_toggle(void)
{
    GPIO_togglePin(DEVICE_GPIO_PIN_LED1);
}

void Drv_WorkLed_On(void)
{
    GPIO_writePin(DEVICE_GPIO_PIN_LEDWork, 1);
}

void Drv_WorkLed_Off(void)
{
    GPIO_writePin(DEVICE_GPIO_PIN_LEDWork, 0);
}

void Drv_WorkLed_toggle(void)
{
    GPIO_togglePin(DEVICE_GPIO_PIN_LEDWork);
}

void Drv_FaultLed_On(void)
{
    GPIO_writePin(DEVICE_GPIO_PIN_LEDFault, 1);
}

void Drv_FaultLed_Off(void)
{
    GPIO_writePin(DEVICE_GPIO_PIN_LEDFault, 0);
}

void Drv_FaultLed_toggle(void)
{
    GPIO_togglePin(DEVICE_GPIO_PIN_LEDFault);
}


