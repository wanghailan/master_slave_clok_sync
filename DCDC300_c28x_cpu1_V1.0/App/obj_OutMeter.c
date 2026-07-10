//#############################################################################
//
// FILE:   PCS_FUN.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

/* CODEMAP_OUTMETER_C
 * Role: convert CLA sampled per-unit data into readable voltage/current/power/energy values.
 * These values feed protection, state-machine precharge decisions and Modbus status registers.
 */
#include <Driver/device/device.h>
#include <Driver/device/driverlib.h>
#include "bsp.h"
#include "fastrts.h"
#include "fpu32/C28x_FPU_FastRTS.h"
#include "obj_OutMeter.h"


#define MEAS_ISR_FREQUENCY_HZ      ((float32_t)8000.0f) //有效值计算中断频率

#define DC_Meter_DIV             0.02f     //1/KMeanPointCntCnst   1/50=0.02f;


DCDC_OUTPUT_DATA     DC_OutMeter;
static int16_t       i16_RmsCnt;       //有效值累加数

static float32_t    DCVbat_Sum_pu;
static float32_t    DCVbus_Sum_pu;
static float32_t    DCVbusS_Sum_pu;
static float32_t    DCVbatS_Sum_pu;
//static float32_t    DCVbatP_Sum_pu;
//static float32_t    DCVbatN_Sum_pu;
static float32_t    DCVdc7_Sum_pu;
static float32_t    DCVdc8_Sum_pu;
static float32_t    DCVdc9_Sum_pu;

static float32_t    DCIbat_Sum_pu;
static float32_t    DCIbus_Sum_pu;
static float32_t    DCIL1_Sum_pu;
static float32_t    DCIL2_Sum_pu;
static float32_t    DCIL3_Sum_pu;
static float32_t    DCIdc5_Sum_pu;
static float32_t    DCIdc6_Sum_pu;


/* CODEMAP_OUT_INIT: clear accumulation buffers and displayed meter values. */
void  obj_OutputParam_Init(void)
{
    i16_RmsCnt = 0;

    DCVbat_Sum_pu   = 0.0f;
    DCVbus_Sum_pu   = 0.0f;
    DCVbusS_Sum_pu  = 0.0f;
    DCVbatS_Sum_pu  = 0.0f;
//    DCVbatP_Sum_pu  = 0.0f;
//    DCVbatN_Sum_pu  = 0.0f;
    DCVdc7_Sum_pu   = 0.0f;
    DCVdc8_Sum_pu   = 0.0f;
    DCVdc9_Sum_pu   = 0.0f;

    DCIbat_Sum_pu   = 0.0f;
    DCIbus_Sum_pu   = 0.0f;
    DCIL1_Sum_pu    = 0.0f;
    DCIL2_Sum_pu    = 0.0f;
    DCIL3_Sum_pu    = 0.0f;
    DCIdc5_Sum_pu   = 0.0f;
    DCIdc6_Sum_pu   = 0.0f;

    DC_OutMeter.DC_VBat_Mean    = 0.0f;
    DC_OutMeter.DC_VBus_Mean    = 0.0f;
    DC_OutMeter.DC_VBatS_Mean   = 0.0f;
    DC_OutMeter.DC_VBusS_Mean   = 0.0f;
//    DC_OutMeter.DC_VBatP_Mean   = 0.0f;
//    DC_OutMeter.DC_VBatN_Mean   = 0.0f;
    DC_OutMeter.DC_Vdc7_Mean    = 0.0f;
    DC_OutMeter.DC_Vdc8_Mean    = 0.0f;
    DC_OutMeter.DC_Vdc9_Mean    = 0.0f;

    DC_OutMeter.DC_IBat_Mean    = 0.0f;
    DC_OutMeter.DC_IBus_Mean    = 0.0f;
    DC_OutMeter.DC_IL1_Mean     = 0.0f;
    DC_OutMeter.DC_IL2_Mean     = 0.0f;
    DC_OutMeter.DC_IL3_Mean     = 0.0f;
    DC_OutMeter.DC_Idc5_Mean    = 0.0f;
    DC_OutMeter.DC_Idc6_Mean    = 0.0f;

}


/* CODEMAP_POWER_CALC: calculate high/low side power and efficiency from averaged values. */
void  obj_Power_MeanCalc(void)
{
    //两侧功率平均值计算
    DC_OutMeter.DC_PowerBat   = DC_OutMeter.DC_VBatS_Mean*DC_OutMeter.DC_IBat_Mean*0.001f;
    DC_OutMeter.DC_PowerBus   = DC_OutMeter.DC_VBusS_Mean*DC_OutMeter.DC_IBus_Mean*0.001f;
    if(tCpu2Cla.PwmStartEnable == 1)
    {
        if((DC_OutMeter.DC_PowerBat > 1.0f)&&(DC_OutMeter.DC_PowerBus > 1.0f))
        {
            // 高压侧为电池侧(燃料电池)，低压侧为母线侧(和实际接线有关) (储能电池)
            if(DcDc_gParam.WorkMode == Mode_Buck)//降压模式,从高压侧启动
                DC_OutMeter.DC_transfEffi = DC_OutMeter.DC_PowerBus/DC_OutMeter.DC_PowerBat;  // 输出/输入
            else  // Boost升压,低压→高压
                DC_OutMeter.DC_transfEffi = DC_OutMeter.DC_PowerBat/DC_OutMeter.DC_PowerBus;
        }
    }
}


