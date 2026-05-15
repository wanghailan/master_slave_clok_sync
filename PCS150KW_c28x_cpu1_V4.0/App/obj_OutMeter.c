//#############################################################################
//
// FILE:   PCS_FUN.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

#include <Driver/device/device.h>
#include <Driver/device/driverlib.h>
#include "bsp.h"
#include "fastrts.h"
#include "fpu32/C28x_FPU_FastRTS.h"
#include "obj_OutMeter.h"


#define PCS_GRID_MAX_FREQ           55
#define PCS_GRID_MIN_FREQ           45
#define MEAS_ISR_FREQUENCY_HZ      ((float32_t)8000.0f) //有效值计算中断频率

#define PCS_50HZ_8K_DIV             0.00625f


static float32_t  math_f32_square(float32_t f32_Dat);

POWER_MEAS_SINE_ANALYZER Pcs_MeasAnalyzer_A;
POWER_MEAS_SINE_ANALYZER Pcs_MeasAnalyzer_B;
POWER_MEAS_SINE_ANALYZER Pcs_MeasAnalyzer_C;


PCS_OUTPUT_DATA        Pcs_Output_Meter;
static int16_t         i16_RmsCnt;       //有效值累加数

static float32_t       ACRms_VC_Sum_fo;

static float32_t       ACRms_VAB_Sum_pu;
static float32_t       ACRms_VBC_Sum_pu;
static float32_t       ACRms_VCA_Sum_pu;

static float32_t       ACRms_VLA_Sum_pu;
static float32_t       ACRms_VLB_Sum_pu;
static float32_t       ACRms_VLC_Sum_pu;

static float32_t       ACRms_IINVA_Sum_pu;
static float32_t       ACRms_IINVB_Sum_pu;
static float32_t       ACRms_IINVC_Sum_pu;

static float32_t       DCVolts_Sum_pu;
static float32_t       DCVoltsP_Sum_pu;
static float32_t       DCVoltsN_Sum_pu;
static float32_t       DCCurrent_Sum_pu;
static float32_t       DCIBUS_Sum_pu;


