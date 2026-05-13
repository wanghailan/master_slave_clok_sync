//#############################################################################
// TITLE:  Ethernet PTP Basic Master Example
//! <h1> Ethernet PTP Basic Master </h1>
//!
//! This example configures the device in IEEE PTPv2 Master mode and then
//! periodically sends Sync packets to the slave. On receiving the DelayReq
//! packets from the slave, the master also sends out the DelayResp packets.
//!
//! \b External \b Connections \n
//! This example programs the Ethernet module in PTP Basic Master mode.
//! The example project \e Ethernet \e PTP \e Basic \e Slave is intended to
//! be used along with this project to see the whole PTP Protocol state in
//! action. The second device is configured as \e Slave and both devices in
//! conjunction exchange Sync, DelayReq and DelayResp packets.
//!
//! Refer to the C28x CPU1 code of ethernet_config_c28x project for configuring
//! the PTP clock that drives the system time counter on the Ethernet module.
//!
//! \b Watch \b Variables \n
//!  - gPtpMasterState
//!
//
//#############################################################################

#include "driverlib_cm.h"
#include "string.h"
#include "cm.h"

void ptp_master_init();

void ptp_master_run();

