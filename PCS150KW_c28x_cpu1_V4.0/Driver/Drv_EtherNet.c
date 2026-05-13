//#############################################################################
//
// FILE:   Drv_EtherNet.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

#include <Driver/device/device.h>
#include <Driver/device/driverlib.h>
#include "Drv_EtherNet.h"


//*****************************************************************************
//
// EtherNet Configurations
//
//*****************************************************************************
void Drv_EtherNetPinInit(void)
{
   //
   // Set up EnetCLK to use SYSPLL as the clock source and set the
   // clock divider to 2.
   //
   // This way we ensure that the PTP clock is 100 MHz. Note that this value
   // is not automatically/dynamically known to the CM core and hence it needs
   // to be made available to the CM side code beforehand.
   SysCtl_setEnetClk(SYSCTL_ENETCLKOUT_DIV_2, SYSCTL_SOURCE_SYSPLL);

   //
   // Configure the GPIOs for ETHERNET.
   //

   //
   // MDIO Signals
   //
   GPIO_setPinConfig(GPIO_105_ENET_MDIO_CLK);
   GPIO_setPinConfig(GPIO_106_ENET_MDIO_DATA);

   //
   // Use this only for RMII Mode
   //GPIO_setPinConfig(GPIO_73_ENET_RMII_CLK);
   //

   //
   //MII Signals
   //
   GPIO_setPinConfig(GPIO_109_ENET_MII_CRS);
   GPIO_setPinConfig(GPIO_110_ENET_MII_COL);

   GPIO_setPinConfig(GPIO_121_ENET_MII_TX_DATA0);
   GPIO_setPinConfig(GPIO_122_ENET_MII_TX_DATA1);
   GPIO_setPinConfig(GPIO_123_ENET_MII_TX_DATA2);
   GPIO_setPinConfig(GPIO_124_ENET_MII_TX_DATA3);

   //
   //Use this only if the TX Error pin has to be connected
   //GPIO_setPinConfig(GPIO_46_ENET_MII_TX_ERR);
   //

   GPIO_setPinConfig(GPIO_118_ENET_MII_TX_EN);

   GPIO_setPinConfig(GPIO_114_ENET_MII_RX_DATA0);
   GPIO_setPinConfig(GPIO_115_ENET_MII_RX_DATA1);
   GPIO_setPinConfig(GPIO_116_ENET_MII_RX_DATA2);
   GPIO_setPinConfig(GPIO_117_ENET_MII_RX_DATA3);
   GPIO_setPinConfig(GPIO_113_ENET_MII_RX_ERR);
   GPIO_setPinConfig(GPIO_112_ENET_MII_RX_DV);

   GPIO_setPinConfig(GPIO_120_ENET_MII_TX_CLK);
   GPIO_setPinConfig(GPIO_111_ENET_MII_RX_CLK);

   //
   //Power down pin to bring the external PHY out of Power down
   //
   GPIO_setDirectionMode(108, GPIO_DIR_MODE_OUT);
   GPIO_setPadConfig(108, GPIO_PIN_TYPE_PULLUP);
   GPIO_writePin(108,1);

   //
   //PHY Reset Pin to be driven High to bring external PHY out of Reset
   //
   //PHY_RSTn
   GPIO_setDirectionMode(119, GPIO_DIR_MODE_OUT);
   GPIO_setPadConfig(119, GPIO_PIN_TYPE_PULLUP);
   GPIO_writePin(119,1);
}





