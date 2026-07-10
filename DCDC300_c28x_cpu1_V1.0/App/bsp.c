//#############################################################################
//
// FILE:   Drv_ADC.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

/* CODEMAP_BSP_C
 * Role: board support and global parameter plumbing.
 * It initializes hardware, loads EEPROM parameters, resets global states and copies CPU parameters to CLA.
 */
#include <Driver/device/device.h>
#include <Driver/device/driverlib.h>
//#include <Modbus_RTU.h>
 const uint8_t key0[16]   = {0x10, 0x21, 0x32, 0x43, 0x54, 0x65, 0x76, 0x87,
                                   0x98, 0xa4, 0xba, 0xcb, 0xdc, 0xed, 0x7e, 0x9f};

 const uint8_t key1[16]   = {0xe0, 0xd1, 0xc2, 0xb3, 0xa4, 0x95, 0x86, 0x77,
                                   0x68, 0x59, 0x4a, 0x3b, 0x2c, 0x1d, 0x0e, 0x2f};
uint8_t  u8_UID_REGS[32];
uint8_t  u8_AES_READ[32];
float32_t MPPT_Duty = 0.5f;
float32_t Vdc_MPPT;
int16_t   Cnt = 0;

//GRAPH    Graph1;
//GRAPH    Graph2;
//GRAPH    Graph3;
//#############################################################################
//
// FILE:   bsp.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

#include "bsp.h"