void  obj_PcsOutput_Init(void)
{
    // sine analyzer initialization
    POWER_MEAS_SINE_ANALYZER_reset(&Pcs_MeasAnalyzer_C);
    POWER_MEAS_SINE_ANALYZER_config(&Pcs_MeasAnalyzer_C,
                                    MEAS_ISR_FREQUENCY_HZ,
                                    (float32_t)0.05f,
                                    (float32_t)PCS_GRID_MAX_FREQ,
                                    (float32_t)PCS_GRID_MIN_FREQ);
    POWER_MEAS_SINE_ANALYZER_reset(&Pcs_MeasAnalyzer_A);
    POWER_MEAS_SINE_ANALYZER_config(&Pcs_MeasAnalyzer_A,
                                    MEAS_ISR_FREQUENCY_HZ,
                                    (float32_t)0.05f,
                                    (float32_t)PCS_GRID_MAX_FREQ,
                                    (float32_t)PCS_GRID_MIN_FREQ);
    POWER_MEAS_SINE_ANALYZER_reset(&Pcs_MeasAnalyzer_B);
    POWER_MEAS_SINE_ANALYZER_config(&Pcs_MeasAnalyzer_B,
                                    MEAS_ISR_FREQUENCY_HZ,
                                    (float32_t)0.05f,
                                    (float32_t)PCS_GRID_MAX_FREQ,
                                    (float32_t)PCS_GRID_MIN_FREQ);

    ACRms_VAB_Sum_pu = 0.0f;
    ACRms_VBC_Sum_pu = 0.0f;
    ACRms_VCA_Sum_pu = 0.0f;

    ACRms_VLA_Sum_pu = 0.0f;
    ACRms_VLB_Sum_pu = 0.0f;
    ACRms_VLC_Sum_pu = 0.0f;

    ACRms_VC_Sum_fo = 0.0f;

    i16_RmsCnt = 0;

    DCVolts_Sum_pu   = 0.0f;
    DCVoltsP_Sum_pu  = 0.0f;
    DCVoltsN_Sum_pu  = 0.0f;
    DCCurrent_Sum_pu = 0.0f;
    DCIBUS_Sum_pu    = 0.0f;

    Pcs_Output_Meter.PCS_AC_VRms_PhaseA = 0.0f;
    Pcs_Output_Meter.PCS_AC_VRms_PhaseB = 0.0f;
    Pcs_Output_Meter.PCS_AC_VRms_PhaseC = 0.0f;
    Pcs_Output_Meter.PCS_AC_VRms_LineAB = 0.0f;
    Pcs_Output_Meter.PCS_AC_VRms_LineBC = 0.0f;
    Pcs_Output_Meter.PCS_AC_VRms_LineCA = 0.0f;
    Pcs_Output_Meter.PCS_AC_IRms_A  = 0.0f;
    Pcs_Output_Meter.PCS_AC_IRms_B  = 0.0f;
    Pcs_Output_Meter.PCS_AC_IRms_C  = 0.0f;
    Pcs_Output_Meter.PCS_DC_IBUS_N  = 0.0f;

    Pcs_Output_Meter.PCS_AC_VLRms_PhaseA = 0.0f;
    Pcs_Output_Meter.PCS_AC_VLRms_PhaseB = 0.0f;
    Pcs_Output_Meter.PCS_AC_VLRms_PhaseC = 0.0f;

    Pcs_Output_Meter.PCS_AC_PowerP_A  = 0.0f;
    Pcs_Output_Meter.PCS_AC_PowerP_B  = 0.0f;
    Pcs_Output_Meter.PCS_AC_PowerP_C  = 0.0f;

    Pcs_Output_Meter.PCS_AC_PowerQ_A  = 0.0f;
    Pcs_Output_Meter.PCS_AC_PowerQ_B  = 0.0f;
    Pcs_Output_Meter.PCS_AC_PowerQ_C  = 0.0f;

    Pcs_Output_Meter.PCS_AC_PowerS_A  = 0.0f;
    Pcs_Output_Meter.PCS_AC_PowerS_B  = 0.0f;
    Pcs_Output_Meter.PCS_AC_PowerS_C  = 0.0f;

    Pcs_Output_Meter.PCS_DC_BusVol   = 0.0f;
    Pcs_Output_Meter.PCS_DC_PBusVol  = 0.0f;
    Pcs_Output_Meter.PCS_DC_NBusVol  = 0.0f;
    Pcs_Output_Meter.PCS_DC_BusCurr  = 0.0f;

    Pcs_Output_Meter.PCS_AC_PowerP   = 0.0f;
    Pcs_Output_Meter.PCS_AC_PowerQ   = 0.0f;
    Pcs_Output_Meter.PCS_AC_PowerS   = 0.0f;
    Pcs_Output_Meter.PCS_DC_Power    = 0.0f;
}

