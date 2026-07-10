//#############################################################################
//
// FILE:   obj_ipc.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

#include <Driver/device/device.h>
#include <Driver/device/driverlib.h>
#include "obj_ipc.h"
#include "bsp.h"


#pragma DATA_SECTION(packetData, "MSGRAM_CPU_TO_CM")
float32_t packetData[PACKET_LENGTH];

uint32_t pass;

//IPC_MessageQueue_t messageQueue;
//IPC_Message_t      TxMsg, RxMsg;

void ipc_init(void)
{
//    // Clear any IPC flags if set already
//    IPC_clearFlagLtoR(IPC_CPU1_L_CM_R, IPC_FLAG_ALL);
//
//    // Initialize message queue
//    IPC_initMessageQueue(IPC_CPU1_L_CM_R, &messageQueue, IPC_INT1, IPC_INT1);

    // Synchronize both the cores
//    IPC_sync(IPC_CPU1_L_CM_R, IPC_FLAG31);

    // Clear any IPC flags if set already
    IPC_clearFlagLtoR(IPC_CPU1_L_CM_R, IPC_FLAG_ALL);

    // Synchronize both the cores.
    IPC_sync(IPC_CPU1_L_CM_R, IPC_FLAG31);
}

void ipc_TxData(int16_t len)
{
    //
    // Send a message without message queue
    // Since C28x and CM does not share the same address space for shared RAM,
    // ADDRESS_CORRECTION is enabled
    // Length of the data to be read is passed as data.
    //
    IPC_sendCommand(IPC_CPU1_L_CM_R, IPC_FLAG0, IPC_ADDR_CORRECTION_ENABLE,
                    IPC_CMD_READ_MEM, (uint32_t)packetData, len);

//    //
//    // Wait for acknowledgment
//    //
    IPC_waitForAck(IPC_CPU1_L_CM_R, IPC_FLAG0);
//
//    //
//    // Read response
//    //
//    if(IPC_getResponse(IPC_CPU1_L_CM_R) == TEST_PASS)
//    {
//        pass = 1;
//    }
//    else
//    {
//        pass = 0;
//    }
}

//void ipc_GetData(uint32_t Cmd,int32_t *TxDat,int32_t *RxDat)
//{
//    int16_t i;
//    //
//    // Update the message
//    //
//    for(i=0; i<10; i++)
//        ipcTxData[i] = TxDat[i];
//    TxMsg.command = Cmd;
//    TxMsg.address = (uint32_t)ipcTxData;
//    TxMsg.dataw1  = 10;  // Using dataw1 as data length
//    TxMsg.dataw2  = 1;   // Message identifier
//
//    //
//    // Send message to the queue
//    //
//    IPC_sendMessageToQueue(IPC_CPU1_L_CM_R, &messageQueue, IPC_ADDR_CORRECTION_ENABLE,
//                           &TxMsg, IPC_BLOCKING_CALL);
//
//    //
//    // Read message from the queue
//    //
////    IPC_readMessageFromQueue(IPC_CPU1_L_CM_R, &messageQueue, IPC_ADDR_CORRECTION_DISABLE,
////                             &RxMsg, IPC_BLOCKING_CALL);
////
////    if((RxMsg.command == IPC_CMD_RESP) && (RxMsg.dataw1 == 30) && (RxMsg.dataw2 == 1))
////    {
////        for(i=0; i<30; i++)
////            RxDat[i] = *((uint32_t *)RxMsg.address + i);
////    }
//
//}