/*硬锟斤拷锟芥本锟斤拷*/
asm(" .sect \"Hard_Version\" ");
asm(" .retain ");
asm(" .word 1 ");
/*锟斤拷锟斤拷锟斤拷姹撅拷锟?锟斤拷锟杰伙拷芄锟斤拷写锟戒动时锟睫革拷*/
asm(" .sect \"mSoft_Version\" ");
asm(" .retain ");
asm(" .word 0xA001 ");/*锟斤拷8位锟斤拷A0:锟斤拷锟叫版，B1:锟斤拷锟皆帮拷.锟斤拷8位锟芥本锟斤拷*/
/*锟斤拷锟斤拷影姹撅拷锟?锟斤拷锟斤拷锟斤拷一锟斤拷锟斤拷锟斤拷时锟睫革拷*/
asm(" .sect \"cSoft_Version\" ");
asm(" .retain ");
asm(" .word 2 ");
/*锟斤拷锟斤拷薅锟斤拷姹撅拷锟?bug锟睫革拷锟斤拷小锟侥讹拷时锟睫革拷*/
asm(" .sect \"rSoft_Version\" ");
asm(" .retain ");
asm(" .word 1 ");
/*锟斤拷锟斤拷姹撅拷锟斤拷锟斤拷锟斤拷锟?/
asm(" .sect \"yTime_Version\" ");
asm(" .retain ");
asm(" .word 2602 ");//2026锟斤拷2锟斤拷
/*锟斤拷锟斤拷姹撅拷锟斤拷锟斤拷锟绞?/
asm(" .sect \"dTime_Version\" ");
asm(" .retain ");
asm(" .word 2810 ");//16锟斤拷13时
asm(" .text ");

DCDC_Global_Param    DcDc_gParam;

TINV_CALIBRAT_PARAM   g_calibratParam;
St_DCDC_Fault     FaultStatus;

/* CODEMAP_STORE_PARAM
 * Modbus persistent parameter table.
 * Each row maps: Modbus address -> min/max -> RAM variable -> EEPROM address.
 */Store_PARAM         store_param[EE_PARAM_CNT] =
{//ModAddr      min     max     *g_param                                    eeAddr
 { 307,       8000,  12000,      &(g_calibratParam.VBus_Calibrat),                   0},
 { 308,       8000,  12000,      &(g_calibratParam.VBusS_Calibrat),                  2},
 { 309,       8000,  12000,      &(g_calibratParam.VBat_Calibrat),                   4},
 { 310,       8000,  12000,      &(g_calibratParam.VBatS_Calibrat),                  6},
// { 311,       8000,  12000,      &(g_calibratParam.VBatP_Calibrat),                  8},
// { 312,       8000,  12000,      &(g_calibratParam.VBatN_Calibrat),                 10},
 { 313,       8000,  12000,      &(g_calibratParam.IBus_Calibrat),                   8},
 { 314,       8000,  12000,      &(g_calibratParam.IBat_Calibrat),                  10},
 { 315,       8000,  12000,      &(g_calibratParam.IL1_Calibrat),                   12},
 { 316,       8000,  12000,      &(g_calibratParam.IL2_Calibrat),                   14},
 { 317,       8000,  12000,      &(g_calibratParam.IL3_Calibrat),                   16},
 { 318,       8000,  12000,      &(g_calibratParam.IBus_offset),                    18},
 { 319,       8000,  12000,      &(g_calibratParam.IBat_offset),                    20},
 { 320,       8000,  12000,      &(g_calibratParam.IL1_offset),                     22},
 { 321,       8000,  12000,      &(g_calibratParam.IL2_offset),                     24},
 { 322,       8000,  12000,      &(g_calibratParam.IL3_offset),                     26},
 { 327,       8000,  12000,      &(g_calibratParam.Ref1v5_Calibrat),                28},
 { 323,       8000,  12000,      &(g_calibratParam.VBat_offset),                    30},
 { 324,       8000,  12000,      &(g_calibratParam.VBatS_offset),                   32},
 { 216,          0,   2000,      &(DcDc_Protect_Threshold.Temp_Over_Value),         34},//锟斤拷锟铰憋拷锟斤拷锟斤拷值
 { 208,          0,  20000,      &(DcDc_Protect_Threshold.Bat_OverVol_Value),       36},//锟斤拷压锟斤拷压锟斤拷锟斤拷锟斤拷值
 { 209,          0,  20000,      &(DcDc_Protect_Threshold.Bat_UnderVol_Value),      38},//锟斤拷压欠压锟斤拷锟斤拷锟斤拷值
 { 210,          0,  20000,      &(DcDc_Protect_Threshold.Bus_OverVol_Value),       40},//锟斤拷压锟斤拷压锟斤拷锟斤拷锟斤拷值
 { 211,          0,  20000,      &(DcDc_Protect_Threshold.Bus_UnderVol_Value),      42},//锟斤拷压欠压锟斤拷锟斤拷锟斤拷值
 { 212,          0,   4500,      &(DcDc_Protect_Threshold.Bat_OverCurr_Value),      44},//锟斤拷压锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷值
 { 213,          0,   4500,      &(DcDc_Protect_Threshold.Bus_OverCurr_Value),      46},//锟斤拷压锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷值
 { 214,          0,   2000,      &(DcDc_Protect_Threshold.Vol_Unbance_Value),       48},//锟斤拷锟斤拷锟斤拷压锟斤拷平锟解保锟斤拷锟斤拷值
 { 215,          0,   1000,      &(DcDc_Protect_Threshold.Curr_Unbance_Value),      50}, //直锟斤拷锟斤拷压锟斤拷平锟解保锟斤拷锟斤拷值
 { 101,          0,     1,       &(DcDc_gParam.WorkMode),                           52}, //锟斤拷锟斤拷模式 buck/boost
 { 102,          0,     4,       &(DcDc_gParam.CtrlMode),                           54}, //锟斤拷锟斤拷模式 0:锟斤拷锟斤拷,1:锟斤拷压,2:锟斤拷锟斤拷,3:锟姐功锟斤拷,4:锟斤拷锟斤拷
 { 103,          0,     1,       &(DcDc_gParam.LocalRemote),                        56}, //锟斤拷锟斤拷远锟斤拷模式
 { 104,          1, 11000,       &(DcDc_gParam.DcConstVolSet),                      58}, //锟斤拷压模式 锟斤拷压锟借定
 { 105,          1,  2275,       &(DcDc_gParam.DcConstCurrSet),                     60}, //锟斤拷锟斤拷模式 锟斤拷锟斤拷锟借定
 { 106,          1,  1375,       &(DcDc_gParam.DcConstPowerSet),                    62}, //锟姐功锟斤拷模式 锟斤拷锟斤拷锟借定
 { 107,          1, 10000,       &(DcDc_gParam.DcConstResSet),                      64}, //锟斤拷锟斤拷模式 锟斤拷锟斤拷锟借定
 { 415,          0, 32000,       &(DcDc_gParam.VBuck_Kp),                           66}, //Buck锟斤拷压锟解环Kp锟斤拷锟斤拷
 { 416,          0, 32000,       &(DcDc_gParam.VBuck_Ki),                           68}, //Buck锟斤拷压锟解环Ki锟斤拷锟斤拷
 { 417,          0, 32000,       &(DcDc_gParam.IdcBuck_Kp),                         70}, //Buck锟斤拷锟斤拷锟节伙拷Kp锟斤拷锟斤拷
 { 418,          0, 32000,       &(DcDc_gParam.IdcBuck_Ki),                         72}, //Buck锟斤拷锟斤拷锟节伙拷Ki锟斤拷锟斤拷
 { 108,          0,  2730,       &(DcDc_gParam.DcBatMaxCurrent),                    74}, //锟斤拷压锟斤拷锟斤拷锟斤拷锟斤拷锟借定
 { 109,          0,  2730,       &(DcDc_gParam.DcBusMaxCurrent),                    76}, //锟斤拷压锟斤拷锟斤拷锟斤拷锟斤拷锟借定
 { 110,          0, 12100,       &(DcDc_gParam.DcBatMaxVol),                        78}, //锟斤拷压锟斤拷锟斤拷锟斤拷压锟借定
 { 111,          0, 11495,       &(DcDc_gParam.DcBusMaxVol),                        80}, //锟斤拷压锟斤拷锟斤拷锟斤拷压锟借定
 { 419,          0, 32000,       &(DcDc_gParam.IBuck_Kp),                           82}, //Buck锟斤拷锟斤拷锟解环Kp锟斤拷锟斤拷
 { 420,          0, 32000,       &(DcDc_gParam.IBuck_Ki),                           84}, //Buck锟斤拷锟斤拷锟解环Ki锟斤拷锟斤拷
 { 421,          0, 32000,       &(DcDc_gParam.VBoost_Kp),                          86}, //Boost锟斤拷压锟解环Kp锟斤拷锟斤拷
 { 422,          0, 32000,       &(DcDc_gParam.VBoost_Ki),                          88}, //Boost锟斤拷压锟解环Ki锟斤拷锟斤拷
 { 423,          0, 32000,       &(DcDc_gParam.IdcBoost_Kp),                        90}, //Boost锟斤拷锟斤拷锟节伙拷Kp锟斤拷锟斤拷
 { 424,          0, 32000,       &(DcDc_gParam.IdcBoost_Ki),                        92}, //Boost锟斤拷锟斤拷锟节伙拷Ki锟斤拷锟斤拷
 { 425,          0, 32000,       &(DcDc_gParam.IBoost_Kp),                          94}, //Boost锟斤拷锟斤拷锟解环Kp锟斤拷锟斤拷
 { 426,          0, 32000,       &(DcDc_gParam.IBoost_Ki),                          96}, //Boost锟斤拷锟斤拷锟解环Ki锟斤拷锟斤拷
 { 113,          0,     1,       &(DcDc_gParam.MPPT_En),                            98}, //MPPT使锟斤拷
 { 325,          0, 12000,       &(g_calibratParam.VBus_offset),                   100}, //锟斤拷压锟剿口碉拷压偏锟斤拷校准
 { 326,          0, 12000,       &(g_calibratParam.VBusS_offset),                  102}, //锟斤拷压锟斤拷锟斤拷锟窖蛊拷锟叫Ｗ?
 { 114,          0, 12000,      &(DcDc_gParam.MPPT_VdcMax),                        108}, //MPPT锟斤拷锟斤拷压
 { 115,          0, 12000,      &(DcDc_gParam.MPPT_VdcMin),                        106}, //MPPT锟斤拷小锟斤拷压
 { 331,       8000, 12000,      &(g_calibratParam.Ilac_Coffset),                   108},
 { 332,       8000, 12000,      &(g_calibratParam.IdcN_offset),                    110}
// { 122,          0,    1,        &(DcDc_gParam.pcsLineMode),                   112}  //锟斤拷锟斤拷锟斤拷锟斤拷
};


