/*
 * modbus_Param.c
 *
 *  Created on: 2025年6月24日
 *      Author: guowei
 */

#include <stdio.h>
#include <string.h>
#include "modbus_Param.h"
#include "ipc.h"
#include "bsp.h"


uint16_t   status_RegHoldingBuf_w[REG_STATUS_HOLDING_LEN];//状态寄存器
uint16_t   control_RegHoldingBuf_w[REG_CONTROL_HOLDING_LEN];//控制寄存器
uint16_t   fault_RegHoldingBuf_w[REG_FAULT_HOLDING_LEN];//故障寄存器
uint16_t   param_RegHoldingBuf_w[REG_PARAM_HOLDING_LEN];//参数寄存器
uint16_t   debug_RegHoldingBuf_w[REG_DEBUG_HOLDING_LEN];//功能使能寄存器

static int16_t status_upDataHoldingCBReg(void);
static int16_t Ctrl_upDataHoldingCBReg(void);
static int16_t Fault_upDataHoldingCBReg(void);
static int16_t Param_upDataHoldingCBReg(void);
static int16_t debug_upDataHoldingCBReg(void);


//状态寄存器数据更新 0~32
static int16_t status_upDataHoldingCBReg(void)
{
    //A相相电压(0.1V)
    status_RegHoldingBuf_w[0]    =  CmIpc_cpu2cm.PCS_AC_VRms_PhaseA;
    //AB线电压(0.1V)
    status_RegHoldingBuf_w[1]    =  CmIpc_cpu2cm.PCS_AC_VRms_LineAB;
    //A相电网频率(0.01HZ)
    status_RegHoldingBuf_w[2]    =  CmIpc_cpu2cm.PCS_AC_Freq_A;
    //A相功率因数(0.01)
    status_RegHoldingBuf_w[3]    =  CmIpc_cpu2cm.PCS_AC_PF_A;
    //A相输出电流(0.1A)
    status_RegHoldingBuf_w[4]    =  CmIpc_cpu2cm.PCS_AC_IRms_A;
    //A相有功功率(0.01KW)
    status_RegHoldingBuf_w[5]    =  CmIpc_cpu2cm.PCS_AC_PowerP_A;
    //A相无功功率(0.01KVar)
    status_RegHoldingBuf_w[6]    =  CmIpc_cpu2cm.PCS_AC_PowerQ_A;
    //A相视在功率(0.01KVA)
    status_RegHoldingBuf_w[7]    =  CmIpc_cpu2cm.PCS_AC_PowerS_A;

    //B相相电压(0.1V)
    status_RegHoldingBuf_w[8]    =  CmIpc_cpu2cm.PCS_AC_VRms_PhaseB;
    //BC线电压(0.1V)
    status_RegHoldingBuf_w[9]    =  CmIpc_cpu2cm.PCS_AC_VRms_LineBC;
    //B相电网频率(0.01HZ)
    status_RegHoldingBuf_w[10]   =  CmIpc_cpu2cm.PCS_AC_Freq_B;
    //B相功率因数(0.01)
    status_RegHoldingBuf_w[11]   =  CmIpc_cpu2cm.PCS_AC_PF_B;
    //B相输出电流(0.1A)
    status_RegHoldingBuf_w[12]   =  CmIpc_cpu2cm.PCS_AC_IRms_B;
    //B相有功功率(0.01KW)
    status_RegHoldingBuf_w[13]   =  CmIpc_cpu2cm.PCS_AC_PowerP_B;
    //B相无功功率(0.01KVar)
    status_RegHoldingBuf_w[14]   =  CmIpc_cpu2cm.PCS_AC_PowerQ_B;
    //B相视在功率(0.01KVA)
    status_RegHoldingBuf_w[15]   =  CmIpc_cpu2cm.PCS_AC_PowerS_B;

    //C相相电压(0.1V)
    status_RegHoldingBuf_w[16]   =  CmIpc_cpu2cm.PCS_AC_VRms_PhaseC;
    //CA线电压(0.1V)
    status_RegHoldingBuf_w[17]   =  CmIpc_cpu2cm.PCS_AC_VRms_LineCA;
    //C相电网频率(0.01HZ)
    status_RegHoldingBuf_w[18]   =  CmIpc_cpu2cm.PCS_AC_Freq_C;
    //C相功率因数(0.01)
    status_RegHoldingBuf_w[19]   =  CmIpc_cpu2cm.PCS_AC_PF_C;
    //C相输出电流(0.1A)
    status_RegHoldingBuf_w[20]   =  CmIpc_cpu2cm.PCS_AC_IRms_C;
    //C相有功功率(0.01KW)
    status_RegHoldingBuf_w[21]   =  CmIpc_cpu2cm.PCS_AC_PowerP_C;
    //C相无功功率(0.01KVar)
    status_RegHoldingBuf_w[22]   =  CmIpc_cpu2cm.PCS_AC_PowerQ_C;
    //C相视在功率(0.01KVA)
    status_RegHoldingBuf_w[23]   =  CmIpc_cpu2cm.PCS_AC_PowerS_C;

    //三相总有功功率(0.01KW)
    status_RegHoldingBuf_w[24]   =  CmIpc_cpu2cm.PCS_AC_PowerP;
    //三相总无功功率(0.01KVar)
    status_RegHoldingBuf_w[25]   =  CmIpc_cpu2cm.PCS_AC_PowerQ;
    //三相总视在功率(0.01KVA)
    status_RegHoldingBuf_w[26]   =  CmIpc_cpu2cm.PCS_AC_PowerS;
    //充电电量(万度)
    status_RegHoldingBuf_w[27]   =  CmLocalParam.PCS_MChargeKWH;
    //充电电量(KWh)
    status_RegHoldingBuf_w[28]   =  CmLocalParam.PCS_LChargeKWH;

    //直流电压(0.1V)
    status_RegHoldingBuf_w[29]   =  CmIpc_cpu2cm.PCS_DC_BusVol;
    //直流电流(0.1A)
    status_RegHoldingBuf_w[30]   =  CmIpc_cpu2cm.PCS_DC_BusCurr;
    //直流功率(0.01KW)
    status_RegHoldingBuf_w[31]   =  CmIpc_cpu2cm.PCS_DC_Power;
    //放电电量(万KWh)
    status_RegHoldingBuf_w[32]   =  CmLocalParam.PCS_MdisChargeKWH;
    //放电电量(KWh)
    status_RegHoldingBuf_w[33]   =  CmLocalParam.PCS_LdisChargeKWH;
    //正母线电压(0.1V)
    status_RegHoldingBuf_w[34]   =  CmIpc_cpu2cm.PCS_DC_PBusVol;
    //负母线电压(0.1V)
    status_RegHoldingBuf_w[35]   =  CmIpc_cpu2cm.PCS_DC_NBusVol;
    //IGBT温度最大值(0.1℃)
    status_RegHoldingBuf_w[36]   =  CmIpc_cpu2cm.Temp_igbtMax;
    //离网自生成信号锁相环频率
    status_RegHoldingBuf_w[37]   =  CmIpc_cpu2cm.Pcs_Localpllfo;
    //电网信号锁相环频率
    status_RegHoldingBuf_w[38]   =  CmIpc_cpu2cm.Pcs_Gridpllfo;
    //PCS状态机状态
    status_RegHoldingBuf_w[39]   =  CmIpc_cpu2cm.PcsCtrlState;
    //累计运行时间
    status_RegHoldingBuf_w[40]   =  CmLocalParam.PCS_RUN_TimeM;
    //A相IGBT温度
    status_RegHoldingBuf_w[41]   =  CmIpc_cpu2cm.Temp_igbtA;
    //B相IGBT温度
    status_RegHoldingBuf_w[42]   =  CmIpc_cpu2cm.Temp_igbtB;
    //C相IGBT温度
    status_RegHoldingBuf_w[43]   =  CmIpc_cpu2cm.Temp_igbtC;
    //N相IGBT温度
    status_RegHoldingBuf_w[44]   =  CmIpc_cpu2cm.Temp_igbtN;
    //进风口温度
    status_RegHoldingBuf_w[45]   =  CmIpc_cpu2cm.Temp_AmbInlet;
    //出风口温度
    status_RegHoldingBuf_w[46]   =  CmIpc_cpu2cm.Temp_AmbOutlet;
    //A相电感电流
    status_RegHoldingBuf_w[47]   =  CmIpc_cpu2cm.PCS_AC_ILRms_A;
    //B相电感电流
    status_RegHoldingBuf_w[48]   =  CmIpc_cpu2cm.PCS_AC_ILRms_B;
    //C相电感电流
    status_RegHoldingBuf_w[49]   =  CmIpc_cpu2cm.PCS_AC_ILRms_C;
    //N线电流
    status_RegHoldingBuf_w[50]   =  CmIpc_cpu2cm.PCS_DC_IBUS_N;
    //降额运行比
    status_RegHoldingBuf_w[51]   =  CmIpc_cpu2cm.DerateRatio;//%
    //PWM控制比
    status_RegHoldingBuf_w[52]   =  CmIpc_cpu2cm.PwmRatio;//%
    //相位差
    status_RegHoldingBuf_w[53]   =  CmIpc_cpu2cm.pllPhaseErr;//0.1度
    return 0;
}

