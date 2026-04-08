/*
 * bsp.h
 *
 *  Created on: 2024年9月27日
 *      Author: guowei680
 */

#ifndef _BSP_H_
#define _BSP_H_

#include <stdio.h>

#include "ModbusTcp.h"
#include "ModbusRTU.h"
#include "Modbus_Param.h"
#include "ipc_cm.h"
#include "Drv_LED.h"
#include "Drv_Timer.h"
#include "Drv_I2CA.h"
#include "Drv_I2C_EE.h"
#include "Drv_I2C_RTC.h"
#include "w5500.h"
#include "w5500_conf.h"
#include "obj_debug.h"

#define   float32_t    float

#define  EE_PARAM_CNT       58 //57

#define  EE_ADDR_IP1        192
#define  EE_ADDR_IP2        196
//充电数据存储地址
#define  EE_ADDR_CHARGE     256

enum
{
    GRID_DISCONNECTED = 0,
    GRID_CONNECTED  = 1
};


typedef struct{
    uint16_t    ModAddr;
    int16_t    min;
    int16_t    max;
    int16_t    *g_param;
    uint16_t    eeAddr;
}Store_PARAM;


//#pragma pack(1) //设置内存对齐为：1字节对齐
typedef struct _Stru_Fault
{
    union
     {
         uint16_t PCS_Waring1_All;
         struct
         {
             uint16_t  bOverLoad               :1;//过载警告
             uint16_t  bOverTempReduLimit      :1;//过温降额警告
             uint16_t  bUncontrRectification   :1;//不控整流
             uint16_t  bRsvd                   :13;
         }tbits;
     }PCS_Waring1;

     union
     {
         uint16_t PCS_Waring2_All;
         struct
         {
             uint16_t  bRsvd                   :16;
         }tbits;
     }PCS_Waring2;

     union
     {
         uint16_t PCS_Fault1_All;//AC故障
         struct
         {
             uint16_t  bACVOverFault           :1;//交流过压故障
             uint16_t  bACVUnderFault          :1;//交流欠压故障
             uint16_t  bACVBanlanceFault       :1;//电网电压不平衡故障
             uint16_t  bACVPhaseSequFault      :1;//电网相序故障
             uint16_t  bACVFreqFault           :1;//电网频率故障
             uint16_t  bNLostFault             :1;//缺N线故障
             uint16_t  bPELostFault            :1;//缺PE故障
             uint16_t  bIINVOverFault          :1;//逆变过流故障
             uint16_t  bOverLoadTimeOutFault   :1;//过载超时故障
             uint16_t  bVINVFault              :1;//逆变电压故障
             uint16_t  bVINVUnBanaceFault      :1;//逆变电压不平衡
             uint16_t  bVINVFreqFault          :1;//逆变频率故障
             uint16_t  bINVIsLandFault         :1;//逆变器孤岛故障
             uint16_t  bINVShortFault          :1;//逆变器短路故障
             uint16_t  bINVSoftRelyFault       :1;//逆变器软起故障
             uint16_t  bGridACVFault           :1;//电网电压故障
         }tbits;
     }PCS_Fault1;

     union
     {
         uint16_t PCS_Fault2_All;//DC故障
         struct
         {
             uint16_t  bBUSVOverFault          :1;//母线过压故障
             uint16_t  bBUSVUnderFault         :1;//母线欠压故障
             uint16_t  bBUSVUnBanlanceFault    :1;//母线电压不平衡故障
             uint16_t  bBUSIOverFault          :1;//母线过流故障
             uint16_t  bBUSReversedFault       :1;//母线反接故障
             uint16_t  bBUSSoftRelyTOutFault   :1;//母线软起超时故障
             uint16_t  bBUSRelyOPenFault       :1;//直流继电器开路故障
             uint16_t  bBUSRelyShortFault      :1;//直流继电器短路故障
             uint16_t  bDCIMFault              :1;//直流绝缘电阻故障
             uint16_t  bRsvd                   :7;
         }tbits;
     }PCS_Fault2;

     union
     {
         uint16_t PCS_Fault3_All;
         struct
         {
             uint16_t  bAmbTemptOverFault       :1;//环境过温故障
             uint16_t  bIgbtTemptOverFault      :1;//IGBT过温故障
             uint16_t  bAuxPowerFault           :1;//辅源故障
             uint16_t  bFanFault                :1;//风扇故障
             uint16_t  bEEpromFault             :1;//EE存储器故障
             uint16_t  bRs485_1Fault            :1;//RS485模块1故障
             uint16_t  bRs485_2Fault            :1;//RS485模块2故障
             uint16_t  bCAN_1Fault              :1;//CAN1故障
             uint16_t  bCAN_2Fault              :1;//CAN2故障
             uint16_t  bEtherNETFault           :1;//以太网故障
             uint16_t  bRsvd                    :6;
         }tbits;
     }PCS_Fault3;

     union
     {
         uint16_t PCS_Fault4_All;
         struct
         {
             uint16_t  bRsvd                    :16;
         }tbits;
     }PCS_Fault4;

     union
     {
         uint16_t PCS_HFault_All;
         struct
         {
             uint16_t  bDCIOver_Charge           :1;//直流充电过流
             uint16_t  bDCIOver_DisCharge        :1;//直流放电过流
             uint16_t  bACAIOver_Charge          :1;//A相充电过流
             uint16_t  bACAIOver_DisCharge       :1;//A相放电过流
             uint16_t  bACBIOver_Charge          :1;//B相充电过流
             uint16_t  bACBIOver_DisCharge       :1;//B相放电过流
             uint16_t  bACCIOver_Charge          :1;//C相充电过流
             uint16_t  bACCIOver_DisCharge       :1;//C相放电过流
             uint16_t  bACHIOver_Charge          :1;//AC硬件充电过流
             uint16_t  bACHIOver_DisCharge       :1;//AC硬件放电过流
             uint16_t  bDCHIOver_Charge          :1;//DC硬件充电过流
             uint16_t  bDCHIOver_DisCharge       :1;//DC硬件放电过流
             uint16_t  bEmergencyStop            :1;//急停输入
         }tbits;
     }PCS_HFault;

    int16_t  globalFault;
}Stru_Fault;
//#pragma pack()