/* CODEMAP_EE_INIT
 * Load saved calibration/control/protection parameters from EEPROM.
 * Invalid values are replaced by the table minimum and EEparamCheckOK is incremented.
 */
static void EEParam_Init(void)
{
    uint16_t param_buf[EE_PARAM_CNT*2];
    int16_t TempregData;
    int16_t index;
    uint16_t reg_buf[8]={0};

    DcDc_gParam.EEparamCheckOK = 0;
    if(0==I2C_EE_ReadBuf(0, EE_PARAM_CNT*2, param_buf))
    {
        //校准锟斤拷锟斤拷
        for(index=0;index<EE_PARAM_CNT*2;index=index+2)
        {
            TempregData      = U16_DataChange(param_buf[index+1],param_buf[index]);
            if((TempregData >= store_param[index/2].min)&&(TempregData <= store_param[index/2].max))
            {
                *(store_param[index/2].g_param) = TempregData;
            }
            else
            {
                *(store_param[index/2].g_param) = store_param[index/2].min;
                DcDc_gParam.EEparamCheckOK++;
            }
        }
    }
    else
    {
        DcDc_gParam.EEparamCheckOK = 0;
    }
    I2C_EE_ReadBuf(EE_ADDR_CHARGE, 8, reg_buf);
    DC_OutMeter.DC_LdisChargeKWH     = U16_DataChange(reg_buf[1],reg_buf[0]);
    DC_OutMeter.DC_HdisChargeKWH     = U16_DataChange(reg_buf[3],reg_buf[2]);
    DC_OutMeter.DC_LChargeKWH        = U16_DataChange(reg_buf[5],reg_buf[4]);
    DC_OutMeter.DC_HChargeKWH        = U16_DataChange(reg_buf[7],reg_buf[6]);
    DC_OutMeter.DC_RUN_TimeM         = 0;

}


/* CODEMAP_CPU_TO_CLA
 * Main bridge from slow CPU logic to fast CLA control.
 * Handles debug-mode state forcing, PwmStartEnable gating, mode/control/gain/reference copy,
 * calibration scaling, limit copy and PWM-ratio feedback to Modbus variables.
 */