//控制寄存器数据更新 100~123
static int16_t Ctrl_upDataHoldingCBReg(void)
{
    //启动(1)/停机(0)
    control_RegHoldingBuf_w[0]   = CmIpc_cm2cpu.StartEn;
    //离网(0)/并网(1)
    control_RegHoldingBuf_w[1]   = CmIpc_cm2cpu.WorkMode;
    //并网工作模式(0:恒功率,1:恒流,2:恒直流电压,3:恒直流电流,4:功率因数控制)
    control_RegHoldingBuf_w[2]   = CmIpc_cm2cpu.OnGridMode;
    //本地(0)/远程(1)
    control_RegHoldingBuf_w[3]   = CmIpc_cm2cpu.LocalRemote;
    //有功功率值设定(-150~150KW)
    control_RegHoldingBuf_w[4]   = CmIpc_cm2cpu.ActivePowerSet;
    //无功功率值设定(-100~100KVar)
    control_RegHoldingBuf_w[5]   = CmIpc_cm2cpu.ReActivePowerSet;
    //恒直流电流设定(-250~250A)
    control_RegHoldingBuf_w[6]   = CmIpc_cm2cpu.DcConstCurrSet;
    //恒直流电压设定(600~1000V)
    control_RegHoldingBuf_w[7]   = CmIpc_cm2cpu.DcConstVolSet;
    //恒交流电流设定(-250~250A)
    control_RegHoldingBuf_w[8]   = CmIpc_cm2cpu.AcConstCurrSet;
    //功率因数设定(-1~1)
    control_RegHoldingBuf_w[9]   = CmIpc_cm2cpu.PowerFactorSet;
    //VF电压设定(0~1000.0V)
    control_RegHoldingBuf_w[10]  = CmIpc_cm2cpu.VF_VoSet;
    //VF频率设定(0~60.00HZ)
    control_RegHoldingBuf_w[11]  = CmIpc_cm2cpu.VF_FreqSet;
    //均充电压设定(0~1000.0V)
    control_RegHoldingBuf_w[12]  = CmIpc_cm2cpu.DcEqualChargeVol;
    //浮充电压设定(0~1000.0V)
    control_RegHoldingBuf_w[13]  = CmIpc_cm2cpu.DcFloatChargeVol;
    //额定电流
    control_RegHoldingBuf_w[14]  = CmIpc_cm2cpu.RatedCurrent;
    //额定功率
    control_RegHoldingBuf_w[15]  = CmIpc_cm2cpu.RatedPower;
    //额定电压
    control_RegHoldingBuf_w[16]  = CmIpc_cm2cpu.RatedVoltage;
    //
    control_RegHoldingBuf_w[17]  = 0;
    //
    control_RegHoldingBuf_w[18]  = CmIpc_cm2cpu.DcEqual2FloatCurrent;
    //EOD(终止放电)电压(200.0~1000.0V)
    control_RegHoldingBuf_w[19]  = CmIpc_cm2cpu.BattEODVol;
    //并网DOD(放电深度)(0~100.0%)
    control_RegHoldingBuf_w[20]  = CmIpc_cm2cpu.DODGridConnect;
    //离网DOD(放电深度)(0~100.0%)
    control_RegHoldingBuf_w[21]  = CmIpc_cm2cpu.DODDisGridConnect;
    //接线体制(0~1)
    control_RegHoldingBuf_w[22]  = CmIpc_cm2cpu.pcsLineMode;
    //电量清零(0~1)
    control_RegHoldingBuf_w[23]  = 0;
    //故障清零(0~1)
    control_RegHoldingBuf_w[24]  = 0;
    //开始发波
    control_RegHoldingBuf_w[25]  = CmIpc_cpu2cm.Pwm_StartEn;
    return 0;
}