void  obj_PcsOutput_PowerMeasAC(void)
{
    Pcs_MeasAnalyzer_A.v = tCla2Cpu.Pcs_vGrid_A_sensed_pu;
    Pcs_MeasAnalyzer_A.i = tCla2Cpu.Pcs_iGrid_A_sensed_pu;
    POWER_MEAS_SINE_ANALYZER_run(&Pcs_MeasAnalyzer_A);
    Pcs_Output_Meter.PCS_AC_IRms_A      = Pcs_MeasAnalyzer_A.iRms * PCS_IGRID_MAX_SENSE_AMPS;
    Pcs_Output_Meter.PCS_AC_VRms_PhaseA = Pcs_MeasAnalyzer_A.vRms * PCS_VGRID_MAX_SENSE_VOLTS;
    Pcs_Output_Meter.PCS_AC_PowerP_A    = Pcs_MeasAnalyzer_A.pRms * PCS_VGRID_MAX_SENSE_VOLTS * PCS_IGRID_MAX_SENSE_AMPS*0.001f;
    Pcs_Output_Meter.PCS_AC_PowerS_A    = Pcs_MeasAnalyzer_A.vaRms * PCS_VGRID_MAX_SENSE_VOLTS * PCS_IGRID_MAX_SENSE_AMPS*0.001f;
    Pcs_Output_Meter.PCS_AC_PF_A        = Pcs_MeasAnalyzer_A.powerFactor;
    Pcs_Output_Meter.PCS_AC_Freq_A      = Pcs_MeasAnalyzer_A.acFreqAvg;

    Pcs_MeasAnalyzer_B.v = tCla2Cpu.Pcs_vGrid_B_sensed_pu;
    Pcs_MeasAnalyzer_B.i = tCla2Cpu.Pcs_iGrid_B_sensed_pu;
    POWER_MEAS_SINE_ANALYZER_run(&Pcs_MeasAnalyzer_B);
    Pcs_Output_Meter.PCS_AC_IRms_B      = Pcs_MeasAnalyzer_B.iRms * PCS_IGRID_MAX_SENSE_AMPS;
    Pcs_Output_Meter.PCS_AC_VRms_PhaseB = Pcs_MeasAnalyzer_B.vRms * PCS_VGRID_MAX_SENSE_VOLTS;
    Pcs_Output_Meter.PCS_AC_PowerP_B    = Pcs_MeasAnalyzer_B.pRms * PCS_VGRID_MAX_SENSE_VOLTS * PCS_IGRID_MAX_SENSE_AMPS*0.001f;
    Pcs_Output_Meter.PCS_AC_PowerS_B    = Pcs_MeasAnalyzer_B.vaRms * PCS_VGRID_MAX_SENSE_VOLTS * PCS_IGRID_MAX_SENSE_AMPS*0.001f;
    Pcs_Output_Meter.PCS_AC_PF_B        = Pcs_MeasAnalyzer_B.powerFactor;
    Pcs_Output_Meter.PCS_AC_Freq_B      = Pcs_MeasAnalyzer_B.acFreqAvg;

    Pcs_MeasAnalyzer_C.v = tCla2Cpu.Pcs_vGrid_C_sensed_pu;
    Pcs_MeasAnalyzer_C.i = tCla2Cpu.Pcs_iGrid_C_sensed_pu;
    POWER_MEAS_SINE_ANALYZER_run(&Pcs_MeasAnalyzer_C);
    Pcs_Output_Meter.PCS_AC_IRms_C      = Pcs_MeasAnalyzer_C.iRms * PCS_IGRID_MAX_SENSE_AMPS;
    Pcs_Output_Meter.PCS_AC_VRms_PhaseC = Pcs_MeasAnalyzer_C.vRms * PCS_VGRID_MAX_SENSE_VOLTS;
    Pcs_Output_Meter.PCS_AC_PowerP_C    = Pcs_MeasAnalyzer_C.pRms * PCS_VGRID_MAX_SENSE_VOLTS * PCS_IGRID_MAX_SENSE_AMPS*0.001f;
    Pcs_Output_Meter.PCS_AC_PowerS_C    = Pcs_MeasAnalyzer_C.vaRms * PCS_VGRID_MAX_SENSE_VOLTS * PCS_IGRID_MAX_SENSE_AMPS*0.001f;
    Pcs_Output_Meter.PCS_AC_PF_C        = Pcs_MeasAnalyzer_C.powerFactor;
    Pcs_Output_Meter.PCS_AC_Freq_C      = Pcs_MeasAnalyzer_C.acFreqAvg;

    Cpu1Ipc_cpu2cm.PCS_AC_IRms_A         = (int16_t)(Pcs_Output_Meter.PCS_AC_IRms_A*10.0f);
    Cpu1Ipc_cpu2cm.PCS_AC_IRms_B         = (int16_t)(Pcs_Output_Meter.PCS_AC_IRms_B*10.0f);
    Cpu1Ipc_cpu2cm.PCS_AC_IRms_C         = (int16_t)(Pcs_Output_Meter.PCS_AC_IRms_C*10.0f);
    Cpu1Ipc_cpu2cm.PCS_AC_VRms_PhaseA    = (int16_t)(Pcs_Output_Meter.PCS_AC_VRms_PhaseA*10.0f);
    Cpu1Ipc_cpu2cm.PCS_AC_VRms_PhaseB    = (int16_t)(Pcs_Output_Meter.PCS_AC_VRms_PhaseB*10.0f);
    Cpu1Ipc_cpu2cm.PCS_AC_VRms_PhaseC    = (int16_t)(Pcs_Output_Meter.PCS_AC_VRms_PhaseC*10.0f);
    Cpu1Ipc_cpu2cm.PCS_AC_PowerP_A       = (int16_t)(Pcs_Output_Meter.PCS_AC_PowerP_A*10.0f);
    Cpu1Ipc_cpu2cm.PCS_AC_PowerP_B       = (int16_t)(Pcs_Output_Meter.PCS_AC_PowerP_B*10.0f);
    Cpu1Ipc_cpu2cm.PCS_AC_PowerP_C       = (int16_t)(Pcs_Output_Meter.PCS_AC_PowerP_C*10.0f);
    Cpu1Ipc_cpu2cm.PCS_AC_PowerS_A       = (int16_t)(Pcs_Output_Meter.PCS_AC_PowerS_A*10.0f);
    Cpu1Ipc_cpu2cm.PCS_AC_PowerS_B       = (int16_t)(Pcs_Output_Meter.PCS_AC_PowerS_B*10.0f);
    Cpu1Ipc_cpu2cm.PCS_AC_PowerS_C       = (int16_t)(Pcs_Output_Meter.PCS_AC_PowerS_C*10.0f);
    Cpu1Ipc_cpu2cm.PCS_AC_PF_A           = (int16_t)(Pcs_Output_Meter.PCS_AC_PF_A*100.0f);
    Cpu1Ipc_cpu2cm.PCS_AC_PF_B           = (int16_t)(Pcs_Output_Meter.PCS_AC_PF_B*100.0f);
    Cpu1Ipc_cpu2cm.PCS_AC_PF_C           = (int16_t)(Pcs_Output_Meter.PCS_AC_PF_C*100.0f);
    Cpu1Ipc_cpu2cm.PCS_AC_Freq_A         = (int16_t)(Pcs_Output_Meter.PCS_AC_Freq_A*100.0f);
    Cpu1Ipc_cpu2cm.PCS_AC_Freq_B         = (int16_t)(Pcs_Output_Meter.PCS_AC_Freq_B*100.0f);
    Cpu1Ipc_cpu2cm.PCS_AC_Freq_C         = (int16_t)(Pcs_Output_Meter.PCS_AC_Freq_C*100.0f);
}