void cpu2claParam_Upgrade(void)
{
    static  int16_t  delayCnt = 0;
    if (DcDc_gParam.debugMode == 1)
    {
        DcDc_gParam.DcCtrlState = DcDcOn_Mode;
    }
    if(DcDc_gParam.DcCtrlState == DcDcOn_Mode)//
    {
        if(DcDc_gParam.Pwm_StartEn==1)
        {
            delayCnt = 0;
            tCpu2Cla.PwmStartEnable  =  1;
        }
        else
        {
            if(tCpu2Cla.PwmStartEnable == 1)
            {
                if(DcDc_gParam.CtrlMode == 2)//锟斤拷锟斤拷锟侥Ｊ?
                {
                    DcDc_gParam.DcConstCurrSet = 200;
                    if(m_st_TimerFlag.u16_b10ms == 1)
                        delayCnt++;
//                    if((DC_OutMeter.DC_IBus_Mean < 5.0f)&&(DC_OutMeter.DC_IBat_Mean < 5.0f))
                        if(delayCnt > 50)
                        {
                            tCpu2Cla.PwmStartEnable  =  0;
                            delayCnt = 0;
                        }
                }
                else if(DcDc_gParam.CtrlMode == 3)//锟姐功锟斤拷模式
                {
                    DcDc_gParam.DcConstPowerSet = 20;
                    if(m_st_TimerFlag.u16_b1ms == 1)
                        delayCnt++;
                    if(delayCnt > 10)
                    {
                        tCpu2Cla.PwmStartEnable  =  0;
                        delayCnt = 0;
                    }
                }
                else
                {
                    tCpu2Cla.PwmStartEnable  =  0;
                }
            }
            else
                delayCnt = 0;
        }
    }
    else
    {
        delayCnt = 0;
        tCpu2Cla.PwmStartEnable  =  0;
    }

    tCpu2Cla.WorkMode        = DcDc_gParam.WorkMode;
    tCpu2Cla.CtrlMode        = DcDc_gParam.CtrlMode;
    tCpu2Cla.OutLoop_En      = DcDc_gParam.OutLoop_En;
    tCpu2Cla.MPPT_En         = DcDc_gParam.MPPT_En;

    tCpu2Cla.VBus_Calibrat   = (float32_t)g_calibratParam.VBus_Calibrat*0.0001f;
    tCpu2Cla.VBusS_Calibrat  = (float32_t)g_calibratParam.VBusS_Calibrat*0.0001f;
    tCpu2Cla.VBat_Calibrat   = (float32_t)g_calibratParam.VBat_Calibrat*0.0001f;
    tCpu2Cla.VBatS_Calibrat  = (float32_t)g_calibratParam.VBatS_Calibrat*0.0001f;
    tCpu2Cla.VBatP_Calibrat  = (float32_t)g_calibratParam.VBatP_Calibrat*0.0001f;
    tCpu2Cla.VBatN_Calibrat  = (float32_t)g_calibratParam.VBatN_Calibrat*0.0001f;
    tCpu2Cla.IBus_Calibrat   = (float32_t)g_calibratParam.IBus_Calibrat*0.0001f;
    tCpu2Cla.IBat_Calibrat   = (float32_t)g_calibratParam.IBat_Calibrat*0.0001f;
    tCpu2Cla.IL1_Calibrat    = (float32_t)g_calibratParam.IL1_Calibrat*0.0001f;
    tCpu2Cla.IL2_Calibrat    = (float32_t)g_calibratParam.IL2_Calibrat*0.0001f;
    tCpu2Cla.IL3_Calibrat    = (float32_t)g_calibratParam.IL3_Calibrat*0.0001f;
    tCpu2Cla.Ref1v5_Calibrat = (float32_t)g_calibratParam.Ref1v5_Calibrat*0.0001f;
    tCpu2Cla.IBus_offset     = (float32_t)g_calibratParam.IBus_offset*0.0001f;
    tCpu2Cla.IBat_offset     = (float32_t)g_calibratParam.IBat_offset*0.0001f;
    tCpu2Cla.VDC_Bat_offset  = (float32_t)g_calibratParam.VBat_offset*0.0001f;
    tCpu2Cla.VDC_Bus_offset  = (float32_t)g_calibratParam.VBatS_offset*0.0001f;
    tCpu2Cla.VDC_BatS_offset = (float32_t)g_calibratParam.VBus_offset*0.0001f;
    tCpu2Cla.VDC_BusS_offset = (float32_t)g_calibratParam.VBusS_offset*0.0001f;
    tCpu2Cla.IL1_offset      = (float32_t)g_calibratParam.IL1_offset*0.0001f;
    tCpu2Cla.IL2_offset      = (float32_t)g_calibratParam.IL2_offset*0.0001f;
    tCpu2Cla.IL3_offset      = (float32_t)g_calibratParam.IL3_offset*0.0001f;

    if(DcDc_gParam.WorkMode == 0) //buck
    {
        tCpu2Cla.VOCtrl_Kp        = (float32_t)DcDc_gParam.VBuck_Kp*0.01f;
        tCpu2Cla.VOCtrl_Ki        = (float32_t)DcDc_gParam.VBuck_Ki*0.01f;
        tCpu2Cla.IOCtrl_Kp        = (float32_t)DcDc_gParam.IBuck_Kp*0.01f;
        tCpu2Cla.IOCtrl_Ki        = (float32_t)DcDc_gParam.IBuck_Ki*0.01f;
        tCpu2Cla.IICtrl_Kp        = (float32_t)DcDc_gParam.IdcBuck_Kp*0.01f;
        tCpu2Cla.IICtrl_Ki        = (float32_t)DcDc_gParam.IdcBuck_Ki*0.01f;
    }
    else if(DcDc_gParam.WorkMode == 1) //boost
    {
        tCpu2Cla.VOCtrl_Kp        = (float32_t)DcDc_gParam.VBoost_Kp*0.01f;
        tCpu2Cla.VOCtrl_Ki        = (float32_t)DcDc_gParam.VBoost_Ki*0.01f;
        tCpu2Cla.IOCtrl_Kp        = (float32_t)DcDc_gParam.IBoost_Kp*0.01f;
        tCpu2Cla.IOCtrl_Ki        = (float32_t)DcDc_gParam.IBoost_Ki*0.01f;
        tCpu2Cla.IICtrl_Kp        = (float32_t)DcDc_gParam.IdcBoost_Kp*0.01f;
        tCpu2Cla.IICtrl_Ki        = (float32_t)DcDc_gParam.IdcBoost_Ki*0.01f;
    }

//    if(DcDc_gParam.WorkMode == 0) //buck
//    {
//        if(DcDc_gParam.DcConstVolSet > DcDc_gParam.DcBusMaxVol)
//            DcDc_gParam.DcConstVolSet = DcDc_gParam.DcBusMaxVol;
//        if(DcDc_gParam.DcConstCurrSet > DcDc_gParam.DcBusMaxCurrent)
//            DcDc_gParam.DcConstCurrSet = DcDc_gParam.DcBusMaxCurrent;
//    }
//    tCpu2Cla.DcDc_VdcRef     = (float32_t)DcDc_gParam.DcConstVolSet*0.1f;
    if(DcDc_gParam.SoftStart_En == 0)
    {
        tCpu2Cla.DcDc_VdcRef     = (float32_t)DcDc_gParam.DcConstVolSet*0.1f;
        tCpu2Cla.DcDc_IdcRef     = (float32_t)DcDc_gParam.DcConstCurrSet*0.1f;
        tCpu2Cla.DcDc_PdcRef     = (float32_t)DcDc_gParam.DcConstPowerSet*100.0f;//Kw
        tCpu2Cla.DcDc_RdcRef     = (float32_t)DcDc_gParam.DcConstResSet*0.1f;
    }

    tCpu2Cla.DcBatMaxCurrent = (float32_t)DcDc_gParam.DcBatMaxCurrent*0.1f;
    tCpu2Cla.DcBusMaxCurrent = (float32_t)DcDc_gParam.DcBusMaxCurrent*0.1f;
    tCpu2Cla.DcBatMaxVol     = (float32_t)DcDc_gParam.DcBatMaxVol*0.1f;
    tCpu2Cla.DcBusMaxVol     = (float32_t)DcDc_gParam.DcBusMaxVol*0.1f;

    DcDc_gParam.PwmRatio1    =  (int16_t)(tCla2Cpu.PwmRatio1*10000.0f);
    DcDc_gParam.PwmRatio2    =  (int16_t)(tCla2Cpu.PwmRatio2*10000.0f);
    DcDc_gParam.PwmRatio3    =  (int16_t)(tCla2Cpu.PwmRatio3*10000.0f);
}

