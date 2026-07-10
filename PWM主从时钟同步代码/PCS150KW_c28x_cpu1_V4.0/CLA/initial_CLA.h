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
#include "obj_setting.h"
#include "abc_dq0_pos.h"
#include "abc_dq0_neg.h"
#include "dq0_abc.h"
#include "DCLCLA.h"
#include "clarkT.h"
#include "parkT.h"

#define  SQRT2  1.4142135623731f
#define  SQRT3  1.7320508075689f
#define  PI2    2.0f*PI
#define  Lf1    0.000165f  //330uH*2
#define  Lf2    0.000035f  //70uH*2
#define  Cf     0.000024f  //24uF
#define  GridFreq     50.0f  //50HZ

#define  float32    float

#define   Tcylce        0.00003125f //32K 采样周期

//逆变电压瞬时值环Kp系数
#define   INV3P_VINV_KP      0.75f
//逆变电压瞬时值环Ki系数
#define   INV3P_VINV_KI     (10.0f*Tcylce)  //0.0115966796875f

//逆变瞬时电流环Kp,Ki系数
#define   INV3P_IINV_KP      1.06667f
#define   INV3P_IINV_KI      (106.6667f*Tcylce)

//直流恒电流电流环Kp,Ki系数
#define   INV3P_IDC_KP      2.0f
#define   INV3P_IDC_KI      (10.0f*Tcylce)

#define   lFilter_a         0.08939813f  //500HZ低通滤波


enum
{
    CLA_GRID_DISCONNECTED = 0,
    CLA_GRID_CONNECTED  = 1
};

enum
{
    CLA_PHS_OUT_3P3L = 0,
    CLA_PHS_OUT_3P4L = 1
};