void  obj_PcsOutput_PowerMeas(void)
{
    //AC总有功功率
    Pcs_Output_Meter.PCS_AC_PowerP      = Pcs_Output_Meter.PCS_AC_PowerP_A+Pcs_Output_Meter.PCS_AC_PowerP_B+Pcs_Output_Meter.PCS_AC_PowerP_C;
    //AC总无功功率
    Pcs_Output_Meter.PCS_AC_PowerQ      = Pcs_Output_Meter.PCS_AC_PowerQ_A+Pcs_Output_Meter.PCS_AC_PowerQ_B+Pcs_Output_Meter.PCS_AC_PowerQ_C;
    //AC总视在功功率
    Pcs_Output_Meter.PCS_AC_PowerS      = Pcs_Output_Meter.PCS_AC_PowerS_A+Pcs_Output_Meter.PCS_AC_PowerS_B+Pcs_Output_Meter.PCS_AC_PowerS_C;
    Pcs_Output_Meter.PCS_DC_Power       = Pcs_Output_Meter.PCS_DC_BusVol*Pcs_Output_Meter.PCS_DC_BusCurr*0.001f;

    Cpu1Ipc_cpu2cm.PCS_AC_PowerP        = (int16_t)(Pcs_Output_Meter.PCS_AC_PowerP*100.0f);
    Cpu1Ipc_cpu2cm.PCS_AC_PowerQ        = (int16_t)(Pcs_Output_Meter.PCS_AC_PowerQ*100.0f);
    Cpu1Ipc_cpu2cm.PCS_AC_PowerS        = (int16_t)(Pcs_Output_Meter.PCS_AC_PowerS*100.0f);
    Cpu1Ipc_cpu2cm.PCS_DC_Power         = (int16_t)(Pcs_Output_Meter.PCS_DC_Power*100.0f);
}