/* CODEMAP_FAULT_RESET
 * Clears most software fault words and allows startup again.
 * Note: DCDC_Fault4 reset is currently commented out, so reverse/short faults may remain latched.
 */
void App_AllFault_Reset(void)
{
    FaultStatus.DCDC_Waring1.DCDC_Waring1_All = 0;
    FaultStatus.DCDC_Waring2.DCDC_Waring2_All = 0;
    FaultStatus.DCDC_Fault1.DCDC_Fault1_All = 0;
    FaultStatus.DCDC_Fault2.DCDC_Fault2_All = 0;
    FaultStatus.DCDC_Fault3.DCDC_Fault3_All = 0;
//    FaultStatus.DCDC_Fault4.DCDC_Fault4_All = 0;
    FaultStatus.DCDC_HFault.DCDC_HFault_All = 0;
    FaultStatus.globalFault = 0;
    DcDc_gParam.DcOnAllowed = 1;
}

/* CODEMAP_GLOBAL_INIT
 * Power-on default state: faults cleared, EEPROM loaded, relays off, PWM off, StartEn=0, normal mode.
 */
void GloabParam_Init(void)
{
//    g_param.m_i16_ReSet = 0;
//    g_param.StartEnable = 0;
//    g_param.opCtrlRatio = 5000;//小锟斤拷锟斤拷锟?位
//    g_param.CtrlMode    = 0;//默锟较匡拷锟斤拷
//    g_param.RelayCtrl   = 0;
//    i32RecInkWS_Cnt     = 0;

//    Graph1.i = 0;Graph2.i = 0;Graph3.i = 0;
    App_AllFault_Reset();
    EEParam_Init();
//    if(DcDc_gParam.EEparamCheckOK > 0)
//        FaultStatus.PCS_Fault3.tbits.bEEpromFault = 1;

    DcDc_gParam.SelfCheckOK = DcDc_gParam.EEparamCheckOK;

    DcDc_gParam.DcCtrlState = PowerOn_Mode;
    DcDc_gParam.DcOnAllowed = 1;
    DcDc_gParam.RelaySt_DcBusSoft   = 0;
    DcDc_gParam.RelaySt_DcBatSoft   = 0;
    DcDc_gParam.RelaySt_DcBusMaster = 0;
    DcDc_gParam.RelaySt_DcBatMaster = 0;
    DcDc_gParam.RelaySt_FanCtrl = 0;
    DcDc_gParam.RelaySt_WorkLed = 0;
    DcDc_gParam.RelaySt_FaultLed = 0;
    DcDc_gParam.Pwm_StartEn = 0;
    DcDc_gParam.StartEn  = 0;
    DcDc_gParam.debugMode = 0;//默锟斤拷锟斤拷锟斤拷模式
    DC_OutMeter.DC_RUN_TimeM = 0;
    DcDc_gParam.SoftStart_En = 1;//默锟较达拷锟斤拷锟斤拷
    DcDc_gParam.OutLoop_En   = 1;//默锟较达拷锟解环
    tCpu2Cla.Vdc_MPP = 70.0f;
    MPPT_Duty = 0.5f;
    Vdc_MPPT = DC_OutMeter.DC_VBusS_Mean * 0.8f;
    objControl_RelayControl();
    cpu2claParam_Upgrade();
}