typedef struct _IPC_DATA_CPU2CM //
{
    int16_t   PCS_AC_VRms_PhaseA;        //A相电压，0.1V
    int16_t   PCS_AC_VRms_PhaseB;        //B相电压，0.1V
    int16_t   PCS_AC_VRms_PhaseC;        //C相电压，0.1V

    int16_t   PCS_AC_VRms_LineAB;        //AB线电压，0.1V
    int16_t   PCS_AC_VRms_LineBC;        //BC线电压，0.1V
    int16_t   PCS_AC_VRms_LineCA;        //CA线电压，0.1V

    int16_t   PCS_AC_VLRms_PhaseA;       //A相电压，0.1V
    int16_t   PCS_AC_VLRms_PhaseB;       //B相电压，0.1V
    int16_t   PCS_AC_VLRms_PhaseC;       //C相电压，0.1V

    int16_t   PCS_AC_IRms_A;             //A相电流，0.1A
    int16_t   PCS_AC_IRms_B;             //B相电流，0.1A
    int16_t   PCS_AC_IRms_C;             //C相电流，0.1A

    int16_t   PCS_AC_ILRms_A;             //A1相电流，0.1A
    int16_t   PCS_AC_ILRms_B;             //B1相电流，0.1A
    int16_t   PCS_AC_ILRms_C;             //C1相电流，0.1A

    int16_t   PCS_AC_PowerP_A;            //A相有功功率，0.01KW
    int16_t   PCS_AC_PowerP_B;            //A相有功功率，0.01KW
    int16_t   PCS_AC_PowerP_C;            //A相有功功率，0.01KW

    int16_t   PCS_AC_PowerQ_A;            //A相无功功率，0.01KVar
    int16_t   PCS_AC_PowerQ_B;            //B相无功功率，0.01KVar
    int16_t   PCS_AC_PowerQ_C;            //C相无功功率，0.01KVar

    int16_t   PCS_AC_PowerS_A;            //A相视在功率，0.01KVA
    int16_t   PCS_AC_PowerS_B;            //B相视在功率，0.01KVA
    int16_t   PCS_AC_PowerS_C;            //C相视在功率，0.01KVA

    int16_t   PCS_AC_PowerP;              //三相总有功功率，0.01KW
    int16_t   PCS_AC_PowerQ;              //三相总无功功率，0.01KVar
    int16_t   PCS_AC_PowerS;              //三相总视在功率，0.01KVA

    int16_t   PCS_AC_Freq_A;              //A相频率，0.01HZ
    int16_t   PCS_AC_Freq_B;              //B相频率，0.01HZ
    int16_t   PCS_AC_Freq_C;              //C相频率，0.01HZ

    int16_t   PCS_AC_PF_A;                //A相功率因数， 0.01
    int16_t   PCS_AC_PF_B;                //B相功率因数， 0.01
    int16_t   PCS_AC_PF_C;                //C相功率因数， 0.01

    int16_t   PCS_DC_BusVol;              //母线侧直流电压
    int16_t   PCS_DC_BusCurr;             //母线侧直流电流，0.01A
    int16_t   PCS_DC_Power;               //母线侧直流功率，0.01KW

    int16_t   PCS_DC_PBusVol;             //正母线电压，0.1V
    int16_t   PCS_DC_NBusVol;             //负母线电压，0.1V
    int16_t   PCS_DC_IBUS_N;              //N线电流，0.1A

    int16_t   Temp_igbtMax;               //IGBT温度最大值
    int16_t   Temp_igbtA;                 //IGBT温度 A相
    int16_t   Temp_igbtB;                 //IGBT温度 B相
    int16_t   Temp_igbtC;                 //IGBT温度 C相
    int16_t   Temp_igbtN;                 //IGBT温度 N相
    int16_t   Temp_AmbInlet;              //进风口环境温度
    int16_t   Temp_AmbOutlet;             //出风口环境温度

    int16_t   Vol_1V5;                    //+1.5V  0.001
    int16_t   Vol_5V;                     //+5V  0.001
    int16_t   Vol_p15V;                   //+15V  0.001

    int16_t   Pcs_Localpllfo;             //离网自生成信号锁相环频率
    int16_t   Pcs_Gridpllfo;              //电网信号锁相环频率

    int16_t   PwmRatio;                   //PWM1控制比例(100.00%)
    int16_t   DerateRatio;                //过温降额运行百分比
    int16_t   pllPhaseErr;                //并离网锁相相位差

    int16_t   Relay_DCSoft;               //直流软起继电器控制状态
    int16_t   Relay_DCMaster;             //直流主继电器控制状态
    int16_t   Relay_ACSoft;               //交流软起继电器控制状态
    int16_t   Relay_ACMaster;             //交流主继电器控制状态
    int16_t   Relay_FanCtrl;              //风扇控制
    int16_t   Relay_WorkLed;              //工作指示灯
    int16_t   Relay_FaultLed;             //故障指示灯
    int16_t   FanCtrl_duty;               //风扇控制

    int16_t   PcsCtrlState;               //控制状态机状态
    int16_t   PcsOnAllowed;               //允许开机 0:不允许开机，1:允许开机
    int16_t   uniqueID_M;                 //CPU_UID 高16位
    int16_t   uniqueID_L;                 //CPU_UID 低16位

    int16_t   Pwm_StartEn;                //cla中的发波使能标志

   Stru_Fault  FaultStatus;
}IPC_DATA_CPU2CM;

