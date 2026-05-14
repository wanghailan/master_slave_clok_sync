/*
 * obj_Control.h
 *
 *  Created on: 2025年02月14日
 *      Author: guowei680
 */

#ifndef OBJ_CONTROLH_
#define OBJ_CONTROLH_

#include "bsp.h"
#include "rampgen.h"

typedef enum{
    PowerOn_Mode = 0,
    PcsOff_Mode,    //停机
    PreStart_Mode,  //预启动
    DcSoft_Mode,    //直流软起
    DcStart_Mode,   //直流主继电器启动
    AcSoft_Mode,    //交流软起
    AcStart_Mode,   //交流主继电器启动
    PcsOn_Mode      //启动
}PCS_CTRL_STATE;

typedef enum{
    GridDisConnectMode = 0, //离网
    GridConnectMode    //并网
}PCS_WORK_MODE;

extern RAMPGEN PCS_rgen;

extern float32_t Pcs_Localpllfo;
extern float32_t Pcs_Gridpllfo;

void objControl_globalVarInit(void);
inline void Pcs_runISR1(void);
void     objControl_DOControl(void);
void     objControl_FanControl(void);
void     objControl_StateCtrl(void);
void     objControl_LedControl(void);
int16_t  Pcs_InvCtrl_SoftStar(void);
int16_t  Pcs_DcConstVolCtrl_SoftStar(void);
int16_t  Pcs_AcConstCurrCtrl_SoftStar(void);
int16_t  Pcs_DcConstCurrCtrl_SoftStar(void);
int16_t  Pcs_PQConstCtrl_SoftStar(void);

#endif /* obj_Control.h */