/* CODEMAP_MEAN_CALC
 * Accumulates KMeanPointCntCnst samples from tCla2Cpu and then converts to real units.
 * If Modbus voltage/current is wrong, trace CLA sampling -> calibration -> this function -> Modbus.
 */
void  obj_VolCurr_MeanCalc(void)
{
    i16_RmsCnt++;

    DCVbat_Sum_pu    += tCla2Cpu.DC_VBat_sensed_pu;
    DCVbus_Sum_pu    += tCla2Cpu.DC_VBus_sensed_pu;
    DCVbatS_Sum_pu   += tCla2Cpu.DC_VBatS_sensed_pu;
    DCVbusS_Sum_pu   += tCla2Cpu.DC_VBusS_sensed_pu;
//    DCVbatP_Sum_pu   += tCla2Cpu.DC_VBatP_sensed_pu;
//    DCVbatN_Sum_pu   += tCla2Cpu.DC_VBatN_sensed_pu;
    DCVdc7_Sum_pu    += tCla2Cpu.DC_Vdc7_sensed_pu;
    DCVdc8_Sum_pu    += tCla2Cpu.DC_Vdc8_sensed_pu;
    DCVdc9_Sum_pu    += tCla2Cpu.DC_Vdc9_sensed_pu;

    DCIbat_Sum_pu    += tCla2Cpu.DC_IBat_sensed_pu;
    DCIbus_Sum_pu    += tCla2Cpu.DC_IBus_sensed_pu;
    DCIL1_Sum_pu     += tCla2Cpu.DC_IL1_sensed_pu;
    DCIL2_Sum_pu     += tCla2Cpu.DC_IL2_sensed_pu;
    DCIL3_Sum_pu     += tCla2Cpu.DC_IL3_sensed_pu;
    DCIdc5_Sum_pu    += tCla2Cpu.DC_Idc5_sensed_pu;
    DCIdc6_Sum_pu    += tCla2Cpu.DC_Idc6_sensed_pu;

    if(i16_RmsCnt >= KMeanPointCntCnst)
    {
        DC_OutMeter.DC_VBat_Mean    =   (DCVbat_Sum_pu*DC_Meter_DIV)*DC_VDC_MAX_SENSE_RATIO;// - 10.0f
//        if (DC_OutMeter.DC_VBat_Mean < 0.0f)
//        {
//            DC_OutMeter.DC_VBat_Mean = 0.0f;
//        }
        DC_OutMeter.DC_VBus_Mean    =   (DCVbus_Sum_pu*DC_Meter_DIV)*DC_VDC_MAX_SENSE_RATIO;
        DC_OutMeter.DC_VBatS_Mean   =   (DCVbatS_Sum_pu*DC_Meter_DIV)*DC_VDC_MAX_SENSE_RATIO;
        DC_OutMeter.DC_VBusS_Mean   =   (DCVbusS_Sum_pu*DC_Meter_DIV)*DC_VDC_MAX_SENSE_RATIO;
//        DC_OutMeter.DC_VBatP_Mean   =   (DCVbatP_Sum_pu*DC_Meter_DIV)*DC_VDC_MAX_SENSE_RATIO;
//        DC_OutMeter.DC_VBatN_Mean   =   (DCVbatN_Sum_pu*DC_Meter_DIV)*DC_VDC_MAX_SENSE_RATIO;
        DC_OutMeter.DC_Vdc7_Mean    =   (DCVdc7_Sum_pu*DC_Meter_DIV)*DC_VDC_MAX_SENSE_RATIO;
        DC_OutMeter.DC_Vdc8_Mean    =   (DCVdc8_Sum_pu*DC_Meter_DIV)*DC_VDC_MAX_SENSE_RATIO;
        DC_OutMeter.DC_Vdc9_Mean    =   (DCVdc9_Sum_pu*DC_Meter_DIV)*DC_VDC_MAX_SENSE_RATIO;

        DC_OutMeter.DC_IBat_Mean    =   (DCIbat_Sum_pu*DC_Meter_DIV)*DC_IDC200_MAX_SENSE_RATIO;
        DC_OutMeter.DC_IBus_Mean    =   (DCIbus_Sum_pu*DC_Meter_DIV)*DC_IDC300_MAX_SENSE_RATIO;
        DC_OutMeter.DC_IL1_Mean     =   (DCIL1_Sum_pu*DC_Meter_DIV)*DC_IDC100_MAX_SENSE_RATIO;
        DC_OutMeter.DC_IL2_Mean     =   (DCIL2_Sum_pu*DC_Meter_DIV)*DC_IDC100_MAX_SENSE_RATIO;
        DC_OutMeter.DC_IL3_Mean     =   (DCIL3_Sum_pu*DC_Meter_DIV)*DC_IDC100_MAX_SENSE_RATIO;
        DC_OutMeter.DC_Idc5_Mean    =   (DCIdc5_Sum_pu*DC_Meter_DIV)*DC_IDC100_MAX_SENSE_RATIO;
        DC_OutMeter.DC_Idc6_Mean    =   (DCIdc6_Sum_pu*DC_Meter_DIV)*DC_IDC100_MAX_SENSE_RATIO;

        DCVbat_Sum_pu   = 0.0f;
        DCVbus_Sum_pu   = 0.0f;
        DCVbusS_Sum_pu  = 0.0f;
        DCVbatS_Sum_pu  = 0.0f;
//        DCVbatP_Sum_pu  = 0.0f;
//        DCVbatN_Sum_pu  = 0.0f;
        DCVdc7_Sum_pu   = 0.0f;
        DCVdc8_Sum_pu   = 0.0f;
        DCVdc9_Sum_pu   = 0.0f;

        DCIbat_Sum_pu   = 0.0f;
        DCIbus_Sum_pu   = 0.0f;
        DCIL1_Sum_pu    = 0.0f;
        DCIL2_Sum_pu    = 0.0f;
        DCIL3_Sum_pu    = 0.0f;
        DCIdc5_Sum_pu   = 0.0f;
        DCIdc6_Sum_pu   = 0.0f;

        i16_RmsCnt = 0;
    }

}


