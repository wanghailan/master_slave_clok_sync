/*
 * initial_CLA.h
 *
 *  Created on: 2024年9月16日
 *      Author: guowei680
 */

#ifndef CLA_INITIAL_CLA_H_
#define CLA_INITIAL_CLA_H_

#include <stdint.h>
#include "CLAmath.h"
#include "Drv_PWM.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// Included Files
//
#include "driverlib.h"
#include "device.h"
#include "Drv_PWM.h"
#include "Pcs_setting.h"
#include "DCLCLA.h"

//#define  SQRT2  1.4142135623731f
//#define  SQRT3  1.7320508075689f
//#define  PI2    2.0f*PI

#define   lFilter_a         0.01925685f  //100HZ低通滤波

#define  float32    float

#define   Vmax_Buck             850.0f  //Buck最大控制电压
#define   Imax_Buck             190.0f   //Buck最大控制电流


//BUCK模式 电压外环PI控制限幅
#define   BUCK_VO_OUTLIMP                   ( 1.2f*Imax_Buck)
#define   BUCK_VO_OUTLIMN                   (-1.2f*Imax_Buck)

//BUCK模式 电流内环PI控制限幅
#define   BUCK_IL_OUTLIMP                   ( 1.2f*Vmax_Buck)
#define   BUCK_IL_OUTLIMN                   (-1.2f*Vmax_Buck)

//逆变电感电流前馈系数比例
#define   INV3P_IINVL_KFEED             1

#define   Tcylce        0.00003125f

//BUCK电压外环PI系数
#define   BUCK_VO_KP      1.5f
#define   BUCK_VO_KI     (1.0f*Tcylce)

//BUCK电压外环PI系数
#define   BUCK_IO_KP      10.0f
#define   BUCK_IO_KI     (20.0f*Tcylce)

//BUCK电流内环PI系数
#define   BUCK_IL_KP      3.0f
#define   BUCK_IL_KI      (2.0f*Tcylce)


enum
{
    CLA_Mode_Buck   = 0,
    CLA_Mode_Boost  = 1
};




typedef struct
{
    int16_t         PwmStartEnable;
    int16_t         WorkMode;//工作模式(0:buck,1:boost)
    int16_t         CtrlMode;//控制模式(0:开环,1:恒压,2:恒流,3:恒功率,4:恒阻)
    int16_t         OutLoop_En; //电流外环使能    0：关闭外环，1：打开外环

    float32_t       VBat_Calibrat;      //电池电压校准    (80.00%~120.00%) 8000~12000
    float32_t       VBus_Calibrat;      //母线电压校准
    float32_t       VBatS_Calibrat;     //电池软起电压校准
    float32_t       VBusS_Calibrat;     //母线软起电压校准
    float32_t       VBatP_Calibrat;     //电池电压正校准
    float32_t       VBatN_Calibrat;     //电池电压负校准

    float32_t       IBat_Calibrat;      //电池电流校准
    float32_t       IBus_Calibrat;      //母线电流校准
    float32_t       IL1_Calibrat;       //单元1电流校准
    float32_t       IL2_Calibrat;       //单元2电流校准
    float32_t       IL3_Calibrat;       //单元3电流校准

    float32_t       Ref1v5_Calibrat;    //1.5V偏置校准
    float32_t       NTC1_Calibrat;
    float32_t       NTC2_Calibrat;

    float32_t       Vdc7_Calibrat;
    float32_t       Vdc8_Calibrat;
    float32_t       Vdc9_Calibrat;

    float32_t       Idc5_Calibrat;
    float32_t       Idc6_Calibrat;

    float32_t       VDC_Bat_offset;     //高压侧端口电压偏置
    float32_t       VDC_Bus_offset;     //低压侧端口电压偏置
    float32_t       VDC_BatS_offset;    //高压侧软起电压偏置
    float32_t       VDC_BusS_offset;    //低压侧软起电压偏置

    float32_t       IBat_offset;        //电池电流偏置校准
    float32_t       IBus_offset;        //母线电流偏置校准
    float32_t       IL1_offset;         //单元1电流偏置校准
    float32_t       IL2_offset;         //单元2电流偏置校准
    float32_t       IL3_offset;         //单元3电流偏置校准

    float32_t       DcDc_VdcRef;    //恒压模式电压给定
    float32_t       DcDc_IdcRef;    //恒流模式电流给定
    float32_t       DcDc_PdcRef;    //恒功率模式功率给定
    float32_t       DcDc_RdcRef;    //恒阻模式电阻给定

    float32_t       VOCtrl_Kp;              //电压外环Kp参数
    float32_t       VOCtrl_Ki;              //电压外环Ki参数
    float32_t       IOCtrl_Kp;              //电流外环Kp参数
    float32_t       IOCtrl_Ki;              //电流外环Ki参数
    float32_t       IICtrl_Kp;              //电流内环Kp参数
    float32_t       IICtrl_Ki;              //电流内环Ki参数

    float32_t       DcBusMaxVol;            //低压侧最大输出电压
    float32_t       DcBatMaxVol;            //高压侧最大输出电压
    float32_t       DcBusMaxCurrent;        //低压侧最大输出电流
    float32_t       DcBatMaxCurrent;        //高压侧最大输出电流

    int16_t         PcsDebugi16_0;
    int16_t         PcsDebugi16_1;
    int16_t         PcsDebugi16_2;

    float32_t       PcsDebugf32_0;
    float32_t       PcsDebugf32_1;
    float32_t       PcsDebugf32_2;

    int16_t         MPPT_En;
    float32_t       Vdc_MPP;
    float32_t       MPPT_Duty;

}T_CPU2CLA_VARIABLE;

