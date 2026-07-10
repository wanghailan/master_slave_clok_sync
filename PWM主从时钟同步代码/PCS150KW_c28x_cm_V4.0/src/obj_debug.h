/*
 * obj_debug.h
 *
 *  Created on: 2025Äê6ÔÂ24ÈÕ
 *      Author: guowei
 */

#ifndef __OBJ_DEBUG_H__
#define __OBJ_DEBUG_H__

#include <stdint.h>


void udpDebug_Init(void);
void udpDebug_TxData(uint8_t *buf,int16_t len);

#endif