typedef struct
{
    int16_t         PwmStartEnable;
    int16_t         WorkMode;//0:离网，1:并网
    int16_t         OnGridMode;//并网工作模式(0:恒功率,1:恒流,2:恒直流电压,3:恒直流电流)
    int16_t         OpenLoopMode;
    int16_t         pcsLineMode;//0:3P3L，1:3P4L
    int16_t         OutLoop_En;

    float32_t       VgridA_Calibrat;    //A相电网电压校准 (80.00%~120.00%) 8000~12000
    float32_t       VgridB_Calibrat;    //B相电网电压校准 (80.00%~120.00%) 8000~12000
    float32_t       VgridC_Calibrat;    //C相电网电压校准 (80.00%~120.00%) 8000~12000
    float32_t       VInvA_Calibrat;     //A相逆变电压校准 (80.00%~120.00%) 8000~12000
    float32_t       VInvB_Calibrat;     //B相逆变电压校准 (80.00%~120.00%) 8000~12000
    float32_t       VInvC_Calibrat;     //C相逆变电压校准 (80.00%~120.00%) 8000~12000
    float32_t       IGridA_Calibrat;    //A相输出电流校准 (80.00%~120.00%) 8000~12000
    float32_t       IGridB_Calibrat;    //B相输出电流校准 (80.00%~120.00%) 8000~12000
    float32_t       IGridC_Calibrat;    //C相输出电流校准 (80.00%~120.00%) 8000~12000
    float32_t       IInvA_Calibrat;     //A相逆变电流校准 (80.00%~120.00%) 8000~12000
    float32_t       IInvB_Calibrat;     //B相逆变电流校准 (80.00%~120.00%) 8000~12000
    float32_t       IInvC_Calibrat;     //C相逆变电流校准 (80.00%~120.00%) 8000~12000
    float32_t       VdcP_Calibrat;      //直流电压正校准 (80.00%~120.00%) 8000~12000
    float32_t       VdcN_Calibrat;      //直流电压负校准 (80.00%~120.00%) 8000~12000
    float32_t       Idc_Calibrat;       //直流电流校准 (80.00%~120.00%) 8000~12000
    float32_t       Vdc_Calibrat;       //直流电压校准 (80.00%~120.00%) 8000~12000
    float32_t       Ref1v5_Calibrat;    //直流电压偏置校准 (80.00%~120.00%) 8000~12000
    float32_t       IN_Calibrat;        //N线电流校准 (80.00%~120.00%) 8000~12000
    float32_t       Idc_offset;         //直流电流偏置校准 (80.00%~120.00%) 8000~12000
    float32_t       Iac_Aoffset;        //交流电流A偏置校准 (80.00%~120.00%) 8000~12000
    float32_t       Iac_Boffset;        //交流电流B偏置校准 (80.00%~120.00%) 8000~12000
    float32_t       Iac_Coffset;        //交流电流C偏置校准 (80.00%~120.00%) 8000~12000

    float32_t       Ilac_Aoffset;
    float32_t       Ilac_Boffset;
    float32_t       Ilac_Coffset;

    float32_t       IdcN_offset;

    float32_t       Pcs_VdcRms;     //直流侧电压有效值
    float32_t       Pcs_VinvMdRef;  //交流输出电压给定

    float32_t       Pcs_PacRef;     //并网有功功率给定
    float32_t       Pcs_QacRef;     //并网无功功率给定
    float32_t       Pcs_VdcRef;
    float32_t       Pcs_IdcRef;
    float32_t       Pcs_ACIoutRef;

    float32_t       Pcs_sine;
    float32_t       Pcs_cosine;

    float32_t       Pcs_Gridsine;
    float32_t       Pcs_Gridcosine;

    float32_t       Pcs_vGrid_dq0_pos_d;
    float32_t       Pcs_vGrid_dq0_pos_q;
    float32_t       Pcs_vGrid_dq0_pos_z;
    float32_t       Pcs_vGrid_dq0_neg_d;
    float32_t       Pcs_vGrid_dq0_neg_q;
    float32_t       Pcs_vGrid_dq0_neg_z;

    float32_t       RatedVoltage;     //额定电压设定
    float32_t       RatedCurrent;     //额定电流设定
    float32_t       RatedPower;       //额定功率设定
    int16_t         i16_Kp_Udc;//直流电压外环Kp
    int16_t         i16_Ki_Udc;//直流电压外环Ki

    int16_t         i16_Kp_Iinv;//逆变电流内环Kp
    int16_t         i16_Ki_Iinv;//逆变电流内环Ki

    int16_t         PcsDebugi16_0;
    int16_t         PcsDebugi16_1;
    int16_t         PcsDebugi16_2;

    float32_t       PcsDebugf32_0;
    float32_t       PcsDebugf32_1;
    float32_t       PcsDebugf32_2;

}T_CPU2CLA_VARIABLE;

typedef struct
{
    //相电压
    float32_t       Pcs_vGrid_AB_sensed_pu;
    float32_t       Pcs_vGrid_BC_sensed_pu;
    float32_t       Pcs_vGrid_CA_sensed_pu;
    //相电压
    float32_t       Pcs_vGrid_A_sensed_pu;
    float32_t       Pcs_vGrid_B_sensed_pu;
    float32_t       Pcs_vGrid_C_sensed_pu;
    //线电压
    float32_t       Pcs_vInv_A_sensed_pu;
    float32_t       Pcs_vInv_B_sensed_pu;
    float32_t       Pcs_vInv_C_sensed_pu;

    float32_t       Pcs_iGrid_A_sensed_pu;
    float32_t       Pcs_iGrid_B_sensed_pu;
    float32_t       Pcs_iGrid_C_sensed_pu;

    float32_t       Pcs_iInv_A_sensed_pu;
    float32_t       Pcs_iInv_B_sensed_pu;
    float32_t       Pcs_iInv_C_sensed_pu;

    float32_t       Pcs_vBusP_sensed_pu;
    float32_t       Pcs_vBusN_sensed_pu;
    float32_t       Pcs_vBus_sensed_pu;
    float32_t       Pcs_iBus_sensed_pu;
    float32_t       Pcs_vBatt_sensed_pu;
    float32_t       Pcs_iBusN_sensed_pu;

    float32_t       Pcs_iInv_Iod;
    float32_t       Pcs_iInv_Ioq;
    float32_t       Pcs_vGrid_Vod;
    float32_t       Pcs_vGrid_Voq;

    float32_t       Pcs_IoutMd;
    float32_t       Pcs_IoutMq;


    float32_t       Pcs_vdRatio_pu;

    int16_t         PcsDebugi16_0;
    int16_t         PcsDebugi16_1;
    int32_t         PcsDebugi16_2;

    float32_t       PcsDebugf32_0;
    float32_t       PcsDebugf32_1;
    float32_t       PcsDebugf32_2;

}T_CLA2CPU_VARIABLE;


