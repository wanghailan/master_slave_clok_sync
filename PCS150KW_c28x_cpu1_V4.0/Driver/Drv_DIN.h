/*
 * Drv_DIN.h
 *
 *  Created on: 2024年9月11日
 *      Author: guowei680
 */

#ifndef DRIVER_DRV_DIN_H_
#define DRIVER_DRV_DIN_H_

enum
{
    Din_EStop = 1, //急停输入
    Din_ReMCtrl,   //远程控制
    Din_IdcCmp,    //直流过流硬件比较
    Din_in1,       //扩展输入1
    Din_in2,       //扩展输入2
    Din_in3,       //扩展输入3
    Din_Addr1,
    Din_Addr2,
    Din_Addr3,
    Din_Addr4
};


void Drv_DinPin_Init(void);
int16_t Drv_DinStatusGet(int16_t RelayIndex);

#endif /* DRIVER_DRV_DIN_H_ */