typedef struct
{
    float32_t       DC_VBat_sensed_pu;    //电池侧电压采样标幺值
    float32_t       DC_VBus_sensed_pu;    //电网侧电压采样标幺值
    float32_t       DC_VBatS_sensed_pu;   //电池侧软起电压采样标幺值
    float32_t       DC_VBusS_sensed_pu;   //电网侧软起电压采样标幺值
    float32_t       DC_VBatP_sensed_pu;   //直流电压正采样标幺值
    float32_t       DC_VBatN_sensed_pu;   //直流电压负采样标幺值

    float32_t       DC_IBat_sensed_pu;    //电池侧电流采样标幺值
    float32_t       DC_IBus_sensed_pu;    //电网侧电流采样标幺值
    float32_t       DC_IL1_sensed_pu;     //单元1电流采样标幺值
    float32_t       DC_IL2_sensed_pu;     //单元2电流采样标幺值
    float32_t       DC_IL3_sensed_pu;     //单元3电流采样标幺值

    float32_t       DC_NTC1_sensed_pu;    //NTC1温度采样标幺值
    float32_t       DC_NTC2_sensed_pu;    //NTC2温度采样标幺值

    float32_t       DC_Vdc7_sensed_pu;
    float32_t       DC_Vdc8_sensed_pu;
    float32_t       DC_Vdc9_sensed_pu;

    float32_t       DC_Idc5_sensed_pu;
    float32_t       DC_Idc6_sensed_pu;

    float32_t       PwmRatio1;    //单元1PWM控制比例
    float32_t       PwmRatio2;    //单元2PWM控制比例
    float32_t       PwmRatio3;    //单元2PWM控制比例

    int16_t         PcsDebugi16_0;
    int16_t         PcsDebugi16_1;
    int32_t         PcsDebugi16_2;

    float32_t       PcsDebugf32_0;
    float32_t       PcsDebugf32_1;
    float32_t       PcsDebugf32_2;

}T_CLA2CPU_VARIABLE;