/* CODEMAP_BSP_INIT
 * Hardware initialization order: device/GPIO -> drivers -> PWM protection -> ADC/I2C/RTC/Modbus
 * -> CLB/PWM enable gate -> IPC -> CLA -> output meters.
 */
void bsp_init(void)
{
    // Initialize device clock and peripherals
    Device_init();

    // Boot CPU2 core
/*#ifdef _FLASH
    Device_bootCPU2(BOOTMODE_BOOT_TO_FLASH_SECTOR0);
#else
    Device_bootCPU2(BOOTMODE_BOOT_TO_M0RAM);
#endif*/
    //
    // Boot CM core
#ifdef _FLASH
    Device_bootCM(BOOTMODE_BOOT_TO_FLASH_SECTOR0);
#else
    Device_bootCM(BOOTMODE_BOOT_TO_S0RAM);
#endif
    //
    // Initialize GPIO and configure the GPIO pin as a push-pull output
    //
    Device_initGPIO();
    Drv_DinPin_Init();
    Drv_LedPin_Init();
    Drv_SCIAPin_Init();
    Drv_SCIBPin_Init();
    Drv_SCICPin_Init();
    Drv_SCIDPin_Init();
    Drv_I2CAPin_Init();
    Drv_PwmPin_Init();
    Drv_EtherNetPinInit();

    ASysCtl_enableTemperatureSensor();
    ASysCtl_lockTemperatureSensor();
    DEVICE_DELAY_US(500);

    // Initialize PIE and clear PIE registers. Disables CPU interrupts.
    Interrupt_initModule();
    Interrupt_initVectorTable();

    Drv_DO_Init();
    tca9535_io1_init();
    tca9535_io2_init();
    objControl_globalVarInit();
    Drv_SCIAInit();
    Drv_SCIBInit();
    Drv_SCICInit();
    Drv_SCIDInit();
    // Disable sync(Freeze clock to PWM as well). GTBCLKSYNC is applicable
    // only for multiple core devices. Uncomment the below statement if applicable.
    SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);
    Drv_Pwm_Init();
//	FanPwm_Init();

    PCS_HAL_setupCBCProtection();
//    // Enable sync and clock to PWM
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);
    Drv_ClockCntInit();
	
