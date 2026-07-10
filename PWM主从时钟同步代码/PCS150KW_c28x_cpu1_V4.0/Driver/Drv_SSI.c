//#############################################################################
//
// FILE:   Drv_SSI.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

#include <Driver/device/device.h>
#include <Driver/device/driverlib.h>
#include "Drv_Timer.h"

//
// SSI 配置
//
#define GPIO_CFG_SSIA_TX       GPIO_54_SSIA_TX   // "pinConfig" for MOSI
#define GPIO_CFG_SSIA_RX       GPIO_55_SSIA_RX   // "pinConfig" for MISO
#define GPIO_CFG_SSIA_CLK      GPIO_56_SSIA_CLK  // "pinConfig" for CANB RX
#define GPIO_CFG_SSIA_FSS      GPIO_57_SSIA_FSS  // "pinConfig" for CANB TX



void Drv_SSI_Init(void)
{
    // Configuring the GPIOs for SSIA.
    GPIO_setPinConfig(GPIO_CFG_SSIA_TX);
    GPIO_setPinConfig(GPIO_CFG_SSIA_RX);
    GPIO_setPinConfig(GPIO_CFG_SSIA_CLK);
    GPIO_setPinConfig(GPIO_CFG_SSIA_FSS);
//    // Allocate Shared Peripheral SSIA to the CM Side.
//    SysCtl_allocateSharedPeripheral(SYSCTL_PALLOCATE_CAN_A,0x1U);
}