typedef struct
{
    float32_t       f32_DRatio1;    //单元1PWM控制比例
    float32_t       f32_DRatio2;    //单元2PWM控制比例
    float32_t       f32_DRatio3;    //单元3PWM控制比例

    float32_t       DcDc_VdcRef;    //恒压模式电压给定
    float32_t       DcDc_IdcRef;    //恒流模式电流给定
    float32_t       DcDc_PdcRef;    //恒功率模式功率给定
    float32_t       DcDc_RdcRef;    //恒阻模式电阻给定

    float32_t       DcDc_IoRefP1;   //单元1 参考电流给定
    float32_t       DcDc_IoRefP2;   //单元2 参考电流给定
    float32_t       DcDc_IoRefP3;   //单元2 参考电流给定
    //外环PI参数
    float32_t       DcDc_VRegIOut;   //外环积分参数
    float32_t       DcDc_VRegOut;    //外环PI参数
    float32_t       DcDc_ErrorVo;    //电压外环控制误差
    float32_t       DcDc_ErrorVo_old;    //电压外环控制误差

    //电压外环PI参数
    float32_t       DcDc_VKp;
    float32_t       DcDc_VKi;
    //电流外环PI参数
    float32_t       DcDc_IKp;
    float32_t       DcDc_IKi;
    //电流内环PI参数
    float32_t       DcDc_ILKp;      //电流内环Kp
    float32_t       DcDc_ILKi;      //电流内环Ki

    float32_t       DcDc_Ip1RegIOut; //单元1电流内环积分输出
    float32_t       DcDc_Ip1RegOut;  //单元1电流内环PI输出
    float32_t       DcDc_ErrorI1;    //单元1 PI差值
    float32_t       DcDc_I1PiOut;    //单元1 电流PI控制输出

    float32_t       DcDc_Ip2RegIOut; //单元2电流内环积分输出
    float32_t       DcDc_Ip2RegOut;  //单元2电流内环PI输出
    float32_t       DcDc_ErrorI2;    //单元2 PI差值
    float32_t       DcDc_I2PiOut;    //单元2 电流PI控制输出

    float32_t       DcDc_Ip3RegIOut; //单元3电流内环积分输出
    float32_t       DcDc_Ip3RegOut;  //单元3电流内环PI输出
    float32_t       DcDc_ErrorI3;    //单元3 PI差值
    float32_t       DcDc_I3PiOut;    //单元3 电流PI控制输出

    float32_t       DC_VBat_pu;    //电池侧电压采样标幺值
    float32_t       DC_VBus_pu;    //电网侧电压采样标幺值
    float32_t       DC_VBatS_pu;   //电池侧软起电压采样标幺值
    float32_t       DC_VBusS_pu;   //电网侧软起电压采样标幺值
    float32_t       DC_VBatP_pu;   //直流电压正采样标幺值
    float32_t       DC_VBatN_pu;   //直流电压负采样标幺值

    float32_t       DC_IBat_pu;    //电池侧电流采样标幺值
    float32_t       DC_IBus_pu;    //电网侧电流采样标幺值
    float32_t       DC_IL1_pu;     //单元1电流采样标幺值
    float32_t       DC_IL2_pu;     //单元2电流采样标幺值
    float32_t       DC_IL3_pu;     //单元3电流采样标幺值

    float32_t       DC_NTC1_pu;    //NTC1温度采样标幺值
    float32_t       DC_NTC2_pu;    //NTC2温度采样标幺值

    float32_t       DC_Vdc7_pu;
    float32_t       DC_Vdc8_pu;
    float32_t       DC_Vdc9_pu;

    float32_t       DC_Idc5_pu;
    float32_t       DC_Idc6_pu;

    float32_t       DC_IMax_Bat;
    float32_t       DC_IMax_Bus;
    float32_t       DC_VMax_Bat;
    float32_t       DC_VMax_Bus;

    float32_t       DC_IL1_Live;     //单元1实时电流
    float32_t       DC_IL2_Live;     //单元2实时电流
    float32_t       DC_IL3_Live;     //单元3实时电流
    float32_t       DC_IBat_Live;    //高压侧实时电流
    float32_t       DC_IBus_Live;    //低压侧实时电流
    float32_t       DC_Vbus_Live;    //低压侧实时电压
    float32_t       DC_Vbat_Live;    //高压侧实时电压
    float32_t       DC_VbatP_Live;   //高压侧正实时电压
    float32_t       DC_VbatN_Live;   //高压侧负实时电压

    float32_t       lpFilter_Yold_V1;
    float32_t       lpFilter_Yold_V2;
    float32_t       lpFilter_Yold_V3;
    float32_t       lpFilter_Yold_V4;
    float32_t       lpFilter_Yold_V5;
    float32_t       lpFilter_Yold_V6;
    float32_t       lpFilter_Yold_V7;
    float32_t       lpFilter_Yold_V8;
    float32_t       lpFilter_Yold_V9;

    float32_t       lpFilter_Yold_I1;
    float32_t       lpFilter_Yold_I2;
    float32_t       lpFilter_Yold_I3;
    float32_t       lpFilter_Yold_I4;
    float32_t       lpFilter_Yold_I5;
    float32_t       lpFilter_Yold_I6;
    float32_t       lpFilter_Yold_I7;

    int16_t         Pwm_Enable;
    int16_t         PcsDebugi16_0;
    int16_t         PcsDebugi16_1;
    int32_t         PcsDebugi16_2;

    float32_t       Delta_Power;
    float32_t       Delta_Voltage;
    float32_t       Vdc_MPP;
    int16_t         MPPT_Enable;
//
//    float32       PcsDebugf32_0;
//    float32       PcsDebugf32_1;
//    float32       PcsDebugf32_2;

}T_CLA_PRIVATE_VARIABLE;


extern volatile T_CPU2CLA_VARIABLE tCpu2Cla;
extern volatile T_CLA2CPU_VARIABLE tCla2Cpu;
extern volatile T_CLA_PRIVATE_VARIABLE tClaPrivate;
//
// Globals
//
//Task 1 (C) Variables

//Task 2 (C) Variables

//Task 3 (C) Variables

//Task 4 (C) Variables

//Task 5 (C) Variables

//Task 6 (C) Variables

//Task 7 (C) Variables

//Task 8 (C) Variables

//Common (C) Variables

//
//CLA C Tasks
//
__interrupt void Cla1Task1();
__interrupt void Cla1Task2();
__interrupt void Cla1Task3();
__interrupt void Cla1Task4();
__interrupt void Cla1Task5();
__interrupt void Cla1Task6();
__interrupt void Cla1Task7();
__interrupt void Cla1Task8();

void initial_CLA(void);

#ifdef __cplusplus
}
#endif // extern "C"

#endif /* CLA_INITIAL_CLA_H_ */
