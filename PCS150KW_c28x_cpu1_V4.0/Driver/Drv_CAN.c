//#############################################################################
//
// FILE:   Drv_CAN.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

#include <Driver/device/device.h>
#include <Driver/device/driverlib.h>
#include "Drv_Timer.h"

//
// CAN External Loopback
//
#define GPIO_CFG_CANRXA      GPIO_36_CANA_RX  // "pinConfig" for CANA RX
#define GPIO_CFG_CANTXA      GPIO_37_CANA_TX  // "pinConfig" for CANA TX
#define GPIO_CFG_CANRXB      GPIO_39_CANB_RX  // "pinConfig" for CANB RX
#define GPIO_CFG_CANTXB      GPIO_38_CANB_TX  // "pinConfig" for CANB TX



void Drv_CAN_Init(void)
{
    // Configuring the GPIOs for CAN A.
    GPIO_setPinConfig(GPIO_CFG_CANRXA);
    GPIO_setPinConfig(GPIO_CFG_CANTXA);
    // Allocate Shared Peripheral CAN A to the CM Side.
    SysCtl_allocateSharedPeripheral(SYSCTL_PALLOCATE_CAN_A,0x1U);

    // Configuring the GPIOs for CAN B.
    GPIO_setPinConfig(GPIO_CFG_CANRXB);
    GPIO_setPinConfig(GPIO_CFG_CANTXB);
    // Allocate Shared Peripheral CAN B to the CM Side.
    SysCtl_allocateSharedPeripheral(SYSCTL_PALLOCATE_CAN_B,0x1U);
}