/************************************************************************************
函数名称：    obj_Output_KWHDeal()
功能描述:     输出电能计算
************************************************************************************/
static int32_t i32RecInkWS_Cnt = 0;//低压侧功率计数
//
/* CODEMAP_KWH
 * One-second energy accumulator. Writes EEPROM only when another 1 kWh is accumulated.
 */
void  obj_Output_KWHDeal(void)
{
    uint32_t TempPout_Bat = 0;
    uint16_t reg_buf[8]={0};
    int16_t  ChargeData_flag = 0;
    if(m_st_TimerFlag.u16_b1s == 1)
    {
        if(DcDc_gParam.WorkMode == Mode_Buck)//降压模式,从高压侧启动
        {
            TempPout_Bat = (uint32_t)(DC_OutMeter.DC_PowerBat*1000.0f);  //交流有功功率 0.001KW
            i32RecInkWS_Cnt += TempPout_Bat;
            if(i32RecInkWS_Cnt >= 3600000)//1度电 3600000 3600S*1000W
            {
                i32RecInkWS_Cnt -= 3600000;
                ChargeData_flag = 1;
                DC_OutMeter.DC_LdisChargeKWH++;
                if(DC_OutMeter.DC_LdisChargeKWH >= (10000-1))
                {
                    DC_OutMeter.DC_LdisChargeKWH = 0;
                    DC_OutMeter.DC_HdisChargeKWH++;
                }
            }
        }
        else//升压模式,从低压侧启动
        {
            TempPout_Bat = (uint32_t)(DC_OutMeter.DC_PowerBat*1000.0f);  //交流有功功率 0.001KW
            i32RecInkWS_Cnt += TempPout_Bat;
            if(i32RecInkWS_Cnt >= 3600000)//1度电 3600000 3600S*1000W
            {
                i32RecInkWS_Cnt -= 3600000;
                ChargeData_flag = 1;
                DC_OutMeter.DC_LChargeKWH++;
                if(DC_OutMeter.DC_LChargeKWH >= (10000-1))
                {
                    DC_OutMeter.DC_LChargeKWH = 0;
                    DC_OutMeter.DC_HChargeKWH++;
                }
            }
        }

        if(ChargeData_flag == 1)
        {
            reg_buf[0] = DC_OutMeter.DC_LdisChargeKWH&0x0ff;
            reg_buf[1] = (DC_OutMeter.DC_LdisChargeKWH>>8)&0x0ff;
            reg_buf[2] = DC_OutMeter.DC_HdisChargeKWH&0x0ff;
            reg_buf[3] = (DC_OutMeter.DC_HdisChargeKWH>>8)&0x0ff;
            reg_buf[4] = DC_OutMeter.DC_LChargeKWH&0x0ff;
            reg_buf[5] = (DC_OutMeter.DC_LChargeKWH>>8)&0x0ff;
            reg_buf[6] = DC_OutMeter.DC_HChargeKWH&0x0ff;
            reg_buf[7] = (DC_OutMeter.DC_HChargeKWH>>8)&0x0ff;
            I2C_EE_WriteBuf(EE_ADDR_CHARGE, 8, reg_buf);
        }
    }
}