//    Drv_Spi_Init();
	Drv_ADC_Init();
	Drv_I2CA_Init();
	GloabParam_Init();
    rtc_init();
    ModbusRtu1_Init();
    ModbusRtu2_Init();
    Board_init();
    //通锟斤拷CLB锟斤拷锟斤拷EPWM2锟斤拷EPWM3锟接匡拷,硬锟斤拷锟斤拷锟斤拷锟斤拷取锟斤拷
    initCLBTILE1(myCLB1_BASE);
    initCLBTILE2(myCLB2_BASE);
    initCLBTILE4(myCLB4_BASE);
    CLB_enableCLB(myCLB1_BASE);
    CLB_enableCLB(myCLB2_BASE);
    CLB_enableCLB(myCLB4_BASE);
    bsp_clb_pwmEnCtrl(0);
    ipc_init();
    // Configure the CLA memory spaces first followed by
    initial_CLA();
    obj_OutputParam_Init();
    Aes_Check();
   // Enable Global Interrupt (INTM) and realtime interrupt (DBGM)

   EINT;
   ERTM;
}

int16_t U16_DataChange(uint16_t datMsb,uint16_t datLsb)
{
    return (datMsb<<8)|datLsb;
}

/* CODEMAP_CLB_PWM_ENABLE
 * Software gate for PWM enable through CLB blocks. Verify this path on the oscilloscope during driver debug.
 */
void bsp_clb_pwmEnCtrl(int16_t enable)
{
    if(enable == 1)
    {
        CLB_setGPREG(myCLB1_BASE, 0x00);//bit2  is soft in bit 1:锟津开ｏ拷0:锟截憋拷
//        CLB_setGPREG(myCLB2_BASE, 0x04);//bit2  is soft in bit 1:锟津开ｏ拷0:锟截憋拷
//        CLB_setGPREG(myCLB4_BASE, 0x04);//bit2  is soft in bit 1:锟津开ｏ拷0:锟截憋拷
        CLB_setGPREG(myCLB2_BASE, 0x04);//bit2
        CLB_setGPREG(myCLB4_BASE, 0x00);//bit2
    }
    else
    {
        CLB_setGPREG(myCLB1_BASE, 0x00);//bit2
        CLB_setGPREG(myCLB2_BASE, 0x00);//bit2
        CLB_setGPREG(myCLB4_BASE, 0x00);//bit2
    }
}

#define deltaV                     10.0f   // 锟斤拷压锟戒化锟斤拷锟斤拷
#define deltaI                     10.0f   // 锟斤拷锟斤拷锟戒化锟斤拷锟斤拷
#define deltaP                     10.0f   // 锟斤拷锟绞变化锟斤拷锟斤拷
#define deltaR                     1.0f    // 锟斤拷值锟戒化锟斤拷锟斤拷

#define Vdc_refMax                 1200.0f
#define Vdc_refMin                 35.0f
static float32_t Vold = 0.0f;
static float32_t Iold = 0.0f;
static float32_t Pold = 0.0f;
static float32_t deltaD = 0.25f;   // 占锟秸比变化锟斤拷锟斤拷

void MPPT_INC_Boost(void)
{
    float32_t Volt = DC_OutMeter.DC_VBusS_Mean;
    float32_t Curr = DC_OutMeter.DC_IBus_Mean;
    float32_t dV, dI;
    float32_t rVI, rdVI;

    if (m_st_TimerFlag.u16_b1s == 1)
    {
        // 锟斤拷锟斤拷锟窖癸拷偷锟斤拷锟斤拷浠拷锟?
        dV = Volt - Vold;
        dI = Curr - Iold;

        // 锟斤拷锟斤拷锟斤拷锟?
        if (Volt != 0.0f)
            rVI = Curr / Volt;
        else
            rVI = 0.0f;  // 锟斤拷止锟斤拷锟斤拷锟斤拷锟?
        if (dV != 0.0f)
            rdVI = dI / dV;
        else
            rdVI = 0.0f;  // 锟斤拷止锟斤拷锟斤拷锟斤拷锟?
        // MPPT锟姐法
        if (dV != 0.0f)
        {
            if((rVI + rdVI)!= 0.0f)
            {
                if ((rVI + rdVI) > 0.0f)
                {
                    tCpu2Cla.Vdc_MPP += deltaD;
                }
                else
                {
                    tCpu2Cla.Vdc_MPP -= deltaD;
                }
            }
        }
        else
        {
            if (dI != 0.0f)
            {
                if (dI > 0.0f)
                {
                    tCpu2Cla.Vdc_MPP += deltaD;
                }
                else
                {
                    tCpu2Cla.Vdc_MPP -= deltaD;
                }
            }
        }
        if (tCpu2Cla.Vdc_MPP >= Vdc_refMax)
            tCpu2Cla.Vdc_MPP = Vdc_refMax;
        if (tCpu2Cla.Vdc_MPP <= Vdc_refMin)
            tCpu2Cla.Vdc_MPP = Vdc_refMin;
        tCpu2Cla.MPPT_Duty = tCpu2Cla.Vdc_MPP / DC_OutMeter.DC_VBatS_Mean;
        Vold = Volt;
        Iold = Curr;
    }
}