typedef struct _IPC_DATA_CM2CPU //
{
    //控制模式设定
    int16_t   StartEn;                //启动/停止
    int16_t   WorkMode;               //工作模式，0:离网,1:并网
    int16_t   OnGridMode;             //并网工作模式(0:恒功率,1:恒流,2:恒直流电压,3:恒直流电流)
    int16_t   LocalRemote;            //0:本地,1:远程
    int16_t   Pwm_StartEn;            //Pwm发波使能
    int16_t   OutLoop_En;             //电流外环使能    0：关闭外环，1：打开外环
    int16_t   SoftStart_En;           //电流软启动使能   0：关闭软启动，1：打开软起动
    int16_t   debugMode;              //0:正常模式，1:调试模式
    int16_t   OpenLoopMode;           //0:开环模式，1:闭环模式
    int16_t   pcsLineMode;            //0:3P3L，1:3P4L
    int16_t   ClearFault;             //故障清除，1:清除
    int16_t   debugData_TxEn;         //调试数据发送使能
    int16_t   Reset_En;               //复位使能 关闭看门狗
    //控制参数设定
    int16_t   ActivePowerSet;         //有功功率设定
    int16_t   ReActivePowerSet;       //无功功率设定
    int16_t   DcConstCurrSet;         //恒直流电流设定
    int16_t   DcConstVolSet;          //恒直流电压设定
    int16_t   AcConstCurrSet;         //恒交流电流设定
    int16_t   PowerFactorSet;         //功率因数设定
    int16_t   VF_VoSet;               //VF输出电压设置
    int16_t   VF_FreqSet;             //VF输出频率设置
    int16_t   DcEqualChargeVol;       //DC均充电压设置
    int16_t   DcFloatChargeVol;       //DC浮充电压设置
    int16_t   RatedVoltage;             //额定电压设定
    int16_t   RatedCurrent;             //额定电流设定
    int16_t   RatedPower;               //额定功率设定
    int16_t   DcEqual2FloatCurrent;   //DC均充转浮充电流设置
    int16_t   BattHighVolSet;         //电池电压保护上限设置
    int16_t   BattLowVolSet;          //电池电压保护下限设置
    int16_t   BattEODVol;             //EOD终止放电电压设置
    int16_t   DODGridConnect;         //并网DOD设置 放电深度
    int16_t   DODDisGridConnect;      //离网DOD设置 放电深度
    //AD校准参数
    int16_t   VgridA_Calibrat;    //A相电网电压校准 (80.00%~120.00%) 8000~12000
    int16_t   VgridB_Calibrat;    //B相电网电压校准 (80.00%~120.00%) 8000~12000
    int16_t   VgridC_Calibrat;    //C相电网电压校准 (80.00%~120.00%) 8000~12000
    int16_t   VInvA_Calibrat;     //A相逆变电压校准 (80.00%~120.00%) 8000~12000
    int16_t   VInvB_Calibrat;     //B相逆变电压校准 (80.00%~120.00%) 8000~12000
    int16_t   VInvC_Calibrat;     //C相逆变电压校准 (80.00%~120.00%) 8000~12000
    int16_t   IGridA_Calibrat;    //A相输出电流校准 (80.00%~120.00%) 8000~12000
    int16_t   IGridB_Calibrat;    //B相输出电流校准 (80.00%~120.00%) 8000~12000
    int16_t   IGridC_Calibrat;    //C相输出电流校准 (80.00%~120.00%) 8000~12000
    int16_t   IInvA_Calibrat;     //A相逆变电流校准 (80.00%~120.00%) 8000~12000
    int16_t   IInvB_Calibrat;     //B相逆变电流校准 (80.00%~120.00%) 8000~12000
    int16_t   IInvC_Calibrat;     //C相逆变电流校准 (80.00%~120.00%) 8000~12000
    int16_t   VdcP_Calibrat;      //直流电压正校准 (80.00%~120.00%) 8000~12000
    int16_t   VdcN_Calibrat;      //直流电压负校准 (80.00%~120.00%) 8000~12000
    int16_t   Idc_Calibrat;       //直流电流校准 (80.00%~120.00%) 8000~12000
    int16_t   Vdc_Calibrat;       //直流电压校准 (80.00%~120.00%) 8000~12000
    int16_t   Ref1v5_Calibrat;      //直流电压偏置校准 (80.00%~120.00%) 8000~12000
    int16_t   IN_Calibrat;        //N线电流校准 (80.00%~120.00%) 8000~12000
    int16_t   Idc_offset;
    int16_t   Iac_Aoffset;
    int16_t   Iac_Boffset;
    int16_t   Iac_Coffset;
    int16_t   Ilac_Aoffset;
    int16_t   Ilac_Boffset;
    int16_t   Ilac_Coffset;
    int16_t   IdcN_offset;
    int16_t   Reserved_Calibrat;
    //PI控制参数
    int16_t   IlInter_Kp;               //电感电流内环Kp参数
    int16_t   IlInter_Ki;               //电感电流内环Ki参数
    int16_t   IAcOuter_Kp;              //AC电流外环Kp参数
    int16_t   IAcOuter_Ki;              //AC电流外环Ki参数
    int16_t   IDcOuter_Kp;              //DC电流外环Kp参数
    int16_t   IDcOuter_Ki;              //DC电流外环Ki参数
    int16_t   VAcOuter_Kp;              //AC电压外环Kp参数
    int16_t   VAcOuter_Ki;              //AC电压外环Ki参数
    int16_t   VDcOuter_Kp;              //DC电压外环Kp参数
    int16_t   VDcOuter_Ki;              //DC电压外环Ki参数
    //故障保护阈值
    int16_t   AC_OverVol_Value;        //交流过压保护阈值
    int16_t   AC_UnderVol_Value;       //交流欠压保护阈值
    int16_t   AC_OverCurr_Value;       //交流过流保护阈值
    int16_t   AC_UnbanceVol_Value;     //交流电压不平衡报护阈值

    int16_t   DC_OverVol_Value;        //直流过压保护阈值
    int16_t   DC_OverCurr_Value;       //直流过流保护阈值
    int16_t   DC_UnderVol_Value;       //直流欠压保护阈值
    int16_t   DC_UnbanceVol_Value;     //直流电压不平衡报护阈值
    int16_t   TempAmb_Over_Value;      //环境过温保护阈值
    int16_t   TempIgbt_Over_Value;     //IGBT过温保护阈值
    //手动控制时，由CM传给CPU1
    int16_t   FanCtrl_duty;           //风扇控制
    int16_t   RelayCtrl_DCSoft;       //直流软起继电器控制状态
    int16_t   RelayCtrl_DCMaster;     //直流主继电器控制状态
    int16_t   RelayCtrl_ACSoft;       //交流软起继电器控制状态
    int16_t   RelayCtrl_ACMaster;     //交流主继电器控制状态
    int16_t   RelayCtrl_FanCtrl;      //风扇控制
    int16_t   RelayCtrl_WorkLed;      //工作指示灯
    int16_t   RelayCtrl_FaultLed;     //故障指示灯
    //CM上电自检状态
    int16_t   SelfCheckOK;            //CM上电自检状态
    int16_t   IpcCpu2Cm_Fault;        //CPU到CM Ipc通信故障
    int16_t   Rs485_1_Fault;          //RS485_1通信故障
    int16_t   Rs485_2_Fault;          //RS485_2通信故障
    int16_t   CAN_1_Fault;            //CAN_1通信故障
    int16_t   CAN_2_Fault;            //CAN_2通信故障
    int16_t   EE_Fault;               //EEPROM故障
    int16_t   Sflash_Fault;           //spi flash通信故障
    uint16_t  PtpSynced;              // PTP同步完成标志，由CM核设置
}IPC_DATA_CM2CPU;


