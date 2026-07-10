/*
 * obj_ipc.h
 *
 *  Created on: 2024Äê10ÔÂ11ÈÕ
 *      Author: guowei680
 */

#ifndef OBJ_IPC_H_
#define OBJ_IPC_H_


#define IPC_CMD_READ_MEM   0x1001
#define IPC_CMD_RESP       0x2001

#define PACKET_LENGTH 132

#define TEST_PASS          0x5555
#define TEST_FAIL          0xAAAA

extern float32_t packetData[PACKET_LENGTH];

void ipc_init(void);
//void ipc_GetData(uint32_t Cmd,int32_t *TxDat,int32_t *RxDat);
void ipc_TxData(int16_t len);
#endif /* obj_ipc.h */