//故障处理寄存器数据更新 200~210
static int16_t Fault_upDataHoldingCBReg(void)
{
    //告警1
    fault_RegHoldingBuf_w[0]   = CmIpc_cpu2cm.FaultStatus.PCS_Waring1.PCS_Waring1_All;
    //告警2
    fault_RegHoldingBuf_w[1]   = CmIpc_cpu2cm.FaultStatus.PCS_Waring2.PCS_Waring2_All;
    //故障1
    fault_RegHoldingBuf_w[2]   = CmIpc_cpu2cm.FaultStatus.PCS_Fault1.PCS_Fault1_All;
    //故障2
    fault_RegHoldingBuf_w[3]   = CmIpc_cpu2cm.FaultStatus.PCS_Fault2.PCS_Fault2_All;
    //故障3
    fault_RegHoldingBuf_w[4]   = CmIpc_cpu2cm.FaultStatus.PCS_Fault3.PCS_Fault3_All;
    //故障4
    fault_RegHoldingBuf_w[5]   = CmIpc_cpu2cm.FaultStatus.PCS_Fault4.PCS_Fault4_All;
    //故障5
    fault_RegHoldingBuf_w[6]   = CmIpc_cpu2cm.FaultStatus.PCS_HFault.PCS_HFault_All;
    //总故障位
    fault_RegHoldingBuf_w[7]   = CmIpc_cpu2cm.FaultStatus.globalFault;
    //交流过压保护阈值(0~500V）
    fault_RegHoldingBuf_w[8]   = CmIpc_cm2cpu.AC_OverVol_Value;
    //交流欠压保护阈值(0~300V)
    fault_RegHoldingBuf_w[9]   = CmIpc_cm2cpu.AC_UnderVol_Value;
    //交流过流保护阈值
    fault_RegHoldingBuf_w[10]  = CmIpc_cm2cpu.AC_OverCurr_Value;
    //直流过压保护阈值(0~1000V)
    fault_RegHoldingBuf_w[11]  = CmIpc_cm2cpu.DC_OverVol_Value;
    //直流欠压保护阈值(0~1000V)
    fault_RegHoldingBuf_w[12]  = CmIpc_cm2cpu.DC_UnderVol_Value;
    //环境过温保护阈值
    fault_RegHoldingBuf_w[13]  = CmIpc_cm2cpu.TempAmb_Over_Value;
    //交流电压不平衡保护阈值
    fault_RegHoldingBuf_w[14]  = CmIpc_cm2cpu.AC_UnbanceVol_Value;
    //直流电压不平衡保护阈值
    fault_RegHoldingBuf_w[15]  = CmIpc_cm2cpu.DC_UnbanceVol_Value;
    //直流过流保护阈值
    fault_RegHoldingBuf_w[16]  = CmIpc_cm2cpu.DC_OverCurr_Value;
    //IGBT过温阈值
    fault_RegHoldingBuf_w[17]  = CmIpc_cm2cpu.TempIgbt_Over_Value;
    return 0;
}