float32_t dP, Power, dV;
void MPPT_PandO_Boost(void)
{
    float32_t Volt = DC_OutMeter.DC_VBusS_Mean;
    float32_t Curr = DC_OutMeter.DC_IBus_Mean;

    if (tCpu2Cla.PwmStartEnable == 1)
    {
        if (m_st_TimerFlag.u16_b1s == 1)
            Cnt ++;
        if (Cnt > 1)
        {
            // 锟斤拷锟姐当前锟斤拷锟绞和变化锟斤拷
            Power = Volt * Curr;              // 锟斤拷前锟斤拷锟斤拷
            dV = Volt - Vold;          // 锟斤拷压锟戒化锟斤拷
            dP = Power - Pold;          // 锟斤拷锟绞变化锟斤拷

            // MPPT锟姐法锟斤拷锟斤拷锟节癸拷锟绞变化锟斤拷锟酵碉拷压锟戒化锟斤拷锟侥凤拷锟斤拷锟叫讹拷锟脚讹拷锟斤拷锟斤拷
            if (dV * dP > 0.0f)             // 锟斤拷锟斤拷锟斤拷锟斤拷锟揭碉拷压锟斤拷锟斤拷 -> 锟斤拷锟斤拷同锟斤拷锟脚讹拷
            {
//                if (fabs(dP / dV) < 0.5f)
//                    deltaD *= 0.0005 * fabs(dP / dV);
//                else
                    deltaD = 0.1f;
                Vdc_MPPT += deltaD;
            }
            else
            {
//                if (fabs(dP / dV) < 0.5f)
//                    deltaD *= 0.0005 * fabs(dP / dV);
//                else
                    deltaD = 0.1f;
                Vdc_MPPT -= deltaD;
            }

            Vold = Volt;
            Pold = Power;
            if (Vdc_MPPT >= DcDc_gParam.MPPT_VdcMax)
                Vdc_MPPT = DcDc_gParam.MPPT_VdcMax;
            if (Vdc_MPPT <= DcDc_gParam.MPPT_VdcMin)
                Vdc_MPPT = DcDc_gParam.MPPT_VdcMin;
            MPPT_Duty = Vdc_MPPT / DC_OutMeter.DC_VBatS_Mean;
        }
    }
    else
    {
        Cnt = 0;
        Vdc_MPPT = DC_OutMeter.DC_VBusS_Mean * 0.5f;
    }
    tCpu2Cla.MPPT_Duty = MPPT_Duty;
}

void Uid_GetRegs(uint8_t *UID_Reg)
{
    int16_t i;
    uint32_t u32_UID_REGS;
    for(i=0;i<8;i++)
     {
         u32_UID_REGS       = HWREG(0x00070200+(i<<1));
         UID_Reg[i*4]       = (uint8_t)(u32_UID_REGS&0x0ff);
         UID_Reg[i*4+1]     = (uint8_t)((u32_UID_REGS>>8)&0x0ff);
         UID_Reg[i*4+2]     = (uint8_t)((u32_UID_REGS>>16)&0x0ff);
         UID_Reg[i*4+3]     = (uint8_t)((u32_UID_REGS>>24)&0x0ff);//锟饺达拷锟斤拷锟街斤拷锟劫达拷锟斤拷锟街斤拷
     }
}

void Read_AesParam(uint8_t *AesParam)
{
    I2C_EE_ReadBuf(EE_ADDR_KEY, 32, AesParam);
}

int16_t Aes_Check(void)
{
    int16_t aes_errCnt,i;

    Uid_GetRegs(u8_UID_REGS);
    Drv_Timer_usDelay(1000);
    AES_Enc(u8_UID_REGS,(uint8_t *)&key0[0]); // 锟斤拷前128bit锟斤拷锟斤拷AES-128锟斤拷锟斤拷锟斤拷锟斤拷
    AES_Enc(&u8_UID_REGS[16],(uint8_t *)&key1[0]); // 锟皆猴拷 128bit 锟斤拷锟斤拷 AES-128 锟斤拷锟斤拷锟斤拷锟斤拷
    Read_AesParam(u8_AES_READ);//
    aes_errCnt = 0;
    for(i=0;i<32;i++)
    {
        if(u8_AES_READ[i] != u8_UID_REGS[i])
            aes_errCnt++;
    }
     return aes_errCnt;
}

//void  GRAPH_calc(GRAPH *g, float32_t v)
//{
////    g->i %= 200;
//    g->buf[g->i++] = v;
//    if(g->i >= 200)
//        g->i = 0;
//}


