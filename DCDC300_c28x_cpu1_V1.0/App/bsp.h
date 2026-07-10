/*
 * bsp.h
 *
 *  Created on: 2024年9月27日
 *      Author: guowei680
 */

#ifndef _BSP_H_
#define _BSP_H_

/* CODEMAP_BSP_H
 * Global project declarations.
 * Most modules communicate through DcDc_gParam, DC_OutMeter, FaultStatus and tCpu2Cla/tCla2Cpu.
 * Read DCDC_Global_Param first to understand commands, relay states, modes and PI parameters.
 */
#include "Drv_SCIA.h"
#include "Drv_SCIB.h"
#include "Drv_SCIC.h"
#include "Drv_SCID.h"
#include "Drv_LED.h"
#include "Drv_Timer.h"
#include "Drv_ADC.h"
#include "Drv_PWM.h"
#include "Drv_I2CA.h"
//#include "Drv_SPI.h"
#include "Drv_tca9535_IO1.h"
#include "Drv_tca9535_IO2.h"
#include "Drv_DO.h"
#include "Drv_DIN.h"
//#include "Drv_WatchDog.h"
#include "initial_CLA.h"
#include "Drv_EtherNet.h"

#include "Drv_I2C_EE.h"
#include "Drv_I2C_RTC.h"
#include "Modbus_RTU.h"
#include "obj_ipc.h"

#include "obj_OutMeter.h"
#include "obj_Control.h"
#include "obj_Protect.h"

#include "board.h"
#include "clb_config.h"
#include "clb.h"

#include "Pcs_setting.h"
#include "aes_128.h"
//#include "C28x_FPU_FastRTS.h"

#define  EE_PARAM_CNT       57

//充电数据存储地址
#define  EE_ADDR_CHARGE     300
//密钥存储地址
#define  EE_ADDR_KEY        1000

typedef struct
{
    float32_t buf[200];
    uint16_t i;
}GRAPH;

//DCDC全局参数
/* CODEMAP_GLOBAL_PARAM
 * CPU-side command/config/status structure.
 * Modbus writes many of these fields; cpu2claParam_Upgrade() copies the needed ones to CLA.
 */typedef struct _DCDC_Global_Param
{
  int16_t   StartEn;                //启动/停止
  int16_t   WorkMode;               //工作模式，0:buck,1:boost
  int16_t   CtrlMode;               //控制模式(//0:开环,1:恒压,2:恒流,3:恒功率,4:恒阻)
  int16_t   LocalRemote;            //0:本地,1:远程
  int16_t   DcConstVolSet;          //恒直流电压设定
  int16_t   DcConstCurrSet;         //恒直流电流设定
  int16_t   DcConstPowerSet;        //恒功率功率设定
  int16_t   DcConstResSet;          //恒电阻阻值设定
  int16_t   DcBusMaxVol;            //低压侧最大输出电压
  int16_t   DcBatMaxVol;            //高压侧最大输出电压
  int16_t   DcBusMaxCurrent;        //低压侧最大输出电流
  int16_t   DcBatMaxCurrent;        //高压侧最大输出电流
  int16_t   BattHighVolSet;         //电池电压保护上限设置
  int16_t   BattLowVolSet;          //电池电压保护下限设置
  int16_t   BattEODVol;             //EOD终止放电电压设置
  int16_t   DODGridConnect;         //并网DOD设置 放电深度
  int16_t   DODDisGridConnect;      //离网DOD设置 放电深度
  int16_t   RelayCtrl;              //继电器控制
  int16_t   SelfCheckOK;            //上电自检状态
  int16_t   DcCtrlState;            //PCS控制状态机状态
  int16_t   DcOnAllowed;            //无故障允许开机
  int16_t   RelaySt_DcBusSoft;      //直流软起继电器控制状态
  int16_t   RelaySt_DcBusMaster;    //直流主继电器控制状态
  int16_t   RelaySt_DcBatSoft;      //交流软起继电器控制状态
  int16_t   RelaySt_DcBatMaster;    //交流主继电器控制状态
  int16_t   EEparamCheckOK;         //EE参数初始化正确
  int16_t   Pwm_StartEn;            //Pwm允许发波
  int16_t   RelaySt_FanCtrl;        //风扇控制
  int16_t   RelaySt_WorkLed;        //工作指示灯
  int16_t   RelaySt_FaultLed;       //故障指示灯
  int16_t   OutLoop_En;             //电流外环使能    0：关闭外环，1：打开外环
  int16_t   SoftStart_En;           //电流软启动使能   0：关闭软启动，1：打开软起动
  int16_t   debugMode;              //0:正常模式，1:调试模式
  int16_t   PwmRatio1;              //PWM1控制比例
  int16_t   PwmRatio2;              //PWM2控制比例
  int16_t   PwmRatio3;              //PWM3控制比例
  int16_t   VBuck_Kp;               //Buck电压外环Kp参数
  int16_t   VBuck_Ki;               //Buck电压外环Ki参数
  int16_t   IBuck_Kp;               //Buck电流外环Kp参数
  int16_t   IBuck_Ki;               //Buck电流外环Ki参数
  int16_t   IdcBuck_Kp;             //Buck电流内环Kp参数
  int16_t   IdcBuck_Ki;             //Buck电流内环Ki参数
  int16_t   VBoost_Kp;              //Boost电压外环Kp参数
  int16_t   VBoost_Ki;              //Boost电压外环Ki参数
  int16_t   IBoost_Kp;              //Boost电流外环Kp参数
  int16_t   IBoost_Ki;              //Boost电流外环Ki参数
  int16_t   IdcBoost_Kp;            //Boost电流内环Kp参数
  int16_t   IdcBoost_Ki;            //Boost电流内环Ki参数
  int16_t   MPPT_En;                //MPPT使能
  int16_t   MPPT_VdcMax;            //MPPT最大电压
  int16_t   MPPT_VdcMin;            //MPPT最小电压
  int16_t   Version;                //主版本
  int16_t   Reversion;              //次版本
  int16_t   Patch;                  //补丁
  int16_t   Build;                  //编译
}DCDC_Global_Param;

