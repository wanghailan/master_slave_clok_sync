/*
 * bsp.h
 *
 *  Created on: 2024锟斤拷9锟斤拷27锟斤拷
 *      Author: guowei680
 */

#ifndef _BSP_H_
#define _BSP_H_

#define PWM_SYNC_ROLE_NONE    0
#define PWM_SYNC_ROLE_MASTER  1
#define PWM_SYNC_ROLE_SLAVE   2

#ifndef PWM_SYNC_ROLE
#if defined(PTP_SYNC_ROLE_MASTER)
#define PWM_SYNC_ROLE PWM_SYNC_ROLE_MASTER
#elif defined(PTP_SYNC_ROLE_SLAVE)
#define PWM_SYNC_ROLE PWM_SYNC_ROLE_SLAVE
#else
#define PWM_SYNC_ROLE PWM_SYNC_ROLE_NONE
#endif
#endif

/*
 * Standalone PWM sync bring-up mode forces PCS state and PWM start enable so
 * EPWM1A/B can be measured without CM/UI commands. Keep it disabled for
 * normal PCS operation; enable only in a dedicated scope/sync test build.
 */
#ifndef PWM_SYNC_STANDALONE_OUTPUT
#define PWM_SYNC_STANDALONE_OUTPUT 1
#endif

#define PWM_SYNC_PPS_CAPTURE_LED_CH 1

//#include "Drv_SCIA.h"
#include "Drv_SCIB.h"
#include "Drv_SCIC.h"
#include "Drv_SCID.h"
#include "Drv_LED.h"
#include "Drv_Timer.h"
#include "Drv_ADC.h"
#include "Drv_PWM.h"
#include "Drv_I2CA.h"
#include "Drv_W5500.h"
#include "Drv_tca9535_IO1.h"
#include "Drv_tca9535_IO2.h"
#include "Drv_DO.h"
#include "Drv_DIN.h"
#include "initial_CLA.h"
#include "Drv_EtherNet.h"

#include "obj_ipc.h"

#include "obj_OutMeter.h"
#include "obj_Control.h"
#include "obj_Protect.h"

#include "board.h"
#include "clb_config.h"
#include "clb.h"

#include "obj_setting.h"

#include "rampgen.h"
#include "abc_dq0_pos.h"
#include "abc_dq0_neg.h"
#include "dq0_abc.h"
#include "clarkT.h"
#include "parkT.h"

#include "spll_3ph_srf.h"
#include "spll_3ph_ddsrf.h"
#include "power_meas_sine_analyzer.h"
//#include "C28x_FPU_FastRTS.h"


/*
Baud rate   Bit rate     Bit time    Character time   3.5 character times
  2400      2400 bits/s   417 us          4.6 ms          16 ms
  4800      4800 bits/s   208 us          2.3 ms          8.0 ms
  9600      9600 bits/s   104 us          1.2 ms          4.0 ms
 19200     19200 bits/s    52 us          573 us          2.0 ms
 38400     38400 bits/s    26 us          286 us          1.75 ms(1.0 ms)
 115200    115200 bit/s   8.7 us           95 us          1.75 ms(0.33 ms) 锟斤拷锟斤拷潭锟斤拷锟轿?750us
*/

#define  SciRxTimeOut         58 //ADC锟叫讹拷31.25us 1.75ms锟斤拷57


enum
{
    GRID_DISCONNECTED = 0,
    GRID_CONNECTED  = 1
};

//typedef struct
//{
//    float32_t buf[200];
//    uint16_t i;
//}GRAPH;


