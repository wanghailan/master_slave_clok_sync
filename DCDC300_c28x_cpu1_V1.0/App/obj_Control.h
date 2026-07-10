/*
 * obj_Control.h
 *
 *  Created on: 2025年02月14日
 *      Author: guowei680
 */

#ifndef OBJ_CONTROLH_
#define OBJ_CONTROLH_

/* CODEMAP_CONTROL_H
 * State and control interface definitions.
 * DCDC_CTRL_STATE is the startup/shutdown/precharge state machine.
 * DCDC_WORK_MODE selects power direction: Buck or Boost.
 */
#include "bsp.h"


typedef enum{
    PowerOn_Mode = 0,
    DcDcOff_Mode,    //停机
    PreStart_Mode,   //预启动
    BatSoft_Mode,    //高压侧软起
    BatStart_Mode,   //高压侧主启动
    BusSoft_Mode,    //低压侧软起
    BusStart_Mode,   //低压侧主启动
    DcDcOn_Mode      //启动
}DCDC_CTRL_STATE;

typedef enum{
    Mode_Buck = 0, //buck模式
    Mode_Boost,    //boost模式
    Mode_Auto      //自动模式
}DCDC_WORK_MODE;



void objControl_globalVarInit(void);
inline void Dsp_runISR1(void);
int16_t calc_NtcTemp(uint16_t AdcDat);
void objControl_RelayControl(void);
void objControl_FanControl(void);
void objControl_StateCtrl(void);
int16_t  objControl_ConstVolCtrl_SoftStar(void);
int16_t  objControl_ConstPowerCtrl_SoftStar(void);
int16_t  objControl_ConstCurrCtrl_SoftStar(void);


#endif /* obj_Control.h */
