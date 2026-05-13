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
// LEDs
//
#define DEVICE_GPIO_PIN_LED3        33U  // GPIO number for LD1

#define DEVICE_GPIO_PIN_LEDWork     82U  // GPIO number for work led
#define DEVICE_GPIO_PIN_LEDFault    149U  // GPIO number for fault led

void Drv_Led_Init(void)
{
    Drv_Led_Off();
//    Drv_WorkLed_Off();
//    Drv_FaultLed_Off();
}

void Drv_Led_On(void)
{
    GPIO_writePin(DEVICE_GPIO_PIN_LED3, 1);
}

void Drv_Led_Off(void)
{
    GPIO_writePin(DEVICE_GPIO_PIN_LED3, 0);
}

void Drv_Led_toggle(void)
{
    GPIO_togglePin(DEVICE_GPIO_PIN_LED3);
}

//void Drv_WorkLed_On(void)
//{
//    GPIO_writePin(DEVICE_GPIO_PIN_LEDWork, 1);
//}
//
//void Drv_WorkLed_Off(void)
//{
//    GPIO_writePin(DEVICE_GPIO_PIN_LEDWork, 0);
//}
//
//void Drv_WorkLed_toggle(void)
//{
//    GPIO_togglePin(DEVICE_GPIO_PIN_LEDWork);
//}
//
//void Drv_FaultLed_On(void)
//{
//    GPIO_writePin(DEVICE_GPIO_PIN_LEDFault, 1);
//}
//
//void Drv_FaultLed_Off(void)
//{
//    GPIO_writePin(DEVICE_GPIO_PIN_LEDFault, 0);
//}
//
//void Drv_FaultLed_toggle(void)
//{
//    GPIO_togglePin(DEVICE_GPIO_PIN_LEDFault);
//}


