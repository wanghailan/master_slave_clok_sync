//#############################################################################
//
// FILE:   Drv_ADC.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

//#include <Driver/device/device.h>
//#include <Driver/device/driverlib.h>
#include "bsp.h"

IPC_DATA_CPU2CM         CmIpc_cpu2cm;
IPC_DATA_CM2CPU         CmIpc_cm2cpu;
LOCAL_PARAM_CM          CmLocalParam;

Store_PARAM         store_param[EE_PARAM_CNT] =
{//ModAddr      min     max     *g_param                                    eeAddr
 { 307,       8000,  12000,      &(CmIpc_cm2cpu.VgridA_Calibrat),           0},
 { 308,       8000,  12000,      &(CmIpc_cm2cpu.VgridB_Calibrat),           2},
 { 309,       8000,  12000,      &(CmIpc_cm2cpu.VgridC_Calibrat),           4},
 { 310,       8000,  12000,      &(CmIpc_cm2cpu.IInvA_Calibrat),            6},
 { 311,       8000,  12000,      &(CmIpc_cm2cpu.IInvB_Calibrat),            8},
 { 312,       8000,  12000,      &(CmIpc_cm2cpu.IInvC_Calibrat),           10},
 { 313,       8000,  12000,      &(CmIpc_cm2cpu.IGridA_Calibrat),          12},
 { 314,       8000,  12000,      &(CmIpc_cm2cpu.IGridB_Calibrat),          14},
 { 315,       8000,  12000,      &(CmIpc_cm2cpu.IGridC_Calibrat),          16},
 { 316,       8000,  12000,      &(CmIpc_cm2cpu.VdcP_Calibrat),            18},
 { 317,       8000,  12000,      &(CmIpc_cm2cpu.VdcN_Calibrat),            20},
 { 318,       8000,  12000,      &(CmIpc_cm2cpu.Idc_Calibrat),             22},
 { 319,       8000,  12000,      &(CmIpc_cm2cpu.Vdc_Calibrat),             24},
 { 320,       8000,  12000,      &(CmIpc_cm2cpu.Ref1v5_Calibrat),          26},
 { 321,       8000,  12000,      &(CmIpc_cm2cpu.VInvA_Calibrat),           28},
 { 322,       8000,  12000,      &(CmIpc_cm2cpu.VInvB_Calibrat),           30},
 { 323,       8000,  12000,      &(CmIpc_cm2cpu.VInvC_Calibrat),           32},
 { 324,       8000,  12000,      &(CmIpc_cm2cpu.IN_Calibrat),              34},
 { 208,          0,   5000,      &(CmIpc_cm2cpu.AC_OverVol_Value),         36},//交流电压过压保护阈值
 { 209,          0,   5000,      &(CmIpc_cm2cpu.AC_UnderVol_Value),        38},//交流电压欠压保护阈值
 { 210,          0,   3000,      &(CmIpc_cm2cpu.AC_OverCurr_Value),        40},//交流过流保护阈值
 { 211,          0,  11000,      &(CmIpc_cm2cpu.DC_OverVol_Value),         42},//直流过压保护阈值
 { 212,          0,   8000,      &(CmIpc_cm2cpu.DC_UnderVol_Value),        44},//直流欠压保护阈值
 { 213,          0,    800,      &(CmIpc_cm2cpu.TempAmb_Over_Value),       46},//环境过温保护阈值
 { 214,          0,   3000,      &(CmIpc_cm2cpu.AC_UnbanceVol_Value),      48},//交流电压不平衡保护阈值
 { 215,          0,   2000,      &(CmIpc_cm2cpu.DC_UnbanceVol_Value),      50}, //直流电压不平衡保护阈值
 { 101,          0,     2,       &(CmIpc_cm2cpu.WorkMode),                 52}, //工作模式
 { 102,          0,     4,       &(CmIpc_cm2cpu.OnGridMode),               54}, //并网工作模式
 { 103,          0,     1,       &(CmIpc_cm2cpu.LocalRemote),              56}, //本地远程模式
 { 104,      -1500,  1500,       &(CmIpc_cm2cpu.ActivePowerSet),           58}, //有功功率设定
 { 105,      -1000,  1000,       &(CmIpc_cm2cpu.ReActivePowerSet),         60}, //无功功率设定
 { 106,      -3000,  3000,       &(CmIpc_cm2cpu.DcConstCurrSet),           62}, //恒直流电流设定
 { 107,        100, 10000,       &(CmIpc_cm2cpu.DcConstVolSet),            64}, //恒直流电压设定
 { 108,      -3000,  3000,       &(CmIpc_cm2cpu.AcConstCurrSet),           66}, //恒交流电流设定
 { 109,          0, 10000,       &(CmIpc_cm2cpu.PowerFactorSet),           68}, //恒功率因数设定
 { 110,        50,   4500,       &(CmIpc_cm2cpu.VF_VoSet),                 70}, //离网交流线电压设定
 { 111,       4500,  6000,       &(CmIpc_cm2cpu.VF_FreqSet),               72}, //离网交流频率设定
 { 112,       5000, 12000,       &(CmIpc_cm2cpu.DcEqualChargeVol),         74}, //均充电压设定
 { 113,       1000, 12000,       &(CmIpc_cm2cpu.DcFloatChargeVol),         76}, //浮充电压设定
 { 114,          0,  3000,       &(CmIpc_cm2cpu.RatedCurrent),             78}, //额定电流设定
 { 115,          0,  1500,       &(CmIpc_cm2cpu.RatedPower),               80}, //额定功率设定
 { 116,          0,  2500,       &(CmIpc_cm2cpu.RatedVoltage),             82}, //额定电压设定
 { 117,       2000, 10000,       &(CmIpc_cm2cpu.BattHighVolSet),           84}, //电池电压保护上限设定
 { 118,       2000, 10000,       &(CmIpc_cm2cpu.DcEqual2FloatCurrent),     86}, //DC均充转浮充电流设定
 { 119,       2000, 10000,       &(CmIpc_cm2cpu.BattEODVol),               88}, //EOD终止放电电压
 { 120,          0, 1000,        &(CmIpc_cm2cpu.DODGridConnect),           90}, //并网DOD
 { 121,          0, 1000,        &(CmIpc_cm2cpu.DODDisGridConnect),        92},  //离网DOD
 { 216,          0, 3000,        &(CmIpc_cm2cpu.DC_OverCurr_Value),        94},  //直流过流保护阈值
 { 325,       8000,  12000,      &(CmIpc_cm2cpu.Idc_offset),               96},  //直流电流偏置校准
 { 326,       8000,  12000,      &(CmIpc_cm2cpu.Iac_Aoffset),              98},  //交流电流A偏置校准
 { 327,       8000,  12000,      &(CmIpc_cm2cpu.Iac_Boffset),             100},  //交流电流B偏置校准
 { 328,       8000,  12000,      &(CmIpc_cm2cpu.Iac_Coffset),             102},  //交流电流C偏置校准
 { 329,       8000,  12000,      &(CmIpc_cm2cpu.Ilac_Aoffset),            104},  //交流电感电流A偏置校准
 { 330,       8000,  12000,      &(CmIpc_cm2cpu.Ilac_Boffset),            106},  //交流电感电流B偏置校准
 { 331,       8000,  12000,      &(CmIpc_cm2cpu.Ilac_Coffset),            108},  //交流电感电流C偏置校准
 { 332,       8000,  12000,      &(CmIpc_cm2cpu.IdcN_offset),             110},  //中线电流偏置校准
 { 122,          0,    1,        &(CmIpc_cm2cpu.pcsLineMode),             112},  //接线体制
 { 217,          0,   1500,      &(CmIpc_cm2cpu.TempIgbt_Over_Value),     114}   //IGBT过温故障阈值
};
static void EEParam_Init(void)
{
    uint8_t param_buf[(EE_PARAM_CNT+2)*2];
    int16_t TempregData;
    int16_t index;
    uint8_t reg_buf[8]={0};

    CmLocalParam.EEparamCheckOK = 0;
    if(1==EE_ReadBuf(param_buf,0, EE_PARAM_CNT*2))
    {
        //校准参数
        for(index=0;index<(EE_PARAM_CNT*2);index=index+2)
        {
            TempregData      = (int16_t)BEBufToUint16(param_buf[index+1],param_buf[index]);
            if((TempregData >= store_param[index/2].min)&&(TempregData <= store_param[index/2].max))
            {
                *(store_param[index/2].g_param) = TempregData;
            }
            else
            {
                *(store_param[index/2].g_param) = store_param[index/2].min;
                CmLocalParam.EEparamCheckOK++;
            }
        }
    }
    if(CmLocalParam.EEparamCheckOK == 0)
        CmIpc_cm2cpu.EE_Fault = 0;
    else
        CmIpc_cm2cpu.EE_Fault = 1;
    EE_ReadBuf(reg_buf,EE_ADDR_IP1, 8);
    if((reg_buf[0] == 255)&&(reg_buf[1] == 255)&&(reg_buf[2] == 255)&&(reg_buf[3] == 255))
    {
        CmLocalParam.pIpAddr1[0] = 192;
        CmLocalParam.pIpAddr1[1] = 168;
        CmLocalParam.pIpAddr1[2] = 0;
        CmLocalParam.pIpAddr1[3] = 4;
        EE_WriteBuf((uint8_t *)&CmLocalParam.pIpAddr1[0],EE_ADDR_IP1, 4);
    }
    else
    {
        CmLocalParam.pIpAddr1[0] = reg_buf[0];
        CmLocalParam.pIpAddr1[1] = reg_buf[1];
        CmLocalParam.pIpAddr1[2] = reg_buf[2];
        CmLocalParam.pIpAddr1[3] = reg_buf[3];
    }
    if((reg_buf[4] == 255)&&(reg_buf[5] == 255)&&(reg_buf[6] == 255)&&(reg_buf[7] == 255))
    {
        CmLocalParam.pIpAddr2[0] = 192;
        CmLocalParam.pIpAddr2[1] = 168;
        CmLocalParam.pIpAddr2[2] = 0;
        CmLocalParam.pIpAddr2[3] = 10;
        EE_WriteBuf((uint8_t *)&CmLocalParam.pIpAddr2[0],EE_ADDR_IP2, 4);
    }
    else
    {
        CmLocalParam.pIpAddr2[0] = reg_buf[4];
        CmLocalParam.pIpAddr2[1] = reg_buf[5];
        CmLocalParam.pIpAddr2[2] = reg_buf[6];
        CmLocalParam.pIpAddr2[3] = reg_buf[7];
    }
    EE_ReadBuf(reg_buf,EE_ADDR_CHARGE, 8);
    CmLocalParam.PCS_MChargeKWH       = BEBufToUint16(reg_buf[1],reg_buf[0]);
    CmLocalParam.PCS_LChargeKWH       = BEBufToUint16(reg_buf[3],reg_buf[2]);
    CmLocalParam.PCS_MdisChargeKWH    = BEBufToUint16(reg_buf[5],reg_buf[4]);
    CmLocalParam.PCS_LdisChargeKWH    = BEBufToUint16(reg_buf[7],reg_buf[6]);
    CmLocalParam.PCS_RUN_TimeM        = 0;

}

