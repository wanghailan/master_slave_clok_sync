/*
 * bsp.h
 *
 *  Created on: 2024��9��27��
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
 115200    115200 bit/s   8.7 us           95 us          1.75 ms(0.33 ms) ����̶����?750us
*/

#define  SciRxTimeOut         58 //ADC�ж�31.25us 1.75ms��57


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


//#pragma pack(1) //�����ڴ����Ϊ��?�ֽڶ���
typedef struct _Stru_Fault
{
    union
     {
         uint16_t PCS_Waring1_All;
         struct
         {
             uint16_t  bOverLoad               :1;//���ؾ���
             uint16_t  bOverTempReduLimit      :1;//���½����
             uint16_t  bUncontrRectification   :1;//��������
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
         uint16_t PCS_Fault1_All;//AC����
         struct
         {
             uint16_t  bACVOverFault           :1;//������ѹ����
             uint16_t  bACVUnderFault          :1;//����Ƿѹ����
             uint16_t  bACVBanlanceFault       :1;//������ѹ��ƽ�����?
             uint16_t  bACVPhaseSequFault      :1;//�����������?
             uint16_t  bACVFreqFault           :1;//����Ƶ�ʹ���
             uint16_t  bNLostFault             :1;//ȱN�߹���
             uint16_t  bPELostFault            :1;//ȱPE����
             uint16_t  bIINVOverFault          :1;//����������
             uint16_t  bOverLoadTimeOutFault   :1;//���س�ʱ����
             uint16_t  bVINVFault              :1;//����ѹ����
             uint16_t  bVINVUnBanaceFault      :1;//����ѹ��ƽ��
             uint16_t  bVINVFreqFault          :1;//���Ƶ�ʹ���?
             uint16_t  bINVIsLandFault         :1;//������µ�����?
             uint16_t  bINVShortFault          :1;//�������·����?
             uint16_t  bINVSoftRelyFault       :1;//������������
             uint16_t  bGridACVFault           :1;//������ѹ����
         }tbits;
     }PCS_Fault1;

     union
     {
         uint16_t PCS_Fault2_All;//DC����
         struct
         {
             uint16_t  bBUSVOverFault          :1;//ĸ�߹�ѹ����
             uint16_t  bBUSVUnderFault         :1;//ĸ��Ƿѹ����
             uint16_t  bBUSVUnBanlanceFault    :1;//ĸ�ߵ�ѹ��ƽ�����?
             uint16_t  bBUSIOverFault          :1;//ĸ�߹�������
             uint16_t  bBUSReversedFault       :1;//ĸ�߷��ӹ���
             uint16_t  bBUSSoftRelyTOutFault   :1;//ĸ������ʱ����
             uint16_t  bBUSRelyOPenFault       :1;//ֱ���̵�����·����
             uint16_t  bBUSRelyShortFault      :1;//ֱ���̵�����·����
             uint16_t  bDCIMFault              :1;//ֱ����Ե�������?
             uint16_t  bRsvd                   :7;
         }tbits;
     }PCS_Fault2;

     union
     {
         uint16_t PCS_Fault3_All;
         struct
         {
             uint16_t  bAmbTemptOverFault       :1;//�������¹���
             uint16_t  bIgbtTemptOverFault      :1;//IGBT���¹���
             uint16_t  bAuxPowerFault           :1;//��Դ����
             uint16_t  bFanFault                :1;//���ȹ���
             uint16_t  bEEpromFault             :1;//EE�洢������
             uint16_t  bRs485_1Fault            :1;//RS485ģ��1����
             uint16_t  bRs485_2Fault            :1;//RS485ģ��2����
             uint16_t  bCAN_1Fault              :1;//CAN1����
             uint16_t  bCAN_2Fault              :1;//CAN2����
             uint16_t  bEtherNETFault           :1;//��̫������
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
             uint16_t  bDCIOver_Charge           :1;//ֱ��������
             uint16_t  bDCIOver_DisCharge        :1;//ֱ���ŵ����?
             uint16_t  bACAIOver_Charge          :1;//A�������?
             uint16_t  bACAIOver_DisCharge       :1;//A��ŵ����
             uint16_t  bACBIOver_Charge          :1;//B�������?
             uint16_t  bACBIOver_DisCharge       :1;//B��ŵ����
             uint16_t  bACCIOver_Charge          :1;//C�������?
             uint16_t  bACCIOver_DisCharge       :1;//C��ŵ����
             uint16_t  bACHIOver_Charge          :1;//ACӲ��������
             uint16_t  bACHIOver_DisCharge       :1;//ACӲ���ŵ����?
             uint16_t  bDCHIOver_Charge          :1;//DCӲ��������
             uint16_t  bDCHIOver_DisCharge       :1;//DCӲ���ŵ����?
             uint16_t  bEmergencyStop            :1;//��ͣ����
         }tbits;
     }PCS_HFault;

    int16_t  globalFault;
}Stru_Fault;
//#pragma pack()

typedef struct _IPC_DATA_CPU2CM //
{
    int16_t   PCS_AC_VRms_PhaseA;        //A���ѹ��?.1V
    int16_t   PCS_AC_VRms_PhaseB;        //B���ѹ��?.1V
    int16_t   PCS_AC_VRms_PhaseC;        //C���ѹ��?.1V

    int16_t   PCS_AC_VRms_LineAB;        //AB�ߵ�ѹ��0.1V
    int16_t   PCS_AC_VRms_LineBC;        //BC�ߵ�ѹ��0.1V
    int16_t   PCS_AC_VRms_LineCA;        //CA�ߵ�ѹ��0.1V

    int16_t   PCS_AC_VLRms_PhaseA;       //A���ѹ��?.1V
    int16_t   PCS_AC_VLRms_PhaseB;       //B���ѹ��?.1V
    int16_t   PCS_AC_VLRms_PhaseC;       //C���ѹ��?.1V

    int16_t   PCS_AC_IRms_A;             //A�������?.1A
    int16_t   PCS_AC_IRms_B;             //B�������?.1A
    int16_t   PCS_AC_IRms_C;             //C�������?.1A

    int16_t   PCS_AC_ILRms_A;             //A1�������?.1A
    int16_t   PCS_AC_ILRms_B;             //B1�������?.1A
    int16_t   PCS_AC_ILRms_C;             //C1�������?.1A

    int16_t   PCS_AC_PowerP_A;            //A���й����ʣ�0.01KW
    int16_t   PCS_AC_PowerP_B;            //A���й����ʣ�0.01KW
    int16_t   PCS_AC_PowerP_C;            //A���й����ʣ�0.01KW

    int16_t   PCS_AC_PowerQ_A;            //A���޹����ʣ�0.01KVar
    int16_t   PCS_AC_PowerQ_B;            //B���޹����ʣ�0.01KVar
    int16_t   PCS_AC_PowerQ_C;            //C���޹����ʣ�0.01KVar

    int16_t   PCS_AC_PowerS_A;            //A�����ڹ��ʣ�0.01KVA
    int16_t   PCS_AC_PowerS_B;            //B�����ڹ��ʣ�0.01KVA
    int16_t   PCS_AC_PowerS_C;            //C�����ڹ��ʣ�0.01KVA

    int16_t   PCS_AC_PowerP;              //�������й����ʣ�0.01KW
    int16_t   PCS_AC_PowerQ;              //�������޹����ʣ�0.01KVar
    int16_t   PCS_AC_PowerS;              //���������ڹ��ʣ�0.01KVA

    int16_t   PCS_AC_Freq_A;              //A��Ƶ�ʣ�0.01HZ
    int16_t   PCS_AC_Freq_B;              //B��Ƶ�ʣ�0.01HZ
    int16_t   PCS_AC_Freq_C;              //C��Ƶ�ʣ�0.01HZ

    int16_t   PCS_AC_PF_A;                //A�๦�������� 0.01
    int16_t   PCS_AC_PF_B;                //B�๦�������� 0.01
    int16_t   PCS_AC_PF_C;                //C�๦�������� 0.01

    int16_t   PCS_DC_BusVol;              //ĸ�߲�ֱ����ѹ
    int16_t   PCS_DC_BusCurr;             //ĸ�߲�ֱ��������0.01A
    int16_t   PCS_DC_Power;               //ĸ�߲�ֱ�����ʣ�0.01KW

    int16_t   PCS_DC_PBusVol;             //��ĸ�ߵ�ѹ��0.1V
    int16_t   PCS_DC_NBusVol;             //��ĸ�ߵ�ѹ��0.1V
    int16_t   PCS_DC_IBUS_N;              //N�ߵ�����0.1A

    int16_t   Temp_igbtMax;               //IGBT�¶�����?
    int16_t   Temp_igbtA;                 //IGBT�¶� A��
    int16_t   Temp_igbtB;                 //IGBT�¶� B��
    int16_t   Temp_igbtC;                 //IGBT�¶� C��
    int16_t   Temp_igbtN;                 //IGBT�¶� N��
    int16_t   Temp_AmbInlet;              //����ڻ����¶�?
    int16_t   Temp_AmbOutlet;             //����ڻ����¶�?

    int16_t   Vol_1V5;                    //+1.5V  0.001
    int16_t   Vol_5V;                     //+5V  0.001
    int16_t   Vol_p15V;                   //+15V  0.001

    int16_t   Pcs_Localpllfo;             //�����������ź����໷Ƶ��
    int16_t   Pcs_Gridpllfo;              //�����ź����໷Ƶ��

    int16_t   PwmRatio;                   //PWM1���Ʊ���(100.00%)
    int16_t   DerateRatio;                //���½������аٷֱ�
    int16_t   pllPhaseErr;                //������������λ��

    int16_t   Relay_DCSoft;               //ֱ������̵�������״�?
    int16_t   Relay_DCMaster;             //ֱ�����̵�������״̬
    int16_t   Relay_ACSoft;               //��������̵�������״�?
    int16_t   Relay_ACMaster;             //�������̵�������״̬
    int16_t   Relay_FanCtrl;              //���ȿ���
    int16_t   Relay_WorkLed;              //����ָʾ��
    int16_t   Relay_FaultLed;             //����ָʾ��
    int16_t   FanCtrl_duty;               //���ȿ���

    int16_t   PcsCtrlState;               //����״̬��״̬
    int16_t   PcsOnAllowed;               //������ 0:����������1:������
    int16_t   uniqueID_M;                 //CPU_UID ��16λ
    int16_t   uniqueID_L;                 //CPU_UID ��16λ

    int16_t   Pwm_StartEn;                //cla�еķ���ʹ�ܱ�־

   Stru_Fault  FaultStatus;
}IPC_DATA_CPU2CM;

typedef struct _IPC_DATA_CM2CPU //
{
    //����ģʽ�趨
    int16_t   StartEn;                //����/ֹͣ
    int16_t   WorkMode;               //����ģʽ��0:����,1:����
    int16_t   OnGridMode;             //��������ģʽ(0:�㹦��,1:����,2:��ֱ����ѹ,3:��ֱ������)
    int16_t   LocalRemote;            //0:����,1:Զ��
    int16_t   Pwm_StartEn;            //Pwm����ʹ��
    int16_t   OutLoop_En;             //�����⻷ʹ��    0���ر��⻷��1�����⻷
    int16_t   SoftStart_En;           //����������ʹ��   0���ر���������1��������
    int16_t   debugMode;              //0:����ģʽ��1:����ģʽ
    int16_t   OpenLoopMode;           //0:����ģʽ��1:�ջ�ģʽ
    int16_t   pcsLineMode;            //0:3P3L��1:3P4L
    int16_t   ClearFault;             //���������?:���?
    int16_t   debugData_TxEn;         //�������ݷ���ʹ��
    int16_t   Reset_En;               //��λʹ�� �رտ��Ź�
    //���Ʋ����趨
    int16_t   ActivePowerSet;         //�й������趨
    int16_t   ReActivePowerSet;       //�޹������趨
    int16_t   DcConstCurrSet;         //��ֱ�������趨
    int16_t   DcConstVolSet;          //��ֱ����ѹ�趨
    int16_t   AcConstCurrSet;         //�㽻�������趨
    int16_t   PowerFactorSet;         //���������趨
    int16_t   VF_VoSet;               //VF�����ѹ����?
    int16_t   VF_FreqSet;             //VF���Ƶ������?
    int16_t   DcEqualChargeVol;       //DC�����ѹ����?
    int16_t   DcFloatChargeVol;       //DC�����ѹ����?
    int16_t   RatedVoltage;             //���ѹ�趨
    int16_t   RatedCurrent;             //������趨
    int16_t   RatedPower;               //������趨
    int16_t   DcEqual2FloatCurrent;   //DC����ת�����������?
    int16_t   BattHighVolSet;         //��ص�ѹ������������?
    int16_t   BattLowVolSet;          //��ص�ѹ������������?
    int16_t   BattEODVol;             //EOD��ֹ�ŵ��ѹ����?
    int16_t   DODGridConnect;         //����DOD���� �ŵ����?
    int16_t   DODDisGridConnect;      //����DOD���� �ŵ����?
    //ADУ׼����
    int16_t   VgridA_Calibrat;    //A�������ѹУ�?(80.00%~120.00%) 8000~12000
    int16_t   VgridB_Calibrat;    //B�������ѹУ�?(80.00%~120.00%) 8000~12000
    int16_t   VgridC_Calibrat;    //C�������ѹУ�?(80.00%~120.00%) 8000~12000
    int16_t   VInvA_Calibrat;     //A������ѹУ׼ (80.00%~120.00%) 8000~12000
    int16_t   VInvB_Calibrat;     //B������ѹУ׼ (80.00%~120.00%) 8000~12000
    int16_t   VInvC_Calibrat;     //C������ѹУ׼ (80.00%~120.00%) 8000~12000
    int16_t   IGridA_Calibrat;    //A���������У�?(80.00%~120.00%) 8000~12000
    int16_t   IGridB_Calibrat;    //B���������У�?(80.00%~120.00%) 8000~12000
    int16_t   IGridC_Calibrat;    //C���������У�?(80.00%~120.00%) 8000~12000
    int16_t   IInvA_Calibrat;     //A��������У׼ (80.00%~120.00%) 8000~12000
    int16_t   IInvB_Calibrat;     //B��������У׼ (80.00%~120.00%) 8000~12000
    int16_t   IInvC_Calibrat;     //C��������У׼ (80.00%~120.00%) 8000~12000
    int16_t   VdcP_Calibrat;      //ֱ����ѹ��У׼ (80.00%~120.00%) 8000~12000
    int16_t   VdcN_Calibrat;      //ֱ����ѹ��У׼ (80.00%~120.00%) 8000~12000
    int16_t   Idc_Calibrat;       //ֱ������У׼ (80.00%~120.00%) 8000~12000
    int16_t   Vdc_Calibrat;       //ֱ����ѹУ׼ (80.00%~120.00%) 8000~12000
    int16_t   Ref1v5_Calibrat;      //ֱ����ѹƫ��У׼ (80.00%~120.00%) 8000~12000
    int16_t   IN_Calibrat;        //N�ߵ���У׼ (80.00%~120.00%) 8000~12000
    int16_t   Idc_offset;
    int16_t   Iac_Aoffset;
    int16_t   Iac_Boffset;
    int16_t   Iac_Coffset;
    int16_t   Ilac_Aoffset;
    int16_t   Ilac_Boffset;
    int16_t   Ilac_Coffset;
    int16_t   IdcN_offset;
    int16_t   Reserved_Calibrat;
    //PI���Ʋ���
    int16_t   IlInter_Kp;               //��е����ڻ�Kp����
    int16_t   IlInter_Ki;               //��е����ڻ�Ki����
    int16_t   IAcOuter_Kp;              //AC�����⻷Kp����
    int16_t   IAcOuter_Ki;              //AC�����⻷Ki����
    int16_t   IDcOuter_Kp;              //DC�����⻷Kp����
    int16_t   IDcOuter_Ki;              //DC�����⻷Ki����
    int16_t   VAcOuter_Kp;              //AC��ѹ�⻷Kp����
    int16_t   VAcOuter_Ki;              //AC��ѹ�⻷Ki����
    int16_t   VDcOuter_Kp;              //DC��ѹ�⻷Kp����
    int16_t   VDcOuter_Ki;              //DC��ѹ�⻷Ki����
    //���ϱ�����ֵ
    int16_t   AC_OverVol_Value;        //������ѹ������ֵ
    int16_t   AC_UnderVol_Value;       //����Ƿѹ������ֵ
    int16_t   AC_OverCurr_Value;       //��������������ֵ
    int16_t   AC_UnbanceVol_Value;     //������ѹ��ƽ�ⱨ����ֵ

    int16_t   DC_OverVol_Value;        //ֱ����ѹ������ֵ
    int16_t   DC_OverCurr_Value;       //ֱ������������ֵ
    int16_t   DC_UnderVol_Value;       //ֱ��Ƿѹ������ֵ
    int16_t   DC_UnbanceVol_Value;     //ֱ����ѹ��ƽ�ⱨ����ֵ
    int16_t   TempAmb_Over_Value;      //�������±�����ֵ
    int16_t   TempIgbt_Over_Value;     //IGBT���±�����ֵ
    //�ֶ�����ʱ����CM����CPU1
    int16_t   FanCtrl_duty;           //���ȿ���
    int16_t   RelayCtrl_DCSoft;       //ֱ������̵�������״�?
    int16_t   RelayCtrl_DCMaster;     //ֱ�����̵�������״̬
    int16_t   RelayCtrl_ACSoft;       //��������̵�������״�?
    int16_t   RelayCtrl_ACMaster;     //�������̵�������״̬
    int16_t   RelayCtrl_FanCtrl;      //���ȿ���
    int16_t   RelayCtrl_WorkLed;      //����ָʾ��
    int16_t   RelayCtrl_FaultLed;     //����ָʾ��
    //CM�ϵ��Լ�״̬
    int16_t   SelfCheckOK;            //CM�ϵ��Լ�״̬
    int16_t   IpcCpu2Cm_Fault;        //CPU��CM Ipcͨ�Ź���
    int16_t   Rs485_1_Fault;          //RS485_1ͨ�Ź���
    int16_t   Rs485_2_Fault;          //RS485_2ͨ�Ź���
    int16_t   CAN_1_Fault;            //CAN_1ͨ�Ź���
    int16_t   CAN_2_Fault;            //CAN_2ͨ�Ź���
    int16_t   EE_Fault;               //EEPROM����
    int16_t   Sflash_Fault;           //spi flashͨ�Ź���
    int16_t   PtpSynced;              //0:δͬ��,1:��ͬ��
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
