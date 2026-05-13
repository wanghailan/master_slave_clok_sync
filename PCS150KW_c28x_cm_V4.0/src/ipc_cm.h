/*
 * ipc_cm.h
 *
 *  Created on: 2024Äê10ÔÂ11ÈÕ
 *      Author: guowei680
 */

#ifndef IPC_CM_H_
#define IPC_CM_H_


//
// Defines
//

// Defines
//
#define PACKET_LENGTH 256

#define IpcCmd_TX     0x1000
#define IpcCmd_RX     0x2000

enum
{
    ipcCmd_null = 0,
    ipcCmd_SciB = 1,
    ipcCmd_SciC,
    ipcCmd_SciD,
    ipcCmd_Param
};

extern uint16_t ipcRxData[PACKET_LENGTH];
extern int16_t IpcCpu2Cm_ErrCnt;

extern float ipcRx_DebugData[20];
extern int8_t flag_ipcRx;
extern int16_t cpuIpc_flag;

void ipc_init(void);
void ipc_TxData_CmToCpu2(int16_t cmd,uint16_t *txBuf,int16_t txlen);
void ipc_GetData(uint32_t Cmd,uint32_t *TxDat,uint32_t *RxDat);

#endif /* ipc_cm.h */