//#pragma pack(1) //锟斤拷锟斤拷锟节达拷锟斤拷锟轿拷锟?锟街节讹拷锟斤拷
typedef struct _Stru_Fault
{
    union
     {
         uint16_t PCS_Waring1_All;
         struct
         {
             uint16_t  bOverLoad               :1;//锟斤拷锟截撅拷锟斤拷
             uint16_t  bOverTempReduLimit      :1;//锟斤拷锟铰斤拷锟筋警锟斤拷
             uint16_t  bUncontrRectification   :1;//锟斤拷锟斤拷锟斤拷锟斤拷
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
         uint16_t PCS_Fault1_All;//AC锟斤拷锟斤拷
         struct
         {
             uint16_t  bACVOverFault           :1;//锟斤拷锟斤拷锟斤拷压锟斤拷锟斤拷
             uint16_t  bACVUnderFault          :1;//锟斤拷锟斤拷欠压锟斤拷锟斤拷
             uint16_t  bACVBanlanceFault       :1;//锟斤拷锟斤拷锟斤拷压锟斤拷平锟斤拷锟斤拷锟?
             uint16_t  bACVPhaseSequFault      :1;//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟?
             uint16_t  bACVFreqFault           :1;//锟斤拷锟斤拷频锟绞癸拷锟斤拷
             uint16_t  bNLostFault             :1;//缺N锟竭癸拷锟斤拷
             uint16_t  bPELostFault            :1;//缺PE锟斤拷锟斤拷
             uint16_t  bIINVOverFault          :1;//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
             uint16_t  bOverLoadTimeOutFault   :1;//锟斤拷锟截筹拷时锟斤拷锟斤拷
             uint16_t  bVINVFault              :1;//锟斤拷锟斤拷压锟斤拷锟斤拷
             uint16_t  bVINVUnBanaceFault      :1;//锟斤拷锟斤拷压锟斤拷平锟斤拷
             uint16_t  bVINVFreqFault          :1;//锟斤拷锟狡碉拷使锟斤拷锟?
             uint16_t  bINVIsLandFault         :1;//锟斤拷锟斤拷锟斤拷碌锟斤拷锟斤拷锟?
             uint16_t  bINVShortFault          :1;//锟斤拷锟斤拷锟斤拷锟铰凤拷锟斤拷锟?
             uint16_t  bINVSoftRelyFault       :1;//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
             uint16_t  bGridACVFault           :1;//锟斤拷锟斤拷锟斤拷压锟斤拷锟斤拷
         }tbits;
     }PCS_Fault1;

     union
     {
         uint16_t PCS_Fault2_All;//DC锟斤拷锟斤拷
         struct
         {
             uint16_t  bBUSVOverFault          :1;//母锟竭癸拷压锟斤拷锟斤拷
             uint16_t  bBUSVUnderFault         :1;//母锟斤拷欠压锟斤拷锟斤拷
             uint16_t  bBUSVUnBanlanceFault    :1;//母锟竭碉拷压锟斤拷平锟斤拷锟斤拷锟?
             uint16_t  bBUSIOverFault          :1;//母锟竭癸拷锟斤拷锟斤拷锟斤拷
             uint16_t  bBUSReversedFault       :1;//母锟竭凤拷锟接癸拷锟斤拷
             uint16_t  bBUSSoftRelyTOutFault   :1;//母锟斤拷锟斤拷锟斤拷时锟斤拷锟斤拷
             uint16_t  bBUSRelyOPenFault       :1;//直锟斤拷锟教碉拷锟斤拷锟斤拷路锟斤拷锟斤拷
             uint16_t  bBUSRelyShortFault      :1;//直锟斤拷锟教碉拷锟斤拷锟斤拷路锟斤拷锟斤拷
             uint16_t  bDCIMFault              :1;//直锟斤拷锟斤拷缘锟斤拷锟斤拷锟斤拷锟?
             uint16_t  bRsvd                   :7;
         }tbits;
     }PCS_Fault2;

     union
     {
         uint16_t PCS_Fault3_All;
         struct
         {
             uint16_t  bAmbTemptOverFault       :1;//锟斤拷锟斤拷锟斤拷锟铰癸拷锟斤拷
             uint16_t  bIgbtTemptOverFault      :1;//IGBT锟斤拷锟铰癸拷锟斤拷
             uint16_t  bAuxPowerFault           :1;//锟斤拷源锟斤拷锟斤拷
             uint16_t  bFanFault                :1;//锟斤拷锟饺癸拷锟斤拷
             uint16_t  bEEpromFault             :1;//EE锟芥储锟斤拷锟斤拷锟斤拷
             uint16_t  bRs485_1Fault            :1;//RS485模锟斤拷1锟斤拷锟斤拷
             uint16_t  bRs485_2Fault            :1;//RS485模锟斤拷2锟斤拷锟斤拷
             uint16_t  bCAN_1Fault              :1;//CAN1锟斤拷锟斤拷
             uint16_t  bCAN_2Fault              :1;//CAN2锟斤拷锟斤拷
             uint16_t  bEtherNETFault           :1;//锟斤拷太锟斤拷锟斤拷锟斤拷
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
             uint16_t  bDCIOver_Charge           :1;//直锟斤拷锟斤拷锟斤拷锟斤拷
             uint16_t  bDCIOver_DisCharge        :1;//直锟斤拷锟脚碉拷锟斤拷锟?
             uint16_t  bACAIOver_Charge          :1;//A锟斤拷锟斤拷锟斤拷锟?
             uint16_t  bACAIOver_DisCharge       :1;//A锟斤拷诺锟斤拷锟斤拷
             uint16_t  bACBIOver_Charge          :1;//B锟斤拷锟斤拷锟斤拷锟?
             uint16_t  bACBIOver_DisCharge       :1;//B锟斤拷诺锟斤拷锟斤拷
             uint16_t  bACCIOver_Charge          :1;//C锟斤拷锟斤拷锟斤拷锟?
             uint16_t  bACCIOver_DisCharge       :1;//C锟斤拷诺锟斤拷锟斤拷
             uint16_t  bACHIOver_Charge          :1;//AC硬锟斤拷锟斤拷锟斤拷锟斤拷
             uint16_t  bACHIOver_DisCharge       :1;//AC硬锟斤拷锟脚碉拷锟斤拷锟?
             uint16_t  bDCHIOver_Charge          :1;//DC硬锟斤拷锟斤拷锟斤拷锟斤拷
             uint16_t  bDCHIOver_DisCharge       :1;//DC硬锟斤拷锟脚碉拷锟斤拷锟?
             uint16_t  bEmergencyStop            :1;//锟斤拷停锟斤拷锟斤拷
         }tbits;
     }PCS_HFault;

    int16_t  globalFault;
}Stru_Fault;
//#pragma pack()

typedef struct _IPC_DATA_CPU2CM //
{
    int16_t   PCS_AC_VRms_PhaseA;        //A锟斤拷锟窖癸拷锟?.1V
    int16_t   PCS_AC_VRms_PhaseB;        //B锟斤拷锟窖癸拷锟?.1V
    int16_t   PCS_AC_VRms_PhaseC;        //C锟斤拷锟窖癸拷锟?.1V

    int16_t   PCS_AC_VRms_LineAB;        //AB锟竭碉拷压锟斤拷0.1V
    int16_t   PCS_AC_VRms_LineBC;        //BC锟竭碉拷压锟斤拷0.1V
    int16_t   PCS_AC_VRms_LineCA;        //CA锟竭碉拷压锟斤拷0.1V

    int16_t   PCS_AC_VLRms_PhaseA;       //A锟斤拷锟窖癸拷锟?.1V
    int16_t   PCS_AC_VLRms_PhaseB;       //B锟斤拷锟窖癸拷锟?.1V
    int16_t   PCS_AC_VLRms_PhaseC;       //C锟斤拷锟窖癸拷锟?.1V

    int16_t   PCS_AC_IRms_A;             //A锟斤拷锟斤拷锟斤拷锟?.1A
    int16_t   PCS_AC_IRms_B;             //B锟斤拷锟斤拷锟斤拷锟?.1A
    int16_t   PCS_AC_IRms_C;             //C锟斤拷锟斤拷锟斤拷锟?.1A

    int16_t   PCS_AC_ILRms_A;             //A1锟斤拷锟斤拷锟斤拷锟?.1A
    int16_t   PCS_AC_ILRms_B;             //B1锟斤拷锟斤拷锟斤拷锟?.1A
    int16_t   PCS_AC_ILRms_C;             //C1锟斤拷锟斤拷锟斤拷锟?.1A

    int16_t   PCS_AC_PowerP_A;            //A锟斤拷锟叫癸拷锟斤拷锟绞ｏ拷0.01KW
    int16_t   PCS_AC_PowerP_B;            //A锟斤拷锟叫癸拷锟斤拷锟绞ｏ拷0.01KW
    int16_t   PCS_AC_PowerP_C;            //A锟斤拷锟叫癸拷锟斤拷锟绞ｏ拷0.01KW

    int16_t   PCS_AC_PowerQ_A;            //A锟斤拷锟睫癸拷锟斤拷锟绞ｏ拷0.01KVar
    int16_t   PCS_AC_PowerQ_B;            //B锟斤拷锟睫癸拷锟斤拷锟绞ｏ拷0.01KVar
    int16_t   PCS_AC_PowerQ_C;            //C锟斤拷锟睫癸拷锟斤拷锟绞ｏ拷0.01KVar

    int16_t   PCS_AC_PowerS_A;            //A锟斤拷锟斤拷锟节癸拷锟绞ｏ拷0.01KVA
    int16_t   PCS_AC_PowerS_B;            //B锟斤拷锟斤拷锟节癸拷锟绞ｏ拷0.01KVA
    int16_t   PCS_AC_PowerS_C;            //C锟斤拷锟斤拷锟节癸拷锟绞ｏ拷0.01KVA

    int16_t   PCS_AC_PowerP;              //锟斤拷锟斤拷锟斤拷锟叫癸拷锟斤拷锟绞ｏ拷0.01KW
    int16_t   PCS_AC_PowerQ;              //锟斤拷锟斤拷锟斤拷锟睫癸拷锟斤拷锟绞ｏ拷0.01KVar
    int16_t   PCS_AC_PowerS;              //锟斤拷锟斤拷锟斤拷锟斤拷锟节癸拷锟绞ｏ拷0.01KVA

    int16_t   PCS_AC_Freq_A;              //A锟斤拷频锟绞ｏ拷0.01HZ
    int16_t   PCS_AC_Freq_B;              //B锟斤拷频锟绞ｏ拷0.01HZ
    int16_t   PCS_AC_Freq_C;              //C锟斤拷频锟绞ｏ拷0.01HZ

    int16_t   PCS_AC_PF_A;                //A锟洁功锟斤拷锟斤拷锟斤拷锟斤拷 0.01
    int16_t   PCS_AC_PF_B;                //B锟洁功锟斤拷锟斤拷锟斤拷锟斤拷 0.01
    int16_t   PCS_AC_PF_C;                //C锟洁功锟斤拷锟斤拷锟斤拷锟斤拷 0.01

    int16_t   PCS_DC_BusVol;              //母锟竭诧拷直锟斤拷锟斤拷压
    int16_t   PCS_DC_BusCurr;             //母锟竭诧拷直锟斤拷锟斤拷锟斤拷锟斤拷0.01A
    int16_t   PCS_DC_Power;               //母锟竭诧拷直锟斤拷锟斤拷锟绞ｏ拷0.01KW

    int16_t   PCS_DC_PBusVol;             //锟斤拷母锟竭碉拷压锟斤拷0.1V
    int16_t   PCS_DC_NBusVol;             //锟斤拷母锟竭碉拷压锟斤拷0.1V
    int16_t   PCS_DC_IBUS_N;              //N锟竭碉拷锟斤拷锟斤拷0.1A

    int16_t   Temp_igbtMax;               //IGBT锟铰讹拷锟斤拷锟街?
    int16_t   Temp_igbtA;                 //IGBT锟铰讹拷 A锟斤拷
    int16_t   Temp_igbtB;                 //IGBT锟铰讹拷 B锟斤拷
    int16_t   Temp_igbtC;                 //IGBT锟铰讹拷 C锟斤拷
    int16_t   Temp_igbtN;                 //IGBT锟铰讹拷 N锟斤拷
    int16_t   Temp_AmbInlet;              //锟斤拷锟斤拷诨锟斤拷锟斤拷露锟?
    int16_t   Temp_AmbOutlet;             //锟斤拷锟斤拷诨锟斤拷锟斤拷露锟?

    int16_t   Vol_1V5;                    //+1.5V  0.001
    int16_t   Vol_5V;                     //+5V  0.001
    int16_t   Vol_p15V;                   //+15V  0.001

    int16_t   Pcs_Localpllfo;             //锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟脚猴拷锟斤拷锟洁环频锟斤拷
    int16_t   Pcs_Gridpllfo;              //锟斤拷锟斤拷锟脚猴拷锟斤拷锟洁环频锟斤拷

    int16_t   PwmRatio;                   //PWM1锟斤拷锟狡憋拷锟斤拷(100.00%)
    int16_t   DerateRatio;                //锟斤拷锟铰斤拷锟斤拷锟斤拷锟叫百分憋拷
    int16_t   pllPhaseErr;                //锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷位锟斤拷

    int16_t   Relay_DCSoft;               //直锟斤拷锟斤拷锟斤拷痰锟斤拷锟斤拷锟斤拷锟阶刺?
    int16_t   Relay_DCMaster;             //直锟斤拷锟斤拷锟教碉拷锟斤拷锟斤拷锟斤拷状态
    int16_t   Relay_ACSoft;               //锟斤拷锟斤拷锟斤拷锟斤拷痰锟斤拷锟斤拷锟斤拷锟阶刺?
    int16_t   Relay_ACMaster;             //锟斤拷锟斤拷锟斤拷锟教碉拷锟斤拷锟斤拷锟斤拷状态
    int16_t   Relay_FanCtrl;              //锟斤拷锟饺匡拷锟斤拷
    int16_t   Relay_WorkLed;              //锟斤拷锟斤拷指示锟斤拷
    int16_t   Relay_FaultLed;             //锟斤拷锟斤拷指示锟斤拷
    int16_t   FanCtrl_duty;               //锟斤拷锟饺匡拷锟斤拷

    int16_t   PcsCtrlState;               //锟斤拷锟斤拷状态锟斤拷状态
    int16_t   PcsOnAllowed;               //锟斤拷锟斤拷锟斤拷 0:锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷1:锟斤拷锟斤拷锟斤拷
    int16_t   uniqueID_M;                 //CPU_UID 锟斤拷16位
    int16_t   uniqueID_L;                 //CPU_UID 锟斤拷16位

    int16_t   Pwm_StartEn;                //cla锟叫的凤拷锟斤拷使锟杰憋拷志

   Stru_Fault  FaultStatus;
}IPC_DATA_CPU2CM;

typedef struct _IPC_DATA_CM2CPU //
{
    //锟斤拷锟斤拷模式锟借定
    int16_t   StartEn;                //锟斤拷锟斤拷/停止
    int16_t   WorkMode;               //锟斤拷锟斤拷模式锟斤拷0:锟斤拷锟斤拷,1:锟斤拷锟斤拷
    int16_t   OnGridMode;             //锟斤拷锟斤拷锟斤拷锟斤拷模式(0:锟姐功锟斤拷,1:锟斤拷锟斤拷,2:锟斤拷直锟斤拷锟斤拷压,3:锟斤拷直锟斤拷锟斤拷锟斤拷)
    int16_t   LocalRemote;            //0:锟斤拷锟斤拷,1:远锟斤拷
    int16_t   Pwm_StartEn;            //Pwm锟斤拷锟斤拷使锟斤拷
    int16_t   OutLoop_En;             //锟斤拷锟斤拷锟解环使锟斤拷    0锟斤拷锟截憋拷锟解环锟斤拷1锟斤拷锟斤拷锟解环
    int16_t   SoftStart_En;           //锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷使锟斤拷   0锟斤拷锟截憋拷锟斤拷锟斤拷锟斤拷锟斤拷1锟斤拷锟斤拷锟斤拷锟斤拷
    int16_t   debugMode;              //0:锟斤拷锟斤拷模式锟斤拷1:锟斤拷锟斤拷模式
    int16_t   OpenLoopMode;           //0:锟斤拷锟斤拷模式锟斤拷1:锟秸伙拷模式
    int16_t   pcsLineMode;            //0:3P3L锟斤拷1:3P4L
    int16_t   ClearFault;             //锟斤拷锟斤拷锟斤拷锟斤拷锟?:锟斤拷锟?
    int16_t   debugData_TxEn;         //锟斤拷锟斤拷锟斤拷锟捷凤拷锟斤拷使锟斤拷
    int16_t   Reset_En;               //锟斤拷位使锟斤拷 锟截闭匡拷锟脚癸拷
    //锟斤拷锟狡诧拷锟斤拷锟借定
    int16_t   ActivePowerSet;         //锟叫癸拷锟斤拷锟斤拷锟借定
    int16_t   ReActivePowerSet;       //锟睫癸拷锟斤拷锟斤拷锟借定
    int16_t   DcConstCurrSet;         //锟斤拷直锟斤拷锟斤拷锟斤拷锟借定
    int16_t   DcConstVolSet;          //锟斤拷直锟斤拷锟斤拷压锟借定
    int16_t   AcConstCurrSet;         //锟姐交锟斤拷锟斤拷锟斤拷锟借定
    int16_t   PowerFactorSet;         //锟斤拷锟斤拷锟斤拷锟斤拷锟借定
    int16_t   VF_VoSet;               //VF锟斤拷锟斤拷锟窖癸拷锟斤拷锟?
    int16_t   VF_FreqSet;             //VF锟斤拷锟狡碉拷锟斤拷锟斤拷锟?
    int16_t   DcEqualChargeVol;       //DC锟斤拷锟斤拷锟窖癸拷锟斤拷锟?
    int16_t   DcFloatChargeVol;       //DC锟斤拷锟斤拷锟窖癸拷锟斤拷锟?
    int16_t   RatedVoltage;             //锟筋定锟斤拷压锟借定
    int16_t   RatedCurrent;             //锟筋定锟斤拷锟斤拷锟借定
    int16_t   RatedPower;               //锟筋定锟斤拷锟斤拷锟借定
    int16_t   DcEqual2FloatCurrent;   //DC锟斤拷锟斤拷转锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟?
    int16_t   BattHighVolSet;         //锟斤拷氐锟窖癸拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟?
    int16_t   BattLowVolSet;          //锟斤拷氐锟窖癸拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟?
    int16_t   BattEODVol;             //EOD锟斤拷止锟脚碉拷锟窖癸拷锟斤拷锟?
    int16_t   DODGridConnect;         //锟斤拷锟斤拷DOD锟斤拷锟斤拷 锟脚碉拷锟斤拷锟?
    int16_t   DODDisGridConnect;      //锟斤拷锟斤拷DOD锟斤拷锟斤拷 锟脚碉拷锟斤拷锟?
    //AD校准锟斤拷锟斤拷
    int16_t   VgridA_Calibrat;    //A锟斤拷锟斤拷锟斤拷锟窖剐Ｗ?(80.00%~120.00%) 8000~12000
    int16_t   VgridB_Calibrat;    //B锟斤拷锟斤拷锟斤拷锟窖剐Ｗ?(80.00%~120.00%) 8000~12000
    int16_t   VgridC_Calibrat;    //C锟斤拷锟斤拷锟斤拷锟窖剐Ｗ?(80.00%~120.00%) 8000~12000
    int16_t   VInvA_Calibrat;     //A锟斤拷锟斤拷锟斤拷压校准 (80.00%~120.00%) 8000~12000
    int16_t   VInvB_Calibrat;     //B锟斤拷锟斤拷锟斤拷压校准 (80.00%~120.00%) 8000~12000
    int16_t   VInvC_Calibrat;     //C锟斤拷锟斤拷锟斤拷压校准 (80.00%~120.00%) 8000~12000
    int16_t   IGridA_Calibrat;    //A锟斤拷锟斤拷锟斤拷锟斤拷锟叫Ｗ?(80.00%~120.00%) 8000~12000
    int16_t   IGridB_Calibrat;    //B锟斤拷锟斤拷锟斤拷锟斤拷锟叫Ｗ?(80.00%~120.00%) 8000~12000
    int16_t   IGridC_Calibrat;    //C锟斤拷锟斤拷锟斤拷锟斤拷锟叫Ｗ?(80.00%~120.00%) 8000~12000
    int16_t   IInvA_Calibrat;     //A锟斤拷锟斤拷锟斤拷锟斤拷校准 (80.00%~120.00%) 8000~12000
    int16_t   IInvB_Calibrat;     //B锟斤拷锟斤拷锟斤拷锟斤拷校准 (80.00%~120.00%) 8000~12000
    int16_t   IInvC_Calibrat;     //C锟斤拷锟斤拷锟斤拷锟斤拷校准 (80.00%~120.00%) 8000~12000
    int16_t   VdcP_Calibrat;      //直锟斤拷锟斤拷压锟斤拷校准 (80.00%~120.00%) 8000~12000
    int16_t   VdcN_Calibrat;      //直锟斤拷锟斤拷压锟斤拷校准 (80.00%~120.00%) 8000~12000
    int16_t   Idc_Calibrat;       //直锟斤拷锟斤拷锟斤拷校准 (80.00%~120.00%) 8000~12000
    int16_t   Vdc_Calibrat;       //直锟斤拷锟斤拷压校准 (80.00%~120.00%) 8000~12000
    int16_t   Ref1v5_Calibrat;      //直锟斤拷锟斤拷压偏锟斤拷校准 (80.00%~120.00%) 8000~12000
    int16_t   IN_Calibrat;        //N锟竭碉拷锟斤拷校准 (80.00%~120.00%) 8000~12000
    int16_t   Idc_offset;
    int16_t   Iac_Aoffset;
    int16_t   Iac_Boffset;
    int16_t   Iac_Coffset;
    int16_t   Ilac_Aoffset;
    int16_t   Ilac_Boffset;
    int16_t   Ilac_Coffset;
    int16_t   IdcN_offset;
    int16_t   Reserved_Calibrat;
    //PI锟斤拷锟狡诧拷锟斤拷
    int16_t   IlInter_Kp;               //锟斤拷械锟斤拷锟斤拷诨锟終p锟斤拷锟斤拷
    int16_t   IlInter_Ki;               //锟斤拷械锟斤拷锟斤拷诨锟終i锟斤拷锟斤拷
    int16_t   IAcOuter_Kp;              //AC锟斤拷锟斤拷锟解环Kp锟斤拷锟斤拷
    int16_t   IAcOuter_Ki;              //AC锟斤拷锟斤拷锟解环Ki锟斤拷锟斤拷
    int16_t   IDcOuter_Kp;              //DC锟斤拷锟斤拷锟解环Kp锟斤拷锟斤拷
    int16_t   IDcOuter_Ki;              //DC锟斤拷锟斤拷锟解环Ki锟斤拷锟斤拷
    int16_t   VAcOuter_Kp;              //AC锟斤拷压锟解环Kp锟斤拷锟斤拷
    int16_t   VAcOuter_Ki;              //AC锟斤拷压锟解环Ki锟斤拷锟斤拷
    int16_t   VDcOuter_Kp;              //DC锟斤拷压锟解环Kp锟斤拷锟斤拷
    int16_t   VDcOuter_Ki;              //DC锟斤拷压锟解环Ki锟斤拷锟斤拷
    //锟斤拷锟较憋拷锟斤拷锟斤拷值
    int16_t   AC_OverVol_Value;        //锟斤拷锟斤拷锟斤拷压锟斤拷锟斤拷锟斤拷值
    int16_t   AC_UnderVol_Value;       //锟斤拷锟斤拷欠压锟斤拷锟斤拷锟斤拷值
    int16_t   AC_OverCurr_Value;       //锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷值
    int16_t   AC_UnbanceVol_Value;     //锟斤拷锟斤拷锟斤拷压锟斤拷平锟解报锟斤拷锟斤拷值

    int16_t   DC_OverVol_Value;        //直锟斤拷锟斤拷压锟斤拷锟斤拷锟斤拷值
    int16_t   DC_OverCurr_Value;       //直锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷值
    int16_t   DC_UnderVol_Value;       //直锟斤拷欠压锟斤拷锟斤拷锟斤拷值
    int16_t   DC_UnbanceVol_Value;     //直锟斤拷锟斤拷压锟斤拷平锟解报锟斤拷锟斤拷值
    int16_t   TempAmb_Over_Value;      //锟斤拷锟斤拷锟斤拷锟铰憋拷锟斤拷锟斤拷值
    int16_t   TempIgbt_Over_Value;     //IGBT锟斤拷锟铰憋拷锟斤拷锟斤拷值
    //锟街讹拷锟斤拷锟斤拷时锟斤拷锟斤拷CM锟斤拷锟斤拷CPU1
    int16_t   FanCtrl_duty;           //锟斤拷锟饺匡拷锟斤拷
    int16_t   RelayCtrl_DCSoft;       //直锟斤拷锟斤拷锟斤拷痰锟斤拷锟斤拷锟斤拷锟阶刺?
    int16_t   RelayCtrl_DCMaster;     //直锟斤拷锟斤拷锟教碉拷锟斤拷锟斤拷锟斤拷状态
    int16_t   RelayCtrl_ACSoft;       //锟斤拷锟斤拷锟斤拷锟斤拷痰锟斤拷锟斤拷锟斤拷锟阶刺?
    int16_t   RelayCtrl_ACMaster;     //锟斤拷锟斤拷锟斤拷锟教碉拷锟斤拷锟斤拷锟斤拷状态
    int16_t   RelayCtrl_FanCtrl;      //锟斤拷锟饺匡拷锟斤拷
    int16_t   RelayCtrl_WorkLed;      //锟斤拷锟斤拷指示锟斤拷
    int16_t   RelayCtrl_FaultLed;     //锟斤拷锟斤拷指示锟斤拷
    //CM锟较碉拷锟皆硷拷状态
    int16_t   SelfCheckOK;            //CM锟较碉拷锟皆硷拷状态
    int16_t   IpcCpu2Cm_Fault;        //CPU锟斤拷CM Ipc通锟脚癸拷锟斤拷
    int16_t   Rs485_1_Fault;          //RS485_1通锟脚癸拷锟斤拷
    int16_t   Rs485_2_Fault;          //RS485_2通锟脚癸拷锟斤拷
    int16_t   CAN_1_Fault;            //CAN_1通锟脚癸拷锟斤拷
    int16_t   CAN_2_Fault;            //CAN_2通锟脚癸拷锟斤拷
    int16_t   EE_Fault;               //EEPROM锟斤拷锟斤拷
    int16_t   Sflash_Fault;           //spi flash通锟脚癸拷锟斤拷
    int16_t   PtpSynced;              //0:未同锟斤拷,1:锟斤拷同锟斤拷
}IPC_DATA_CM2CPU;

extern IPC_DATA_CPU2CM         Cpu1Ipc_cpu2cm;
extern IPC_DATA_CM2CPU         Cpu1Ipc_cm2cpu;
extern SPLL_3PH_DDSRF PCS_spll_3ph_grid;

void cpu2claParam_Upgrade(void);
void bsp_init(void);
void App_AllFault_Reset(void);
void GloabParam_Init(void);
int16_t U16_DataChange(uint16_t datMsb,uint16_t datLsb);
void bsp_clb_pwmEnCtrl(int16_t enable);
void Read_UID(void);
//void  GRAPH_calc(GRAPH *g, float32_t v);


#endif /* _BSP_H_ */