void GloabParam_Init(void);
//#############################################################################
//
// FILE:   bsp.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

void bsp_init(void)
{
    Drv_Led_Init();
    Drv_TimerInt_Init();
    Drv_I2CA_Init();
    rtc_init();
    GloabParam_Init();
    ipc_init();
}

void Cm_Fault_Reset(void)
{
    CmIpc_cm2cpu.IpcCpu2Cm_Fault = 0;
    CmIpc_cm2cpu.CAN_1_Fault = 0;
    CmIpc_cm2cpu.CAN_2_Fault = 0;
    CmIpc_cm2cpu.Rs485_1_Fault = 0;
    CmIpc_cm2cpu.Rs485_2_Fault = 0;
    CmIpc_cm2cpu.EE_Fault = 0;
    CmIpc_cm2cpu.Sflash_Fault = 0;
    CmIpc_cm2cpu.SelfCheckOK = 0;
}

void Ptp_SetSynced(int16_t synced)
{
    CmIpc_cm2cpu.PtpSynced = (synced != 0) ? 1 : 0;
}

void GloabParam_Init(void)
{
    Cm_Fault_Reset();
    CmIpc_cm2cpu.Reset_En  = 0;
    CmIpc_cm2cpu.debugData_TxEn  = 0;
    CmIpc_cm2cpu.ClearFault  = 0;
    CmIpc_cm2cpu.OutLoop_En = 1;
    CmIpc_cm2cpu.SoftStart_En = 1;
    CmIpc_cm2cpu.Pwm_StartEn = 0;
    CmIpc_cm2cpu.StartEn = 0;
    CmIpc_cm2cpu.RelayCtrl_DCSoft = 0;
    CmIpc_cm2cpu.RelayCtrl_DCMaster = 0;
    CmIpc_cm2cpu.RelayCtrl_ACSoft = 0;
    CmIpc_cm2cpu.RelayCtrl_ACMaster = 0;
    CmIpc_cm2cpu.RelayCtrl_FanCtrl = 0;
    CmIpc_cm2cpu.RelayCtrl_WorkLed = 0;
    CmIpc_cm2cpu.RelayCtrl_FaultLed = 0;
    CmIpc_cm2cpu.FanCtrl_duty = 0;
    CmIpc_cm2cpu.PtpSynced = 0;
    EEParam_Init();
}

