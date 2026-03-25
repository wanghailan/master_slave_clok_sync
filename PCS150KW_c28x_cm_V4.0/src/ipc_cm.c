//#############################################################################
//
// FILE:   IPC.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

#include "driverlib_cm.h"
#include "cm.h"
#include "bsp.h"

#define IPC_CMD_DEBUG   0x3001

#pragma DATA_SECTION(pDatCmtoCpu1, "MSGRAM_CM_TO_CPU1")
uint16_t pDatCmtoCpu1[PACKET_LENGTH];

int16_t cpuIpc_flag = 0;
int16_t IpcCpu2Cm_ErrCnt = 0;

float ipcRx_DebugData[20];
int8_t flag_ipcRx = 0;

void IPC_ISR0();

void ipc_init(void)
{
      // Clear any IPC flags if set already
      IPC_clearFlagLtoR(IPC_CM_L_CPU1_R, IPC_FLAG_ALL);
      // Enable IPC interrupts
      IPC_registerInterrupt(IPC_CM_L_CPU1_R, IPC_INT0, IPC_ISR0);
      // Synchronize both the cores.
      IPC_sync(IPC_CM_L_CPU1_R, IPC_FLAG31);
}
//
// IPC ISR for Flag 1
// C28x core sends data with message queue using Flag 0
//
void IPC_ISR0()
{
    uint32_t command, addr, data;

    IpcCpu2Cm_ErrCnt = 0;
    // Read the command
    IPC_readCommand(IPC_CM_L_CPU1_R, IPC_FLAG0, IPC_ADDR_CORRECTION_ENABLE, &command, &addr, &data);

    if(command == IPC_CMD_DEBUG)
    {
        memcpy(ipcRx_DebugData,(float *)addr,data*4);
        flag_ipcRx = 1;
    }
    else
    {
        switch(command-IpcCmd_TX)
        {
            case ipcCmd_Param:
                memcpy(&CmIpc_cpu2cm,(uint16_t *)addr,data*2);
                cpuIpc_flag = 1;
                break;
            case ipcCmd_SciC:
                memcpy(Mod1_RxBuf,(uint16_t *)addr,data*2);
                Mod1_RxCnt = data;
                break;
            case ipcCmd_SciD:
                memcpy(Mod2_RxBuf,(uint16_t *)addr,data*2);
                Mod2_RxCnt = data;
                break;
            default:break;
        }
    }
    // Acknowledge the flag
    IPC_ackFlagRtoL(IPC_CM_L_CPU1_R, IPC_FLAG0);
}


void ipc_TxData_CmToCpu2(int16_t cmd,uint16_t *txBuf,int16_t txlen)
{
    uint32_t command;
    uint32_t len;

    len = txlen;
    switch(cmd)
    {
        case ipcCmd_Param:
            command = IpcCmd_RX+cmd;
            memcpy(pDatCmtoCpu1,&CmIpc_cm2cpu,len*2);
            break;
        case ipcCmd_SciC:
            command = IpcCmd_RX+cmd;
            memcpy(pDatCmtoCpu1,txBuf,len*2);
            break;
        case ipcCmd_SciD:
            command = IpcCmd_RX+cmd;
            memcpy(pDatCmtoCpu1,txBuf,len*2);
            break;
        default:break;
    }
    if(cmd != ipcCmd_null)
    {
        IPC_sendCommand(IPC_CM_L_CPU1_R, IPC_FLAG0, IPC_ADDR_CORRECTION_ENABLE,command, (uint32_t)pDatCmtoCpu1, len);
        IPC_waitForAck(IPC_CM_L_CPU1_R, IPC_FLAG0);
    }
}

//