//参数设置寄存器数据更新 300~328
static int16_t Param_upDataHoldingCBReg(void)
{
    //RTC时间  秒
    param_RegHoldingBuf_w[0]   = Str_RtcTime.seconds;
    //RTC时间  分
    param_RegHoldingBuf_w[1]   = Str_RtcTime.minutes;
    //RTC时间  时
    param_RegHoldingBuf_w[2]   = Str_RtcTime.hours;
    //RTC时间  日
    param_RegHoldingBuf_w[3]   = Str_RtcTime.day;
    //RTC时间  月
    param_RegHoldingBuf_w[4]   = Str_RtcTime.month;
    //RTC时间  年
    param_RegHoldingBuf_w[5]   = Str_RtcTime.year;
    //RTC时间  周
    param_RegHoldingBuf_w[6]   = Str_RtcTime.dweek;
    //AC电压GridA校准(80.00%~120.00%)
    param_RegHoldingBuf_w[7]   = CmIpc_cm2cpu.VgridA_Calibrat;
    //AC电压GridB校准
    param_RegHoldingBuf_w[8]   = CmIpc_cm2cpu.VgridB_Calibrat;
    //AC电压GridC校准
    param_RegHoldingBuf_w[9]   = CmIpc_cm2cpu.VgridC_Calibrat;
    //AC电流invA校准
    param_RegHoldingBuf_w[10]  = CmIpc_cm2cpu.IInvA_Calibrat;
    //AC电流invB校准
    param_RegHoldingBuf_w[11]  = CmIpc_cm2cpu.IInvB_Calibrat;
    //AC电流invC校准
    param_RegHoldingBuf_w[12]  = CmIpc_cm2cpu.IInvC_Calibrat;
    //AC电流GridA校准
    param_RegHoldingBuf_w[13]  = CmIpc_cm2cpu.IGridA_Calibrat;
    //AC电流GridB校准
    param_RegHoldingBuf_w[14]  = CmIpc_cm2cpu.IGridB_Calibrat;
    //AC电流GridC校准
    param_RegHoldingBuf_w[15]  = CmIpc_cm2cpu.IGridC_Calibrat;
    //DC电压P校准
    param_RegHoldingBuf_w[16]  = CmIpc_cm2cpu.VdcP_Calibrat;
    //DC电压N校准
    param_RegHoldingBuf_w[17]  = CmIpc_cm2cpu.VdcN_Calibrat;
    //DC电流校准
    param_RegHoldingBuf_w[18]  = CmIpc_cm2cpu.Idc_Calibrat;
    //DC电压校准
    param_RegHoldingBuf_w[19]  = CmIpc_cm2cpu.Vdc_Calibrat;
    //1.5V偏置电压校准
    param_RegHoldingBuf_w[20]  = CmIpc_cm2cpu.Ref1v5_Calibrat;
    //逆变电压A校准
    param_RegHoldingBuf_w[21]  = CmIpc_cm2cpu.VInvA_Calibrat;
    //逆变电压B校准
    param_RegHoldingBuf_w[22]  = CmIpc_cm2cpu.VInvB_Calibrat;
    //逆变电压C校准
    param_RegHoldingBuf_w[23]  = CmIpc_cm2cpu.VInvC_Calibrat;
    //N线电流校准
    param_RegHoldingBuf_w[24]  = CmIpc_cm2cpu.IN_Calibrat;
    //直流电流偏置校准
    param_RegHoldingBuf_w[25]  = CmIpc_cm2cpu.Idc_offset;
    //交流电流A偏置校准
    param_RegHoldingBuf_w[26]  = CmIpc_cm2cpu.Iac_Aoffset;
    //交流电流B偏置校准
    param_RegHoldingBuf_w[27]  = CmIpc_cm2cpu.Iac_Boffset;
    //交流电流C偏置校准
    param_RegHoldingBuf_w[28]  = CmIpc_cm2cpu.Iac_Coffset;
    //交流L电流A偏置校准
    param_RegHoldingBuf_w[29]  = CmIpc_cm2cpu.Ilac_Aoffset;
    //交流L电流B偏置校准
    param_RegHoldingBuf_w[30]  = CmIpc_cm2cpu.Ilac_Boffset;
    //交流L电流C偏置校准
    param_RegHoldingBuf_w[31]  = CmIpc_cm2cpu.Ilac_Coffset;
    //母线中点电流偏置校准
    param_RegHoldingBuf_w[32]  = CmIpc_cm2cpu.IdcN_offset;

    return 0;
}