int8_t  i8_IntCnt = 0;
void Pcs_CmRunISR(void)
{
    switch(i8_IntCnt)
    {
        case 0:
            i8_IntCnt = 1;
            break;
        case 1:
            i8_IntCnt = 2;
            break;
        case 2:
            i8_IntCnt = 3;
            if((CmIpc_cpu2cm.PcsOnAllowed == 0)&&(CmIpc_cpu2cm.PcsCtrlState==7)&&(CmIpc_cpu2cm.Pwm_StartEn==1))
            {
//                CmIpc_cm2cpu.ActivePowerSet;         //有功功率设定
//                CmIpc_cm2cpu.ReActivePowerSet;       //无功功率设定
//                CmIpc_cm2cpu.DcConstCurrSet;         //恒直流电流设定
//                CmIpc_cm2cpu.DcConstVolSet;          //恒直流电压设定
//                CmIpc_cm2cpu.AcConstCurrSet;         //恒交流电流设定
//                CmIpc_cm2cpu.PowerFactorSet;         //功率因数设定
//                CmIpc_cm2cpu.VF_VoSet;               //VF输出电压设置
//                CmIpc_cm2cpu.VF_FreqSet;             //VF输出频率设置
                CmIpc_cm2cpu.Pwm_StartEn = 0;
            }
            break;
        default:
            i8_IntCnt = 0;
            break;
    }
}