void  obj_PcsOutput_RmsCalc(void)
{
    i16_RmsCnt++;

    ACRms_VAB_Sum_pu    += math_f32_square(tCla2Cpu.Pcs_vGrid_AB_sensed_pu);
    ACRms_VBC_Sum_pu    += math_f32_square(tCla2Cpu.Pcs_vGrid_BC_sensed_pu);
    ACRms_VCA_Sum_pu    += math_f32_square(tCla2Cpu.Pcs_vGrid_CA_sensed_pu);

    if(Cpu1Ipc_cm2cpu.debugMode == 1)//调试模式
    {
        ACRms_IINVA_Sum_pu  += math_f32_square(tCla2Cpu.Pcs_iGrid_A_sensed_pu);
        ACRms_IINVB_Sum_pu  += math_f32_square(tCla2Cpu.Pcs_iGrid_B_sensed_pu);
        ACRms_IINVC_Sum_pu  += math_f32_square(tCla2Cpu.Pcs_iGrid_C_sensed_pu);
    }
    else
    {
        ACRms_IINVA_Sum_pu  += math_f32_square(tCla2Cpu.Pcs_iInv_A_sensed_pu);
        ACRms_IINVB_Sum_pu  += math_f32_square(tCla2Cpu.Pcs_iInv_B_sensed_pu);
        ACRms_IINVC_Sum_pu  += math_f32_square(tCla2Cpu.Pcs_iInv_C_sensed_pu);
    }

    ACRms_VLA_Sum_pu    += math_f32_square(tCla2Cpu.Pcs_vInv_A_sensed_pu);
    ACRms_VLB_Sum_pu    += math_f32_square(tCla2Cpu.Pcs_vInv_B_sensed_pu);
    ACRms_VLC_Sum_pu    += math_f32_square(tCla2Cpu.Pcs_vInv_C_sensed_pu);

    ACRms_VC_Sum_fo     += math_f32_square(PCS_spll_3ph_grid.fo);

    DCVoltsP_Sum_pu     += tCla2Cpu.Pcs_vBusP_sensed_pu;
    DCVoltsN_Sum_pu     += tCla2Cpu.Pcs_vBusN_sensed_pu;
    DCVolts_Sum_pu      += tCla2Cpu.Pcs_vBatt_sensed_pu;
    DCCurrent_Sum_pu    += tCla2Cpu.Pcs_iBus_sensed_pu;
//        DCIBUS_Sum_pu       += tCla2Cpu.Pcs_iBusN_sensed_pu;
    DCIBUS_Sum_pu       += math_f32_square(tCla2Cpu.Pcs_iBusN_sensed_pu);


    if(i16_RmsCnt >= K50HzPointCntCnst)
    {
        Pcs_Output_Meter.PCS_AC_VRms_LineAB   = sqrtf(ACRms_VAB_Sum_pu*PCS_50HZ_8K_DIV)*PCS_VGRID_MAX_SENSE_VOLTS;
        Pcs_Output_Meter.PCS_AC_VRms_LineBC   = sqrtf(ACRms_VBC_Sum_pu*PCS_50HZ_8K_DIV)*PCS_VGRID_MAX_SENSE_VOLTS;
        Pcs_Output_Meter.PCS_AC_VRms_LineCA   = sqrtf(ACRms_VCA_Sum_pu*PCS_50HZ_8K_DIV)*PCS_VGRID_MAX_SENSE_VOLTS;

        Pcs_Output_Meter.PCS_AC_VLRms_PhaseA  = sqrtf(ACRms_VLA_Sum_pu*PCS_50HZ_8K_DIV)*PCS_VINV_MAX_SENSE_VOLTS;
        Pcs_Output_Meter.PCS_AC_VLRms_PhaseB  = sqrtf(ACRms_VLB_Sum_pu*PCS_50HZ_8K_DIV)*PCS_VINV_MAX_SENSE_VOLTS;
        Pcs_Output_Meter.PCS_AC_VLRms_PhaseC  = sqrtf(ACRms_VLC_Sum_pu*PCS_50HZ_8K_DIV)*PCS_VINV_MAX_SENSE_VOLTS;

        Pcs_Output_Meter.PCS_DC_BusVol        = (DCVolts_Sum_pu*PCS_50HZ_8K_DIV)*PCS_VBATT_MAX_SENSE_VOLTS;
        Pcs_Output_Meter.PCS_DC_PBusVol       = (DCVoltsP_Sum_pu*PCS_50HZ_8K_DIV)*PCS_VDC_MAX_SENSE_VOLTS;
        Pcs_Output_Meter.PCS_DC_NBusVol       = (DCVoltsN_Sum_pu*PCS_50HZ_8K_DIV)*PCS_VDC_MAX_SENSE_VOLTS;
        Pcs_Output_Meter.PCS_DC_BusCurr       = (DCCurrent_Sum_pu*PCS_50HZ_8K_DIV)*PCS_IDC_MAX_SENSE_VOLTS;
        Pcs_Output_Meter.PCS_DC_IBUS_N        = sqrtf(DCIBUS_Sum_pu*PCS_50HZ_8K_DIV)*PCS_IBUSN_MAX_SENSE_AMPS;
        Pcs_Gridpllfo = sqrtf(ACRms_VC_Sum_fo/(float32)i16_RmsCnt)*1.0f;

        Pcs_Output_Meter.PCS_AC_ILRms_A       = sqrtf(ACRms_IINVA_Sum_pu*PCS_50HZ_8K_DIV)*PCS_IINV_MAX_SENSE_AMPS;
        Pcs_Output_Meter.PCS_AC_ILRms_B       = sqrtf(ACRms_IINVB_Sum_pu*PCS_50HZ_8K_DIV)*PCS_IINV_MAX_SENSE_AMPS;
        Pcs_Output_Meter.PCS_AC_ILRms_C       = sqrtf(ACRms_IINVC_Sum_pu*PCS_50HZ_8K_DIV)*PCS_IINV_MAX_SENSE_AMPS;

        Pcs_Output_Meter.PCS_AC_PowerQ_A      = sqrtf(Pcs_Output_Meter.PCS_AC_PowerS_A*Pcs_Output_Meter.PCS_AC_PowerS_A-Pcs_Output_Meter.PCS_AC_PowerP_A*Pcs_Output_Meter.PCS_AC_PowerP_A);
        Pcs_Output_Meter.PCS_AC_PowerQ_B      = sqrtf(Pcs_Output_Meter.PCS_AC_PowerS_B*Pcs_Output_Meter.PCS_AC_PowerS_B-Pcs_Output_Meter.PCS_AC_PowerP_B*Pcs_Output_Meter.PCS_AC_PowerP_B);
        Pcs_Output_Meter.PCS_AC_PowerQ_C      = sqrtf(Pcs_Output_Meter.PCS_AC_PowerS_C*Pcs_Output_Meter.PCS_AC_PowerS_C-Pcs_Output_Meter.PCS_AC_PowerP_C*Pcs_Output_Meter.PCS_AC_PowerP_C);

        //更新到cm显示
        Cpu1Ipc_cpu2cm.PCS_AC_VRms_LineAB     = (int16_t)(Pcs_Output_Meter.PCS_AC_VRms_LineAB*10.0f);
        Cpu1Ipc_cpu2cm.PCS_AC_VRms_LineBC     = (int16_t)(Pcs_Output_Meter.PCS_AC_VRms_LineBC*10.0f);
        Cpu1Ipc_cpu2cm.PCS_AC_VRms_LineCA     = (int16_t)(Pcs_Output_Meter.PCS_AC_VRms_LineCA*10.0f);

        Cpu1Ipc_cpu2cm.PCS_AC_VRms_PhaseA     = (int16_t)(Pcs_Output_Meter.PCS_AC_VLRms_PhaseA*10.0f);
        Cpu1Ipc_cpu2cm.PCS_AC_VRms_PhaseB     = (int16_t)(Pcs_Output_Meter.PCS_AC_VLRms_PhaseB*10.0f);
        Cpu1Ipc_cpu2cm.PCS_AC_VRms_PhaseC     = (int16_t)(Pcs_Output_Meter.PCS_AC_VLRms_PhaseC*10.0f);

        Cpu1Ipc_cpu2cm.PCS_AC_ILRms_A         = (int16_t)(Pcs_Output_Meter.PCS_AC_ILRms_A*10.0f);
        Cpu1Ipc_cpu2cm.PCS_AC_ILRms_B         = (int16_t)(Pcs_Output_Meter.PCS_AC_ILRms_B*10.0f);
        Cpu1Ipc_cpu2cm.PCS_AC_ILRms_C         = (int16_t)(Pcs_Output_Meter.PCS_AC_ILRms_C*10.0f);

        Cpu1Ipc_cpu2cm.PCS_AC_PowerQ_A        = (int16_t)(Pcs_Output_Meter.PCS_AC_PowerQ_A*100.0f);
        Cpu1Ipc_cpu2cm.PCS_AC_PowerQ_B        = (int16_t)(Pcs_Output_Meter.PCS_AC_PowerQ_B*100.0f);
        Cpu1Ipc_cpu2cm.PCS_AC_PowerQ_C        = (int16_t)(Pcs_Output_Meter.PCS_AC_PowerQ_C*100.0f);

        Cpu1Ipc_cpu2cm.PCS_DC_BusVol          = (int16_t)(Pcs_Output_Meter.PCS_DC_BusVol*10.0f);
        Cpu1Ipc_cpu2cm.PCS_DC_PBusVol         = (int16_t)(Pcs_Output_Meter.PCS_DC_PBusVol*10.0f);
        Cpu1Ipc_cpu2cm.PCS_DC_NBusVol         = (int16_t)(Pcs_Output_Meter.PCS_DC_NBusVol*10.0f);

        Cpu1Ipc_cpu2cm.PCS_DC_BusCurr         = (int16_t)(Pcs_Output_Meter.PCS_DC_BusCurr*10.0f);
        Cpu1Ipc_cpu2cm.PCS_DC_IBUS_N          = (int16_t)(Pcs_Output_Meter.PCS_DC_IBUS_N*10.0f);
        Cpu1Ipc_cpu2cm.Pcs_Gridpllfo          = (int16_t)(Pcs_Gridpllfo*100.0f);
        Cpu1Ipc_cpu2cm.Pcs_Localpllfo         = (int16_t)(Pcs_Localpllfo*100.0f);

        ACRms_VAB_Sum_pu = 0.0f;
        ACRms_VBC_Sum_pu = 0.0f;
        ACRms_VCA_Sum_pu = 0.0f;

        ACRms_VLA_Sum_pu = 0.0f;
        ACRms_VLB_Sum_pu = 0.0f;
        ACRms_VLC_Sum_pu = 0.0f;

        ACRms_VC_Sum_fo = 0.0f;

        ACRms_IINVA_Sum_pu  = 0.0f;
        ACRms_IINVB_Sum_pu  = 0.0f;
        ACRms_IINVC_Sum_pu  = 0.0f;

        DCVolts_Sum_pu   = 0.0f;
        DCVoltsP_Sum_pu  = 0.0f;
        DCVoltsN_Sum_pu  = 0.0f;
        DCCurrent_Sum_pu = 0.0f;
        DCIBUS_Sum_pu    = 0.0f;

        i16_RmsCnt = 0;
    }

}


