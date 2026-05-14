//#############################################################################
//
// FILE:   obj_ipc.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

#include <Driver/device/device.h>
#include <Driver/device/driverlib.h>
#include "bsp.h"

#pragma DATA_SECTION(ptDataCpu1ToCm, "MSGRAM_CPU_TO_CM")
uint16_t ptDataCpu1ToCm[PACKET_LENGTH];
//#pragma DATA_SECTION(debugpData, "MSGRAM_CPU_TO_CM")
float32_t debugpData[20];


int16_t IpcCm2Cpu_ErrCnt = 0;
__interrupt void IPC_ISR0();

void ipc_init(void)
{
     //Clear any IPC flags if set already
    IPC_clearFlagLtoR(IPC_CPU1_L_CM_R, IPC_FLAG_ALL);
    // Enable IPC interrupts
    IPC_registerInterrupt(IPC_CPU1_L_CM_R, IPC_INT0, IPC_ISR0);
    // Synchronize both the cores.
    IPC_sync(IPC_CPU1_L_CM_R, IPC_FLAG31);
}

int16_t ipcTx_flag = 0;
void ipc_TxData_Cpu1ToCM(int16_t cmd,int16_t Tlen)
{
    uint32_t command;
    uint32_t len;

    while(ipcTx_flag)
    {
        Drv_Timer_usDelay(100);
    }
    ipcTx_flag = 1;

    len = Tlen;
    if(cmd == IPC_CMD_DEBUG)
    {
        command = IPC_CMD_DEBUG;
        memcpy(ptDataCpu1ToCm,debugpData,len*2);
    }
    else
    {
        switch(cmd)
        {
            case ipcCmd_Param:
                command = IpcCmd_TX+cmd;
                memcpy(ptDataCpu1ToCm,&Cpu1Ipc_cpu2cm,len);
                break;
            case ipcCmd_SciC:
                command = IpcCmd_TX+cmd;
                memcpy(ptDataCpu1ToCm,SCIC_RxBuf,len);
                break;
            case ipcCmd_SciD:
                command = IpcCmd_TX+cmd;
                memcpy(ptDataCpu1ToCm,SCID_RxBuf,len);
                break;
            default:break;
        }
    }
    if(cmd != ipcCmd_null)
    {
        IPC_sendCommand(IPC_CPU1_L_CM_R, IPC_FLAG0, IPC_ADDR_CORRECTION_ENABLE,command, (uint32_t)ptDataCpu1ToCm, len);
        IPC_waitForAck(IPC_CPU1_L_CM_R, IPC_FLAG0); // Wait for acknowledgment
    }

    ipcTx_flag = 0;
}


__interrupt void IPC_ISR0()
{
    uint32_t command, addr, data;

    IpcCm2Cpu_ErrCnt = 0;
    IPC_readCommand(IPC_CPU1_L_CM_R, IPC_FLAG0, IPC_ADDR_CORRECTION_ENABLE,&command, &addr, &data);

    switch(command-IpcCmd_RX)
    {
        case ipcCmd_Param:
            memcpy(&Cpu1Ipc_cm2cpu,(uint16_t *)addr,data);
            break;
        case ipcCmd_SciC:
            memcpy(SCIC_TxBuf,(uint16_t *)addr,data);
            SCIC_TxCnt = data;
            SCIC_TxFlag = 1;
            break;
        case ipcCmd_SciD:
            memcpy(SCID_TxBuf,(uint16_t *)addr,data);
            SCID_TxCnt = data;
            SCID_TxFlag = 1;
            break;
        default:break;
    }

    IPC_ackFlagRtoL(IPC_CPU1_L_CM_R, IPC_FLAG0);
    // Acknowledge the PIE interrupt.
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP11); //CMTOCPUx IPC interrupt 在GROUP11中
}