//校准参数
/* CODEMAP_CAL_PARAM
 * Calibration and offset parameters used by CLA sampling conversion.
 * Registers 307..327 mainly map to these fields.
 */typedef struct _TINV_CALIBRAT_PARAM
{
  int16_t   VBus_Calibrat;    //低压侧电压校准 (80.00%~120.00%) 8000~12000
  int16_t   VBusS_Calibrat;   //低压软起电压校准 (80.00%~120.00%) 8000~12000
  int16_t   VBat_Calibrat;    //高压侧电压校准 (80.00%~120.00%) 8000~12000
  int16_t   VBatS_Calibrat;   //高压软起电压校准 (80.00%~120.00%) 8000~12000
  int16_t   VBatP_Calibrat;   //高压正电压校准 (80.00%~120.00%) 8000~12000
  int16_t   VBatN_Calibrat;   //高压负电压校准 (80.00%~120.00%) 8000~12000
  int16_t   IBus_Calibrat;    //低压侧电流校准 (80.00%~120.00%) 8000~12000
  int16_t   IBat_Calibrat;    //高压侧电流校准 (80.00%~120.00%) 8000~12000
  int16_t   IL1_Calibrat;     //单元1电流校准 (80.00%~120.00%) 8000~12000
  int16_t   IL2_Calibrat;     //单元2电流校准 (80.00%~120.00%) 8000~12000
  int16_t   IL3_Calibrat;     //单元3电流校准 (80.00%~120.00%) 8000~12000
  int16_t   VBat_offset;      //高压电压偏置校准 (80.00%~120.00%) 8000~12000
  int16_t   VBatS_offset;     //高压软起电压偏置校准 (80.00%~120.00%) 8000~12000
  int16_t   VBus_offset;      //低压电压偏置校准 (80.00%~120.00%) 8000~12000
  int16_t   VBusS_offset;     //低压软起电压偏置校准 (80.00%~120.00%) 8000~12000
  int16_t   IBus_offset;      //低压电流偏置校准 (80.00%~120.00%) 8000~12000
  int16_t   IBat_offset;      //高压电流偏置校准 (80.00%~120.00%) 8000~12000
  int16_t   IL1_offset;       //单元1电流偏置校准 (80.00%~120.00%) 8000~12000
  int16_t   IL2_offset;       //单元2电流偏置校准 (80.00%~120.00%) 8000~12000
  int16_t   IL3_offset;       //单元3电流偏置校准 (80.00%~120.00%) 8000~12000
  int16_t   Idc_Calibrat;
  int16_t   Vdc_Calibrat;
  int16_t   Ref1v5_Calibrat;      //1.5V电压偏置校准 (80.00%~120.00%) 8000~12000
  int16_t   IN_Calibrat;
  int16_t   Idc_offset;
  int16_t   Iac_Aoffset;
  int16_t   Iac_Boffset;
  int16_t   Iac_Coffset;
  int16_t   Ilac_Aoffset;
  int16_t   Ilac_Boffset;
  int16_t   Ilac_Coffset;
  int16_t   IdcN_offset;
}TINV_CALIBRAT_PARAM;

typedef struct{
    uint16_t    ModAddr;
    int16_t    min;
    int16_t    max;
    int16_t    *g_param;
    uint16_t    eeAddr;
}Store_PARAM;