static float32_t  math_f32_square(float32_t f32_Dat)
{
    float32_t sqr_result;

    sqr_result = f32_Dat*f32_Dat;

    return sqr_result;
}

/*
 * AdcDat 是分压后的电压值
 *  TempDat = (int16_t)((float)AdcDat*0.00457771f)*6+36;
 * */
static int16_t calc_NtcTemp(uint16_t AdcDat)
{
    float temp;
    float Rt = 0;
    //Rp 上拉电阻阻值 10K
    float Rp = 30000.0f;
    float R25 = 5000.0f;//5K 25度
    //T2为25度，折算为开尔文温度
    float T25 = 298.15f;//273.15+25;
    float Bx = 3433.0f;//B25/100
    float Ka = 273.15f;
    float Vol;
    uint16_t u16_temp;
    int16_t i16_tempx10;

    u16_temp = AdcDat;
//    Vol = (float)u16_temp*0.0001373312+0.18f;//3*3.0/65535.0;
    Vol = (float)u16_temp*0.0000457763671875f;//3.0/65535.0;
    Rt = (Vol*Rp)/(5.0f-Vol);//分压电阻Rp 电压5V
    temp = 1.0f/(1/T25+logf(Rt/R25)/Bx)-Ka+0.5f;

    i16_tempx10 = (int16_t)(temp*10.0f);

    return i16_tempx10;
}