/************************************************************************************
函数名称：    obj_Output_KWHDeal()
功能描述:     输出电能计算
************************************************************************************/
//
void  Output_KWHDeal(void)
{
    uint32_t TempPout_Bat = 0;
    uint8_t  reg_buf[8]={0};
    int16_t  ChargeData_flag = 0;
    static int32_t i32RecInkWS_Cnt = 0;//直流功率计数

    if(m_st_TimerFlag.u16_b1s == 1)
    {
        if(CmIpc_cpu2cm.PCS_DC_Power > 0)//直流放电，电流从直流到交流
        {
            TempPout_Bat = (uint32_t)(CmIpc_cpu2cm.PCS_DC_Power*10);  //电池侧功率 0.001KW
            i32RecInkWS_Cnt += TempPout_Bat;
            if(i32RecInkWS_Cnt >= 3600000)//1度电 3600000 3600S*1000W
            {
                i32RecInkWS_Cnt -= 3600000;
                ChargeData_flag = 1;
                CmLocalParam.PCS_LdisChargeKWH++;
                if(CmLocalParam.PCS_LdisChargeKWH >= (10000-1))
                {
                    CmLocalParam.PCS_LdisChargeKWH = 0;
                    CmLocalParam.PCS_MdisChargeKWH++;
                    if(CmLocalParam.PCS_MdisChargeKWH > 65535)
                    {
                        CmLocalParam.PCS_MdisChargeKWH = 0;
                        CmLocalParam.PCS_LdisChargeKWH = 0;
                    }
                }
            }
        }
        else//直流充电，电流从交流到直流
        {
            TempPout_Bat = (uint32_t)(abs(CmIpc_cpu2cm.PCS_DC_Power)*10);  //电池侧功率 0.001KW
            i32RecInkWS_Cnt += TempPout_Bat;
            if(i32RecInkWS_Cnt >= 3600000)//1度电 3600000 3600S*1000W
            {
                i32RecInkWS_Cnt -= 3600000;
                ChargeData_flag = 1;
                CmLocalParam.PCS_LChargeKWH++;
                if(CmLocalParam.PCS_LChargeKWH >= (10000-1))
                {
                    CmLocalParam.PCS_LChargeKWH = 0;
                    CmLocalParam.PCS_MChargeKWH++;
                    if(CmLocalParam.PCS_MChargeKWH > 65535)
                    {
                        CmLocalParam.PCS_MChargeKWH = 0;
                        CmLocalParam.PCS_LChargeKWH = 0;
                    }
                }
            }
        }

        if(ChargeData_flag == 1)
        {
            reg_buf[0] = CmLocalParam.PCS_MChargeKWH&0x0ff;
            reg_buf[1] = (CmLocalParam.PCS_MChargeKWH>>8)&0x0ff;
            reg_buf[2] = CmLocalParam.PCS_LChargeKWH&0x0ff;
            reg_buf[3] = (CmLocalParam.PCS_LChargeKWH>>8)&0x0ff;
            reg_buf[4] = CmLocalParam.PCS_MdisChargeKWH&0x0ff;
            reg_buf[5] = (CmLocalParam.PCS_MdisChargeKWH>>8)&0x0ff;
            reg_buf[6] = CmLocalParam.PCS_LdisChargeKWH&0x0ff;
            reg_buf[7] = (CmLocalParam.PCS_LdisChargeKWH>>8)&0x0ff;
            EE_WriteBuf(reg_buf,EE_ADDR_CHARGE,8);
        }
    }
}

/************************************************************************************
函数名称：    obj_Output_RunTimeCnt()
功能描述:     累计运行时间计算
************************************************************************************/
//
void  Output_RunTimeCnt(void)
{
    static  int16_t runCnt = 0;

    if(m_st_TimerFlag.u16_b1s == 1)
    {
        if(CmIpc_cm2cpu.Pwm_StartEn == 1)
            runCnt++;
        if(runCnt > 59)
        {
            runCnt = 0;
            CmLocalParam.PCS_RUN_TimeM++;
            if(CmLocalParam.PCS_RUN_TimeM > 65535)
                CmLocalParam.PCS_RUN_TimeM = 0;
        }
    }
}

/*
*********************************************************************************************************
*   函 数 名: BEBufToUint16
*   功能说明: 将2字节数组(大端Big Endian次序，高字节在前)转换为16位整数
*   形    参: _pBuf : 数组
*   返 回 值: 16位整数值
*
*   大端(Big Endian)与小端(Little Endian)
*********************************************************************************************************
*/
uint16_t BEBufToUint16(uint16_t _pBufM,uint16_t _pBufL)
{
    return (((uint16_t)_pBufM << 8) | _pBufL);
}