//参数设置寄存器数据更新 400~422
static int16_t debug_upDataHoldingCBReg(void)
{
    //直流软起继电器
    debug_RegHoldingBuf_w[0]   = CmIpc_cpu2cm.Relay_DCSoft;
    //直流主继电器
    debug_RegHoldingBuf_w[1]   = CmIpc_cpu2cm.Relay_DCMaster;
    //交流软起继电器
    debug_RegHoldingBuf_w[2]   = CmIpc_cpu2cm.Relay_ACSoft;
    //交流主继电器
    debug_RegHoldingBuf_w[3]   = CmIpc_cpu2cm.Relay_ACMaster;
    //开环闭环模式
    debug_RegHoldingBuf_w[4]   = CmIpc_cm2cpu.OpenLoopMode;
    //调试模式
    debug_RegHoldingBuf_w[5]   = CmIpc_cm2cpu.debugMode;
    //风扇转速
    debug_RegHoldingBuf_w[6]   = CmIpc_cpu2cm.FanCtrl_duty;
    //1.5V电压
    debug_RegHoldingBuf_w[7]   = CmIpc_cpu2cm.Vol_1V5;
    //5V电压
    debug_RegHoldingBuf_w[8]   = CmIpc_cpu2cm.Vol_5V;
    //15V电压
    debug_RegHoldingBuf_w[9]   = CmIpc_cpu2cm.Vol_p15V;
    //风机电源继电器
    debug_RegHoldingBuf_w[10]  = CmIpc_cpu2cm.Relay_FanCtrl;
    //故障指示灯控制
    debug_RegHoldingBuf_w[11]  = CmIpc_cpu2cm.Relay_WorkLed;
    //运行指示灯控制
    debug_RegHoldingBuf_w[12]  = CmIpc_cpu2cm.Relay_FaultLed;
    //软复位
    debug_RegHoldingBuf_w[13]  = 0;
    //软启动使能
    debug_RegHoldingBuf_w[14]  = CmIpc_cm2cpu.SoftStart_En;
    //电感电流内环Kp参数
    debug_RegHoldingBuf_w[15]  = CmIpc_cm2cpu.IlInter_Kp;
    //电感电流内环Ki参数
    debug_RegHoldingBuf_w[16]  = CmIpc_cm2cpu.IlInter_Ki;
    //AC电流外环Kp参数
    debug_RegHoldingBuf_w[17]  = CmIpc_cm2cpu.IAcOuter_Kp;
    //AC电流外环Ki参数
    debug_RegHoldingBuf_w[18]  = CmIpc_cm2cpu.IAcOuter_Ki;
    //DC电流外环Kp参数
    debug_RegHoldingBuf_w[19]  = CmIpc_cm2cpu.IDcOuter_Kp;
    //DC电流外环Ki参数
    debug_RegHoldingBuf_w[20]  = CmIpc_cm2cpu.IDcOuter_Ki;
    //AC电压外环Kp参数
    debug_RegHoldingBuf_w[21]  = CmIpc_cm2cpu.VAcOuter_Kp;
    //AC电压外环Ki参数
    debug_RegHoldingBuf_w[22]  = CmIpc_cm2cpu.VAcOuter_Ki;
    //DC电压外环Kp参数
    debug_RegHoldingBuf_w[23]  = CmIpc_cm2cpu.VDcOuter_Kp;
    //DC电压外环Ki参数
    debug_RegHoldingBuf_w[24]  = CmIpc_cm2cpu.VDcOuter_Ki;
    //
    debug_RegHoldingBuf_w[25]  = 0;
    //
    debug_RegHoldingBuf_w[26]  = 0;
    //NET1 ip地址
    debug_RegHoldingBuf_w[27]  = CmLocalParam.pIpAddr1[3];
    debug_RegHoldingBuf_w[28]  = CmLocalParam.pIpAddr1[2];
    debug_RegHoldingBuf_w[29]  = CmLocalParam.pIpAddr1[1];
    debug_RegHoldingBuf_w[30]  = CmLocalParam.pIpAddr1[0];
    //NET2 ip地址
    debug_RegHoldingBuf_w[31]  = CmLocalParam.pIpAddr2[3];
    debug_RegHoldingBuf_w[32]  = CmLocalParam.pIpAddr2[2];
    debug_RegHoldingBuf_w[33]  = CmLocalParam.pIpAddr2[1];
    debug_RegHoldingBuf_w[34]  = CmLocalParam.pIpAddr2[0];

    return 0;
}