//#pragma pack(1) //设置内存对齐为：1字节对齐
/* CODEMAP_FAULT_BITS
 * Fault bit layout exported through Modbus fault registers.
 * App_AllFault_Deal() sets these bits and then blocks startup through globalFault/DcOnAllowed.
 */typedef struct _St_DCDC_Fault
{
    union
    {
        uint16_t DCDC_Waring1_All;
        struct
        {
            uint16_t  bOverLoad               :1;//过载警告
            uint16_t  bOverTempReduLimit      :1;//过温降额警告
            uint16_t  bUncontrRectification   :1;//不控整流
            uint16_t  bRsvd                   :13;
        }tbits;
    }DCDC_Waring1;

    union
    {
        uint16_t DCDC_Waring2_All;
        struct
        {
            uint16_t  bRsvd                   :16;
        }tbits;
    }DCDC_Waring2;

        union
        {
            uint16_t DCDC_Fault1_All;//电压电流故障
            struct
            {
                uint16_t  bVBatOverFault          :1;//高压侧过压故障
                uint16_t  bVBatUnderFault         :1;//高压侧欠压故障
                uint16_t  bVBusOverFault          :1;//低压侧过压故障
                uint16_t  bVBusUnderFault         :1;//低压侧欠压故障
                uint16_t  bVUnBanlanFault         :1;//电压不平衡故障
                uint16_t  bIBatOverFault          :1;//高压侧过流故障
                uint16_t  bIBusOverFault          :1;//低压侧过流故障
                uint16_t  bIUnBanlanFault         :1;//电流不平衡故障
                uint16_t  bIgbtOverTempFault      :1;//IGBT过问故障
                uint16_t  bOverLoadTimeOutFault   :1;//过载超时故障
//                uint16_t  bVINVFault              :1;//逆变电压故障
//                uint16_t  bVINVUnBanaceFault      :1;//逆变电压不平衡
//                uint16_t  bINVIsLandFault         :1;//逆变器孤岛故障
//                uint16_t  bINVShortFault          :1;//逆变器短路故障
                uint16_t  bRsvd                   :7;
            }tbits;
        }DCDC_Fault1;

        union
        {
            uint16_t DCDC_Fault2_All;//DC故障
            struct
            {
                uint16_t  bEmergencyStop          :1;//外部急停
                uint16_t  bIGBTFault              :1;//IGBT故障，由光纤传过来
                uint16_t  bMIgbt1Fault            :1;//IGBT模块1故障
                uint16_t  bMIgbt2Fault            :1;//IGBT模块2故障
                uint16_t  bMIgbt3Fault            :1;//IGBT模块3故障
                uint16_t  bMIgbt4Fault            :1;//IGBT模块4故障
                uint16_t  bBusRelyTimeOutFault    :1;//直流继电器开路故障
                uint16_t  bBatRelyTimeOutFault    :1;//直流继电器开路故障
                uint16_t  bBUSRelyShortFault      :1;//直流继电器短路故障
                uint16_t  bRsvd                   :7;
            }tbits;
        }DCDC_Fault2;

        union
        {
            uint16_t DCDC_Fault3_All;
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
                uint16_t  bFiberIgbtFault          :1;//光纤接收板通信故障
                uint16_t  bRsvd                    :5;
            }tbits;
        }DCDC_Fault3;

        union
        {
            uint16_t DCDC_Fault4_All;
            struct
            {
                uint16_t  DCRPFault                :1;//极性反接故障
                uint16_t  DC_Bus_ShortFault        :1;//低压短路故障
                uint16_t  DC_Bat_ShortFault        :1;//高压短路故障
                uint16_t  bRsvd                    :13;
            }tbits;
        }DCDC_Fault4;

        union
        {
            uint16_t DCDC_HFault_All;
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
        }DCDC_HFault;

        int16_t  globalFault;
}St_DCDC_Fault;
//#pragma pack()

//
//typedef enum{
//    BAUD_4800 = 0,
//    BAUD_9600 =1,
//    BAUD_19200 =2,
//    BAUD_38400 =3,
//    BAUD_57600 =4,
//    BAUD_115200 =5
//}EN_UART_BAUD;
//

//extern int16_t m_i16_MasterSlave;
extern St_DCDC_Fault     FaultStatus;
extern TINV_CALIBRAT_PARAM   g_calibratParam;
//extern GLOAB_DCDC_PARAM    g_param;
//extern DCDC_SET_REG        g_SetReg;
extern Store_PARAM         store_param[EE_PARAM_CNT];
extern DCDC_Global_Param    DcDc_gParam;

//extern GRAPH    Graph1;
//extern GRAPH    Graph2;
//extern GRAPH    Graph3;
void cpu2claParam_Upgrade(void);
void bsp_init(void);
void App_AllFault_Reset(void);
void GloabParam_Init(void);
int16_t U16_DataChange(uint16_t datMsb,uint16_t datLsb);
void bsp_clb_pwmEnCtrl(int16_t enable);
void MPPT_PandO_Buck(void);
void MPPT_PandO_Boost(void);
void MPPT_INC_Buck(void);
void MPPT_INC_Boost(void);
void Uid_GetRegs(uint8_t *UID_Reg);
void Read_AesParam(uint8_t *AesParam);
int16_t Aes_Check(void);
//void  GRAPH_calc(GRAPH *g, float32_t v);


#endif /* _BSP_H_ */
