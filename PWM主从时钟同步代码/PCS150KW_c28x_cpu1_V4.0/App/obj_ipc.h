/*
 * obj_ipc.h
 *
 *  Created on: 2024年10月11日
 *      Author: guowei680
 */

#ifndef OBJ_IPC_H_
#define OBJ_IPC_H_

#define PACKET_LENGTH 256

#define IpcCmd_TX       0x1000
#define IpcCmd_RX       0x2000
#define IPC_CMD_DEBUG   0x3001

enum
{
    ipcCmd_null = 0,
    ipcCmd_SciB = 1,
    ipcCmd_SciC,
    ipcCmd_SciD,
    ipcCmd_Param
};

extern int16_t IpcCm2Cpu_ErrCnt;
extern float32_t debugpData[20];

void ipc_init(void);
void ipc_TxData_Cpu1ToCM(int16_t cmd,int16_t Tlen);
#endif