//参数设置寄存器数据更新
int16_t Param_WriteDataHoldingReg(uint16_t reg,int16_t value)
{
    uint8_t reg_buf[2];
    uint16_t eeAddr;
    int16_t index=800;
    int16_t ret = 0;

    switch(reg)
    {
        case 100://启动使能
            if(value == 1)
            {
                if(CmIpc_cpu2cm.PcsOnAllowed == 1)//无故障允许开机时才能启动
                    CmIpc_cm2cpu.StartEn  = 1;
                else
                    CmIpc_cm2cpu.StartEn  = 0;
            }
            else
                CmIpc_cm2cpu.StartEn  = 0;
            break;
        case 101: index = 26; break;//工作模式
        case 102: index = 27; break;//并网工作模式
        case 103: index = 28; break;//本地远程
        case 104: index = 29; break;//有功功率设定
        case 105: index = 30; break;//无功功率设定
        case 106: index = 31; break;//恒直流电流设定
        case 107: index = 32; break;//恒直流电压设定
        case 108: index = 33; break;//恒交流电流设定
        case 109: index = 34; break;//恒功率因数设定
        case 110: index = 35; break;//离网交流电压设定
        case 111: index = 36; break;//离网交流频率设定
        case 112: index = 37; break;//均充电压设定
        case 113: index = 38; break;//浮充电压设定
        case 114: index = 39; break;//额定电流
        case 115: index = 40; break;//额定功率
        case 116: index = 41; break;//额定电压
//        case 117: index = 42; break;//电池电压保护上限
        case 118: index = 43; break;//均充转浮充电压设定
        case 119: index = 44; break;//EOD终止放电电压
        case 120: index = 45; break;//并网DOD(电池放电深度)
        case 121: index = 46; break;//离网DOD(电池放电深度)
        case 122: index = 56; break;//接线体制
        case 123://电量计数清零
            if(value == 1)
            {
                uint8_t reg_buf[8]={0};
                EE_WriteBuf(reg_buf,EE_ADDR_CHARGE, 8);
                CmLocalParam.PCS_MChargeKWH       = 0;
                CmLocalParam.PCS_LChargeKWH       = 0;
                CmLocalParam.PCS_MdisChargeKWH    = 0;
                CmLocalParam.PCS_LdisChargeKWH    = 0;
            }
            break;
        case 124://故障清除
             if(value == 1)
             {
                 Cm_Fault_Reset();
                 CmIpc_cm2cpu.ClearFault = 1;
             }
             break;
        case 125://发波使能
            if(value == 1)
            {
                if((CmIpc_cpu2cm.PcsOnAllowed == 1)&&(CmIpc_cpu2cm.PcsCtrlState == 7))
                    CmIpc_cm2cpu.Pwm_StartEn = 1;
                else
                    CmIpc_cm2cpu.Pwm_StartEn = 0;
            }
            else
                CmIpc_cm2cpu.Pwm_StartEn = 0;
           break;
         case 208: index = 18; break;//交流电压过压保护阈值
         case 209: index = 19; break;//交流电压欠压保护阈值
         case 210: index = 20; break;//交流过流保护阈值
         case 211: index = 21; break;//直流过压保护阈值
         case 212: index = 22; break;//直流欠压保护阈值
         case 213: index = 23; break;//环境过温保护阈值
         case 214: index = 24; break;//交流电压不平衡保护阈值
         case 215: index = 25; break;//直流电压不平衡保护阈值
         case 216: index = 47; break;//直流过流保护阈值
         case 217: index = 57; break;//IGBT过温故障阈值
         case 300: rtc_write_time(0,value); break;
         case 301: rtc_write_time(1,value); break;
         case 302: rtc_write_time(2,value); break;
         case 303: rtc_write_time(3,value); break;
         case 304: rtc_write_time(4,value); break;
         case 305: rtc_write_time(5,value); break;
         case 306: rtc_write_time(6,value); break;
         case 307: index = 0; break;
         case 308: index = 1; break;
         case 309: index = 2; break;
         case 310: index = 3; break;
         case 311: index = 4; break;
         case 312: index = 5; break;
         case 313: index = 6; break;
         case 314: index = 7; break;
         case 315: index = 8; break;
         case 316: index = 9; break;
         case 317: index = 10;break;
         case 318: index = 11;break;
         case 319: index = 12;break;
         case 320: index = 13;break;
         case 321: index = 14;break;
         case 322: index = 15;break;
         case 323: index = 16;break;
         case 324: index = 17;break;
         case 325: index = 48;break;//直流电流偏置校准
         case 326: index = 49;break;//交流电流A偏置校准
         case 327: index = 50;break;//交流电流B偏置校准
         case 328: index = 51;break;//交流电流C偏置校准
         case 329: index = 52;break;//交流电感电流A偏置校准
         case 330: index = 53;break;//交流电感电流B偏置校准
         case 331: index = 54;break;//交流电感电流C偏置校准
         case 332: index = 55;break;//中线电流偏置校准
         case 400://母线软起继电器 低压侧
            if(((value == 0)||(value == 1))&&(CmIpc_cm2cpu.debugMode))
                CmIpc_cm2cpu.RelayCtrl_DCSoft   = value;
            break;
         case 401://母线主继电器 低压侧
            if(((value == 0)||(value == 1))&&(CmIpc_cm2cpu.debugMode))
                CmIpc_cm2cpu.RelayCtrl_DCMaster = value;
            break;
         case 402://电池软起继电器 高压侧
            if(((value == 0)||(value == 1))&&(CmIpc_cm2cpu.debugMode))
                CmIpc_cm2cpu.RelayCtrl_ACSoft   = value;
            break;
        case 403://电池主继电器 高压侧
            if(((value == 0)||(value == 1))&&(CmIpc_cm2cpu.debugMode))
                CmIpc_cm2cpu.RelayCtrl_ACMaster = value;
            break;
        case 404://开环模式，只适用于离网模式
             if((value >= 0)&&(value <= 1)&&(CmIpc_cm2cpu.WorkMode == GRID_DISCONNECTED))
             {
                 CmIpc_cm2cpu.OpenLoopMode = value;
             }
              break;
        case 405://调试模式
            if(((value == 0)||(value == 1))&&(CmIpc_cm2cpu.StartEn == 0)&&(CmIpc_cm2cpu.Pwm_StartEn == 0))
            {
                CmIpc_cm2cpu.debugMode = value;
            }
            break;
        case 406://风扇转速控制
            if(((value > 0)&&(value <= 1000))&&(CmIpc_cm2cpu.debugMode))
                CmIpc_cm2cpu.FanCtrl_duty = value;
            break;
        case 410://风机电源控制
            if(((value == 0)||(value == 1))&&(CmIpc_cm2cpu.debugMode))
                CmIpc_cm2cpu.RelayCtrl_FanCtrl = value;
            break;
        case 411://运行指示灯
            if(((value == 0)||(value == 1))&&(CmIpc_cm2cpu.debugMode))
            {
                CmIpc_cm2cpu.RelayCtrl_WorkLed = value;
            }
            break;
        case 412://故障指示灯
            if(((value == 0)||(value == 1))&&(CmIpc_cm2cpu.debugMode))
            {
                CmIpc_cm2cpu.RelayCtrl_FaultLed = value;
            }
            break;
        case 413://系统软件复位
            if((value == 1)&&(CmIpc_cm2cpu.StartEn == 0)&&(CmIpc_cm2cpu.Pwm_StartEn == 0))
                CmIpc_cm2cpu.Reset_En = 1;
            break;
        case 414://软起动使能
            if((CmIpc_cm2cpu.StartEn == 0)&&(CmIpc_cm2cpu.Pwm_StartEn == 0))
                CmIpc_cm2cpu.SoftStart_En = value;
            break;
/*
        case 415: index = 33; break;//Buck电压外环Kp参数
        case 416: index = 34; break;//Buck电压外环Ki参数
        case 417: index = 35; break;//Buck电流内环Kp参数
        case 418: index = 36; break;//Buck电流内环Ki参数
        case 419: index = 41; break;//Buck电流外环Kp参数
        case 420: index = 42; break;//Buck电流外环Ki参数
        case 421: index = 43; break;//Boost电压外环Kp参数
        case 422: index = 44; break;//Boost电压外环Ki参数
        case 423: index = 45; break;//Boost电流内环Kp参数
        case 424: index = 46; break;//Boost电流内环Ki参数
        case 425: index = 47; break;//Boost电流外环Kp参数
        case 426: index = 48; break;//Boost电流外环Ki参数
*/
        case 427:
            if((value > 0)&&(value < 255))
            {
                EE_WriteBuf((uint8_t *)&value,EE_ADDR_IP1+3, 1);
                CmLocalParam.pIpAddr1[3] = value;
            }
            break;//ip1_3
        case 428:
            if((value >= 0)&&(value <= 255))
            {
                EE_WriteBuf((uint8_t *)&value,EE_ADDR_IP1+2, 1);
                CmLocalParam.pIpAddr1[2] = value;
            }
            break;//ip1_2
        case 429:
            if((value >= 0)&&(value <= 255))
            {
                EE_WriteBuf((uint8_t *)&value,EE_ADDR_IP1+1, 1);
                CmLocalParam.pIpAddr1[1] = value;
            }
            break;//ip1_1
        case 430:
            if((value >= 0)&&(value <= 255))
            {
                EE_WriteBuf((uint8_t *)&value,EE_ADDR_IP1, 1);
                CmLocalParam.pIpAddr1[0] = value;
            }
            break;//ip1_0
        case 431:
            if((value > 0)&&(value < 255))
            {
                EE_WriteBuf((uint8_t *)&value,EE_ADDR_IP2+3, 1);
                CmLocalParam.pIpAddr2[3] = value;
            }
            break;//ip1_3
        case 432:
            if((value >= 0)&&(value <= 255))
            {
                EE_WriteBuf((uint8_t *)&value,EE_ADDR_IP2+2, 1);
                CmLocalParam.pIpAddr2[2] = value;
            }
            break;//ip1_2
        case 433:
            if((value >= 0)&&(value <= 255))
            {
                EE_WriteBuf((uint8_t *)&value,EE_ADDR_IP2+1, 1);
                CmLocalParam.pIpAddr2[1] = value;
            }
            break;//ip1_1
        case 434:
            if((value >= 0)&&(value <= 255))
            {
                EE_WriteBuf((uint8_t *)&value,EE_ADDR_IP2, 1);
                CmLocalParam.pIpAddr2[0] = value;
            }
            break;//ip1_0
        default:ret = -1;break;
    }

    if(ret == 0)
    {
        if(reg == store_param[index].ModAddr)
        {
          eeAddr  = store_param[index].eeAddr;
          if((value>=store_param[index].min)&&(value<=store_param[index].max))
          {
              reg_buf[0] = value&0x0ff;
              reg_buf[1] = (value>>8)&0x0ff;
              *(store_param[index].g_param) = value;
              EE_WriteBuf(reg_buf,eeAddr, 2);
          }
          else
          {
              return -2;
          }
        }
        return 0;
    }
    else
        return -1;
}