typedef struct _LOCAL_PARAM_CM //
{
    uint16_t    PCS_MChargeKWH;        //充电电量，万度(KWH)
    uint16_t    PCS_LChargeKWH;        //充电电量，度(KWH)
    uint16_t    PCS_MdisChargeKWH;     //放电电量，万度(KWH)
    uint16_t    PCS_LdisChargeKWH;     //放电电量，度(KWH)

    uint16_t    PCS_RUN_TimeM;         //累计运行时间，分钟(M)
    int16_t     EEparamCheckOK;        //EE参数初始化正确

    uint8_t     pIpAddr1[4];
    uint8_t     pIpAddr2[4];
}LOCAL_PARAM_CM;


extern IPC_DATA_CPU2CM         CmIpc_cpu2cm;
extern IPC_DATA_CM2CPU         CmIpc_cm2cpu;
extern LOCAL_PARAM_CM          CmLocalParam;

extern Store_PARAM              store_param[EE_PARAM_CNT];

extern uint8_t pIpAddr2[4];

void bsp_init(void);
void Pcs_CmRunISR(void);
void Cm_Fault_Reset(void);
void Output_KWHDeal(void);
void Output_RunTimeCnt(void);
uint16_t BEBufToUint16(uint16_t _pBufM,uint16_t _pBufL);

#endif /* _BSP_H_ */
