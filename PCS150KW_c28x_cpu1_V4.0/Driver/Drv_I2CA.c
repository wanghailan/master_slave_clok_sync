//#############################################################################
//
// FILE:   Drv_I2CA.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

#include <Driver/device/device.h>
#include <Driver/device/driverlib.h>
#include "Drv_Timer.h"
//
// I2C
//
#define I2CA_GPIO_PIN_SDA           31U  // GPIO number for I2C SDAA
#define I2CA_GPIO_PIN_SCL           32U  // GPIO number for I2C SCLA
#define I2CA_GPIO_SDA_CONFIG        GPIO_31_GPIO31  // "pinConfig" for I2C SDAA
#define I2CA_GPIO_SCL_CONFIG        GPIO_32_GPIO32  // "pinConfig" for I2C SCLA
//#define I2CA_GPIO_SDA_CONFIG        GPIO_31_CM_I2CA_SDA  // "pinConfig" for I2C SDAA
//#define I2CA_GPIO_SCL_CONFIG        GPIO_32_CM_I2CA_SCL  // "pinConfig" for I2C SCLA


void Drv_I2CAPin_Init(void)
{
    GPIO_setPinConfig(I2CA_GPIO_SDA_CONFIG);
    GPIO_setPadConfig(I2CA_GPIO_PIN_SDA, GPIO_PIN_TYPE_OD);
    GPIO_setDirectionMode(I2CA_GPIO_PIN_SDA, GPIO_DIR_MODE_OUT);

    GPIO_setPinConfig(I2CA_GPIO_SCL_CONFIG);
    GPIO_setPadConfig(I2CA_GPIO_PIN_SCL, GPIO_PIN_TYPE_OD);
    GPIO_setDirectionMode(I2CA_GPIO_PIN_SCL, GPIO_DIR_MODE_OUT);

    GPIO_setMasterCore(I2CA_GPIO_PIN_SDA, GPIO_CORE_CM);
    GPIO_setMasterCore(I2CA_GPIO_PIN_SCL, GPIO_CORE_CM);
}


