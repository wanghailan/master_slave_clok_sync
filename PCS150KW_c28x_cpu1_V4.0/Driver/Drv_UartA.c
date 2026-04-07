//#############################################################################
//
// FILE:   Drv_UARTA.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

#include <Driver/device/device.h>
#include <Driver/device/driverlib.h>
#include <Drv_UartA.h>

//*****************************************************************************
//
// PINMUX Configurations
//
//*****************************************************************************

void Drv_UartAPin_Init(void)
{
    // UARTA ->Pinmux
    GPIO_setPinConfig(UARTA_RX_PIN_CONFIG);
    GPIO_setDirectionMode(UARTA_RX_GPIO, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(UARTA_RX_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(UARTA_RX_GPIO, GPIO_QUAL_ASYNC);

    GPIO_setPinConfig(UARTA_TX_PIN_CONFIG);
    GPIO_setDirectionMode(UARTA_TX_GPIO, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(UARTA_TX_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(UARTA_TX_GPIO, GPIO_QUAL_ASYNC);
}