static int16_t calc_NtcTemp_1(uint16_t AdcDat)
{
    float temp;
    float Rt = 0;
    //Rp 上拉电阻阻值 10K
    float Rp = 30000.0f;
    float R25 = 10000.0f;// 10k 25度
    //T2为25度，折算为开尔文温度
    float T25 = 298.15f;//273.15+25;
    float Bx = 3950.0f;//B25/100
    float Ka = 273.15f;
    float Vol;
    uint16_t u16_temp;
    int16_t i16_tempx10;

    u16_temp = AdcDat;
//    Vol = (float)u16_temp*0.0001373312+0.18f;//3*3.0/65535.0;
    Vol = (float)u16_temp*0.0000457763671875f;//3.0/65535.0;
    Rt = (Vol*Rp)/(5.0f-Vol);//分压电阻Rp 电压5V
    temp = 1.0f/(1/T25+logf(Rt/R25)/Bx)-Ka+0.5f;

    i16_tempx10 = (int16_t)(temp*10.0f);

    return i16_tempx10;
}



void  obj_PcsOutput_TempCalc(void)
{
    static int32_t  TempBuf[6] = {0};
    static int16_t  TempCnt[6] = {0};
    static int16_t  i = 0;
    int16_t TempDat;

    if(m_st_TimerFlag.u16_b50ms != 1)
        return;
    switch(i)
    {
       case 0:TempDat = (int16_t)calc_NtcTemp(ad_ntc1[0]);break;
       case 1:TempDat = (int16_t)calc_NtcTemp(ad_ntc1[1]);break;
       case 2:TempDat = (int16_t)calc_NtcTemp(ad_ntc1[2]);break;
       case 3:TempDat = (int16_t)calc_NtcTemp(ad_ntc1[3]);break;
       case 4:TempDat = (int16_t)calc_NtcTemp_1(ad_ntc2[0]);break;
       case 5:TempDat = (int16_t)calc_NtcTemp_1(ad_ntc2[2]);break;
       default:break;
    }
    TempBuf[i] += TempDat;
    TempCnt[i]++;
    if(TempCnt[i] >= 3)
    {
        switch(i)
        {
           case 0:Cpu1Ipc_cpu2cm.Temp_igbtN     = TempBuf[0]>>2;break;
           case 1:Cpu1Ipc_cpu2cm.Temp_igbtA     = TempBuf[1]>>2;break;
           case 2:Cpu1Ipc_cpu2cm.Temp_igbtB     = TempBuf[2]>>2;break;
           case 3:Cpu1Ipc_cpu2cm.Temp_igbtC     = TempBuf[3]>>2;break;
           case 4:Cpu1Ipc_cpu2cm.Temp_AmbInlet  = TempBuf[4]>>2;break;
           case 5:Cpu1Ipc_cpu2cm.Temp_AmbOutlet = TempBuf[5]>>2;break;
           default:break;
        }
        TempCnt[i] = 0;
        TempBuf[i] = 0;
    }
    if(++i>3) i = 0;
}