typedef struct
{
    //相电压
    float32_t       Pcs_vGrid_AB_sensed_pu;
    float32_t       Pcs_vGrid_BC_sensed_pu;
    float32_t       Pcs_vGrid_CA_sensed_pu;
    //相电压
    float32_t       Pcs_vGrid_A_sensed_pu;
    float32_t       Pcs_vGrid_B_sensed_pu;
    float32_t       Pcs_vGrid_C_sensed_pu;

    float32_t       Pcs_vInv_A_sensed_pu;
    float32_t       Pcs_vInv_B_sensed_pu;
    float32_t       Pcs_vInv_C_sensed_pu;

    float32_t       Pcs_iGrid_A_sensed_pu;
    float32_t       Pcs_iGrid_B_sensed_pu;
    float32_t       Pcs_iGrid_C_sensed_pu;

    float32_t       Pcs_iInv_A_sensed_pu;
    float32_t       Pcs_iInv_B_sensed_pu;
    float32_t       Pcs_iInv_C_sensed_pu;

    float32_t       Pcs_vBatt_sensed_pu;//DC端口电压
    float32_t       Pcs_vBusP_sensed_pu;
    float32_t       Pcs_vBusN_sensed_pu;

    float32_t       Pcs_iBus_sensed_pu;
    float32_t       Pcs_iBusN_sensed_pu;

    float32_t       Pcs_vBus_DC;
    float32_t       Pcs_iBus_DC;

    float32_t       Pcs_activePower;        //有功功率实时值
    float32_t       Pcs_reactivePower;      //无功功率实时值
    float32_t       Pcs_activePowerFilt;    //有功功率滤波值
    float32_t       Pcs_reactivePowerFilt;



    float32_t       Pcs_CosRef;
    float32_t       Pcs_SinRef;

    float32_t       Pcs_VinvAlpha;
    float32_t       Pcs_VinvBeta;
    //交流电压正序DQZ
    float32_t       Pcs_VinvMd;
    float32_t       Pcs_VinvMq;
    float32_t       Pcs_VinvMz;
    //电感电流DQZ
    float32_t       Pcs_IlMd;
    float32_t       Pcs_IlMq;
    float32_t       Pcs_IlMz;
    //交流电压负序DQZ
    float32_t       Pcs_VinvMd_N;
    float32_t       Pcs_VinvMq_N;
    float32_t       Pcs_VinvMz_N;
    //交流电流正序DQZ
    float32_t       Pcs_IoutMd;
    float32_t       Pcs_IoutMq;
    float32_t       Pcs_IoutMz;
    //离网电压环给定
    float32_t       Pcs_VinvMdRef;
//    float32_t       Pcs_VinvMqRef;
//    float32_t       Pcs_VinvMzRef;

    float32_t       Pcs_VdcRef;
    float32_t       Pcs_IdcRef;
    //离网电流环前馈给定
    float32_t       Pcs_VinvMdRef_1;
    float32_t       Pcs_VinvMqRef_1;
    float32_t       Pcs_VinvMzRef_1;

    float32_t       Pcs_PacRef;     //并网有功功率给定
    float32_t       Pcs_QacRef;     //并网无功功率给定

    float32_t       Pcs_ACIoutRef;

    float32_t       Pcs_ErrorVinvMd;
    float32_t       Pcs_ErrorVinvMq;
    float32_t       Pcs_ErrorVinvMz;
    //离网PI调节参数
    float32_t       Pcs_VinvDRegIOut;//D轴调节输出 P参数
    float32_t       Pcs_VinvDRegOut;

    float32_t       Pcs_VinvQRegIOut;//Q轴调节输出
    float32_t       Pcs_VinvQRegOut;

    float32_t       Pcs_VinvZRegIOut;//Z轴调节输出
    float32_t       Pcs_VinvZRegOut;

    float32_t       Pcs_ErrorVinvMd_N;
    float32_t       Pcs_ErrorVinvMq_N;

    float32_t       Pcs_VinvDRegIOut_N;
    float32_t       Pcs_VinvDRegOut_N;
    float32_t       Pcs_VinvQRegIOut_N;
    float32_t       Pcs_VinvQRegOut_N;

    float32_t       Pcs_IlDRegIOut;     // 电流环D轴I输出
    float32_t       Pcs_IlDRegOut;      // 电流环D轴输出

    float32_t       Pcs_IlQRegIOut;     // 电流环Q轴I输出
    float32_t       Pcs_IlQRegOut;      // 电流环Q轴输出

    float32_t       Pcs_IlZRegIOut;     // 电流环Z轴I输出
    float32_t       Pcs_IlZRegOut;      // 电流环Z轴输出

    //正序电压环输出DQ
    float32_t       Pcs_IinvMdRef_P;
    float32_t       Pcs_IinvMqRef_P;
    //负序电压环输出DQ
    float32_t       Pcs_IinvMdRef_N;
    float32_t       Pcs_IinvMqRef_N;
    //正负序电压环合并输出DQZ，电流环输入给定
    float32_t       Pcs_IinvMdRef;
    float32_t       Pcs_IinvMqRef;
    float32_t       Pcs_IinvMzRef;

    float32_t       Pcs_IinvMdRef_1;
    float32_t       Pcs_IinvMqRef_1;
    float32_t       Pcs_IinvMzRef_1;

    float32_t       Pcs_ErrorIlMd;
    float32_t       Pcs_ErrorIlMq;
    float32_t       Pcs_ErrorIlMz;
    float32_t       Pcs_ErrorIlMd_old;
    float32_t       Pcs_ErrorIlMq_old;
    //电流环输出给定PWM输出
    float32_t       Pcs_UirD;
    float32_t       Pcs_UirQ;
    float32_t       Pcs_UirZ;

    //离网电压外环PI参数
    float32_t       Pcs_VinvKp;
    float32_t       Pcs_VinvKi;

    float32_t       Pcs_IlKp;
    float32_t       Pcs_IlKi;

    float32_t       Pcs_Kp_Udc;//直流电压外环Kp
    float32_t       Pcs_Ki_Udc;//直流电压外环Ki

    float32_t       Pcs_Kp_Idc;//直流电流外环Kp
    float32_t       Pcs_Ki_Idc;//直流电流外环Ki

    float32_t       Pcs_VBatt_pu;
    float32_t       Pcs_VBus_pu;
    float32_t       Pcs_VBusP_pu;
    float32_t       Pcs_VBusN_pu;
    float32_t       Pcs_IBatt_pu;

    float32_t       RatedVoltage;     //额定电压设定
    float32_t       RatedCurrent;     //额定电流设定
    float32_t       RatedPower;       //额定功率设定

    float32_t       MaxCurrent;     //最大工作电流
    float32_t       MaxPower;       //最大工作功率
    float32_t       MaxVoltage;       //最大工作电压

    float32_t       lpFilter_Yn_l;
    float32_t       lpFilter_Yn_V;

    float32_t       lpFilter_Yn_BPV;
    float32_t       lpFilter_Yn_BNV;
    int16_t         Pcs_VDC_cnt;

    int16_t         Pwm_Enable;
    int16_t         PcsDebugi16_0;
    int16_t         PcsDebugi16_1;
    int32_t         PcsDebugi16_2;
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