int16_t  Mb03_RegAddrLookUp(uint16_t regAddr,uint16_t regNum,uint32_t *regAddrBegin)
{
    int16_t RspCode;
    int16_t  iRegIndex;

    if( ((int16_t)regAddr >= REG_STATUS_HOLDING_START)&&( regAddr + regNum <= REG_STATUS_HOLDING_START + REG_STATUS_HOLDING_LEN ))
    {
        /* 读取的数据个数要在范围内 */
        if (regNum > REG_STATUS_HOLDING_LEN)
        {
            RspCode = RSP_ERR_VALUE;                    /* 数据值域错误 */
        }
        else
        {
            RspCode = RSP_OK;
            iRegIndex    = (int16_t)( regAddr - REG_STATUS_HOLDING_START);
            *regAddrBegin = (uint32_t)&status_RegHoldingBuf_w[iRegIndex];
        }
    }
    else  if( ((int16_t)regAddr >= REG_CONTROL_HOLDING_START)&&( regAddr + regNum <= REG_CONTROL_HOLDING_START + REG_CONTROL_HOLDING_LEN ))
    {
        /* 读取的数据个数要在范围内 */
        if (regNum > REG_CONTROL_HOLDING_LEN)
        {
            RspCode = RSP_ERR_VALUE;                    /* 数据值域错误 */
        }
        else
        {
            RspCode = RSP_OK;
            iRegIndex    = (int16_t)( regAddr - REG_CONTROL_HOLDING_START);
            *regAddrBegin = (uint32_t)&control_RegHoldingBuf_w[iRegIndex];
        }
    }
    else  if( ((int16_t)regAddr >= REG_FAULT_HOLDING_START)&&( regAddr + regNum <= REG_FAULT_HOLDING_START + REG_FAULT_HOLDING_LEN ))
    {
        /* 读取的数据个数要在范围内 */
        if (regNum > REG_FAULT_HOLDING_LEN)
        {
            RspCode = RSP_ERR_VALUE;                    /* 数据值域错误 */
        }
        else
        {
            RspCode = RSP_OK;
            iRegIndex    = (int16_t)( regAddr - REG_FAULT_HOLDING_START);
            *regAddrBegin = (uint32_t)&fault_RegHoldingBuf_w[iRegIndex];
        }
    }
    else  if( ((int16_t)regAddr >= REG_PARAM_HOLDING_START)&&( regAddr + regNum <= REG_PARAM_HOLDING_START + REG_PARAM_HOLDING_LEN ))
    {
        /* 读取的数据个数要在范围内 */
        if (regNum > REG_PARAM_HOLDING_LEN)
        {
            RspCode = RSP_ERR_VALUE;                    /* 数据值域错误 */
        }
        else
        {
            RspCode = RSP_OK;
            iRegIndex    = (int16_t)( regAddr - REG_PARAM_HOLDING_START);
            *regAddrBegin = (uint32_t)&param_RegHoldingBuf_w[iRegIndex];
        }
    }
    else  if( ((int16_t)regAddr >= REG_DEBUG_HOLDING_START)&&( regAddr + regNum <= REG_DEBUG_HOLDING_START + REG_DEBUG_HOLDING_LEN ))
    {
        /* 读取的数据个数要在范围内 */
        if (regNum > REG_DEBUG_HOLDING_LEN)
        {
            RspCode = RSP_ERR_VALUE;                    /* 数据值域错误 */
        }
        else
        {
            RspCode = RSP_OK;
            iRegIndex    = (int16_t)( regAddr - REG_DEBUG_HOLDING_START);
            *regAddrBegin = (uint32_t)&debug_RegHoldingBuf_w[iRegIndex];
        }
    }
    else
    {
        RspCode = RSP_ERR_REG_ADDR;     /* 寄存器地址错误 */
    }

    return RspCode;
}



//保持寄存器数据更新
int16_t upDataHoldingCBReg(void)
{
    //状态寄存器数据更新
    status_upDataHoldingCBReg();
    Ctrl_upDataHoldingCBReg();
    Fault_upDataHoldingCBReg();
    Param_upDataHoldingCBReg();
    debug_upDataHoldingCBReg();
    return 0;
}


