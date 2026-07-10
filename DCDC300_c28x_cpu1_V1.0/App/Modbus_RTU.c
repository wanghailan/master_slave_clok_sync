//#############################################################################
//
// FILE:   Drv_GPIO.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

#include <Driver/device/device.h>
#include <Driver/device/driverlib.h>
#include <Modbus_RTU.h>

/* CODEMAP_MODBUS
 * Role: RS485 Modbus RTU holding-register interface.
 * Function 03 reads mirrored arrays. Function 06 writes one register and updates live variables.
 * main.c calls upDataHoldingCBReg() every 100 ms to refresh the mirrors.
 */
#include "bsp.h"

extern uint8_t  u8_UID_REGS[32];
extern uint32_t IpcRecvData[10];
extern float32_t  PhaseErr;
/* RTU 应答代码 */
#define RSP_OK              0       /* 成功 */
#define RSP_ERR_CMD         0x01    /* 不支持的功能码 */
#define RSP_ERR_REG_ADDR    0x02    /* 寄存器地址错误 */
#define RSP_ERR_VALUE       0x03    /* 数据值域错误 */
#define RSP_ERR_WRITE       0x04    /* 写入失败 */
/* 硬件版本号 */
#define Hard_Version        HWREGH(0x0BC005)
#define mSoft_Version       HWREGH(0x0BC006)
#define cSoft_Version       HWREGH(0x0BC007)
#define rSoft_Version       HWREGH(0x0BC008)
#define yTime_Version       HWREGH(0x0BC009)
#define dTime_Version       HWREGH(0x0BC00A)


uint16_t    *Mod1_RxBuf;
int16_t     *Mod1_RxCnt;
int16_t     *Mod1_RxTimeOut;

uint16_t    *Mod2_RxBuf;
int16_t     *Mod2_RxCnt;
int16_t     *Mod2_RxTimeOut;

static uint16_t    Mod1_TxBuf[Mod_TXBUF_SIZE];
static int16_t     Mod1_TxCnt;

static uint16_t    Mod2_TxBuf[Mod_TXBUF_SIZE];
static int16_t     Mod2_TxCnt;

static uint16_t   status_RegHoldingBuf_w[REG_STATUS_HOLDING_LEN];//状态寄存器
static uint16_t   control_RegHoldingBuf_w[REG_CONTROL_HOLDING_LEN];//控制寄存器
static uint16_t   fault_RegHoldingBuf_w[REG_FAULT_HOLDING_LEN];//故障寄存器
static uint16_t   param_RegHoldingBuf_w[REG_PARAM_HOLDING_LEN];//参数寄存器
static uint16_t   debug_RegHoldingBuf_w[REG_DEBUG_HOLDING_LEN];//功能使能寄存器
static uint16_t   key_RegHoldingBuf_w[REG_DEBUG_HOLDING_LEN];

    // CRC 高位字节值表
static const uint16_t s_CRCHi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
} ;
// CRC 低位字节值表
const uint16_t s_CRCLo[] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
    0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
    0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
    0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
    0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
    0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
    0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
    0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
    0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
    0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
    0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
    0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
    0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
    0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
    0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
    0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
    0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
    0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};

static void ModbusRtu_AnalyzeApp(int16_t _ch);
/* CODEMAP_MODBUS_READ: read holding registers from one of the mirrored register arrays. */
static void MODS_03H(int16_t _ch);
/* CODEMAP_MODBUS_WRITE: write one holding register through Param_WriteDataHoldingReg(). */
static void MODS_06H(int16_t _ch);
static uint16_t CRC16_Modbus(uint16_t *_pBuf, uint16_t _usLen);
static void MODS_SendWithCRC(int16_t _ch,uint16_t *_pBuf, uint16_t _ucLen);
static void MODS_SendAckErr(int16_t _ch,uint8_t _ucErrCode);
static void MODS_SendAckOk(int16_t _ch);
uint16_t BEBufToUint16(uint8_t *_pBuf);
/* CODEMAP_UPDATE_HOLDING: refresh all Modbus mirror arrays from current runtime variables. */
int16_t upDataHoldingCBReg(void);

/* CODEMAP_STATUS_REGS: status registers 0..39, mainly live volt/current/power/state/fault data. */
static int16_t status_upDataHoldingCBReg(void);
/* CODEMAP_CONTROL_REGS: control registers 100..123, mirrored from DcDc_gParam. */
static int16_t Ctrl_upDataHoldingCBReg(void);
/* CODEMAP_FAULT_REGS: fault registers 200..219, fault words plus protection thresholds. */
static int16_t Fault_upDataHoldingCBReg(void);
/* CODEMAP_PARAM_REGS: parameter registers 300..349, RTC/version/calibration values. */
static int16_t Param_upDataHoldingCBReg(void);
/* CODEMAP_PARAM_WRITE
 * Central write dispatcher. This is where Modbus writes become StartEn, WorkMode, CtrlMode,
 * relay debug commands, Pwm_StartEn, fault reset or EEPROM-backed parameter updates.
 */
static int16_t Param_WriteDataHoldingReg(uint16_t reg,int16_t value);

extern uint8_t  u8_AES_READ[32];
void ModbusRtu1_Init(void)
{
    Mod1_RxBuf     = SCIC_RxBuf;
    Mod1_RxCnt     = &SCIC_RxCnt;
    Mod1_RxTimeOut = &SCIC_RxTimeOut;
//    Drv_SCICInit();
}

void ModbusRtu2_Init(void)
{
    Mod2_RxBuf     = SCID_RxBuf;
    Mod2_RxCnt     = &SCID_RxCnt;
    Mod2_RxTimeOut = &SCID_RxTimeOut;
//    Drv_SCIDInit();
}

int16_t ModbusRtu1_Poll(void)
{
    uint16_t addr;
    uint16_t crc1;

    /* 超过3.5个字符时间后 通知主程序开始解码 */
    if (*Mod1_RxTimeOut < ModRxTimeOut)
    {
        return -1;                             /* 没有超时，继续接收。不要清零 g_tModS.RxCount */
    }

    *Mod1_RxTimeOut = 0;                     /* 清标志 */

    if (*Mod1_RxCnt < 4)                /* 接收到的数据小于4个字节就认为错误，地址（8bit）+指令（8bit）+操作寄存器（16bit） */
    {
        goto err_ret;
    }

    /* 计算CRC校验和，这里是将接收到的数据包含CRC16值一起做CRC16，结果是0，表示正确接收 */
    crc1 = CRC16_Modbus(Mod1_RxBuf, *Mod1_RxCnt);
    if (crc1 != 0)
    {
        goto err_ret;
    }

    /* 站地址 (1字节） */
    addr = Mod1_RxBuf[0];                /* 第1字节 站号 */
    if (addr != RTU_ADDR)                   /* 判断主机发送的命令地址是否符合 */
    {
        goto err_ret;
    }
    /* 分析应用层协议 */
    ModbusRtu_AnalyzeApp(1);
err_ret:
    *Mod1_RxCnt = 0;                    /* 必须清零计数器，方便下次帧同步 */
    return 0;
}

int16_t ModbusRtu2_Poll(void)
{
    uint16_t addr;
    uint16_t crc1;

    /* 超过3.5个字符时间后 通知主程序开始解码 */
    if (*Mod2_RxTimeOut < ModRxTimeOut)
    {
        return -1;                             /* 没有超时，继续接收。不要清零 g_tModS.RxCount */
    }

    *Mod2_RxTimeOut = 0;                     /* 清标志 */

    if (*Mod2_RxCnt < 4)                /* 接收到的数据小于4个字节就认为错误，地址（8bit）+指令（8bit）+操作寄存器（16bit） */
    {
        goto err_ret;
    }

    /* 计算CRC校验和，这里是将接收到的数据包含CRC16值一起做CRC16，结果是0，表示正确接收 */
    crc1 = CRC16_Modbus(Mod2_RxBuf, *Mod2_RxCnt);
    if (crc1 != 0)
    {
        goto err_ret;
    }

    /* 站地址 (1字节） */
    addr = Mod2_RxBuf[0];                /* 第1字节 站号 */
    if (addr != RTU_ADDR)                   /* 判断主机发送的命令地址是否符合 */
    {
        goto err_ret;
    }
    /* 分析应用层协议 */
    ModbusRtu_AnalyzeApp(2);
err_ret:
    *Mod2_RxCnt = 0;                    /* 必须清零计数器，方便下次帧同步 */
    return 0;
}

/*
*********************************************************************************************************
*   函 数 名: MODS_AnalyzeApp
*   功能说明: 分析应用层协议
*   形    参: 无
*   返 回 值: 无
*********************************************************************************************************
*/
static void ModbusRtu_AnalyzeApp(int16_t _ch)
{
    uint16_t *Mod_RxBuf;

    if(_ch == 1)
        Mod_RxBuf = Mod1_RxBuf;
    else
        Mod_RxBuf = Mod2_RxBuf;
//    switch (Mod1_RxBuf[1])                  /* 第2个字节 功能码 */
//    {
////        case 0x01:                          /* 读取线圈状态*/
////            break;
////
////        case 0x02:                          /* 读取输入状态*/
////            break;
//
//        case 0x03:                          /* 读取保持寄存器（此例程存在g_tVar中）*/
//            MODS_03H();
//            break;
//
////        case 0x04:                          /* 读取输入寄存器*/
////            break;
////
////        case 0x05:                          /* 强制单线圈*/
////            break;
//
//        case 0x06:                          /* 写单个保存寄存器*/
//            MODS_06H();
//            break;
//
////        case 0x10:                          /* 写多个保存寄存器*/
////            break;
//
//        default:
//            MODS_SendAckErr(RSP_ERR_CMD);   /* 告诉主机命令错误 */
//            break;
//    }
    if(Mod_RxBuf[1] == 0x03) //读取保持寄存
        MODS_03H(_ch);
    else if(Mod_RxBuf[1] == 0x06)//写单个保存寄存器
        MODS_06H(_ch);
    else
        MODS_SendAckErr(_ch,RSP_ERR_CMD);   /* 告诉主机命令错误 */
}

/*
*********************************************************************************************************
*   函 数 名: MODS_03H
*   功能说明: 读取保持寄存器 在一个或多个保持寄存器中取得当前的二进制值
*   形    参: 无
*   返 回 值: 无
*********************************************************************************************************
*/
/* CODEMAP_MODBUS_READ: read holding registers from one of the mirrored register arrays. */
static void MODS_03H(int16_t _ch)
{
    int16_t  RspCode;
    uint16_t regAddr;//读寄存器开始地址
    uint16_t regNum;//读寄存器寄存器数
    int16_t  iRegIndex;
    uint16_t *regAddrBegin;
    uint16_t i;
    uint16_t *Mod_RxBuf;
    int16_t  *Mod_RxCnt;
    uint16_t *Mod_TxBuf;
    int16_t  *Mod_TxCnt;

    if(_ch == 1)
    {
        Mod_RxBuf = Mod1_RxBuf;
        Mod_RxCnt = Mod1_RxCnt;
        Mod_TxBuf = Mod1_TxBuf;
        Mod_TxCnt = &Mod1_TxCnt;
    }
    else
    {
        Mod_RxBuf = Mod2_RxBuf;
        Mod_RxCnt = Mod2_RxCnt;
        Mod_TxBuf = Mod2_TxBuf;
        Mod_TxCnt = &Mod2_TxCnt;
    }

    RspCode = RSP_OK;

    /** 第1步： 判断接到指定个数数据 ===============================================================*/
    /* 地址（8bit）+指令（8bit）+寄存器起始地址高低字节（16bit）+寄存器个数（16bit）+ CRC16 */
    if (*Mod_RxCnt != 8)                               /* 03H命令必须是8个字节 */
    {
        RspCode = RSP_ERR_VALUE;                    /* 数据值域错误 */
        goto err_ret;
    }

    /** 第2步： 数据解析 ===========================================================================*/
    /* 数据是大端，要转换为小端 */
    regAddr = BEBufToUint16(&Mod_RxBuf[2]);                 /* 寄存器号 */
    regNum  = BEBufToUint16(&Mod_RxBuf[4]);                 /* 寄存器个数 */

    if( ((int16_t)regAddr >= REG_STATUS_HOLDING_START)&&( regAddr + regNum <= REG_STATUS_HOLDING_START + REG_STATUS_HOLDING_LEN ))
    {
        /* 读取的数据个数要在范围内 */
        if (regNum > REG_STATUS_HOLDING_LEN)
        {
            RspCode = RSP_ERR_VALUE;                    /* 数据值域错误 */
            goto err_ret;
        }
        iRegIndex    = (int16_t)( regAddr - REG_STATUS_HOLDING_START);
        regAddrBegin = &status_RegHoldingBuf_w[iRegIndex];
    }
    else  if( ((int16_t)regAddr >= REG_CONTROL_HOLDING_START)&&( regAddr + regNum <= REG_CONTROL_HOLDING_START + REG_CONTROL_HOLDING_LEN ))
    {
        /* 读取的数据个数要在范围内 */
        if (regNum > REG_CONTROL_HOLDING_LEN)
        {
            RspCode = RSP_ERR_VALUE;                    /* 数据值域错误 */
            goto err_ret;
        }
        iRegIndex    = (int16_t)( regAddr - REG_CONTROL_HOLDING_START);
        regAddrBegin = &control_RegHoldingBuf_w[iRegIndex];
    }
    else  if( ((int16_t)regAddr >= REG_FAULT_HOLDING_START)&&( regAddr + regNum <= REG_FAULT_HOLDING_START + REG_FAULT_HOLDING_LEN ))
    {
        /* 读取的数据个数要在范围内 */
        if (regNum > REG_FAULT_HOLDING_LEN)
        {
            RspCode = RSP_ERR_VALUE;                    /* 数据值域错误 */
            goto err_ret;
        }
        iRegIndex    = (int16_t)( regAddr - REG_FAULT_HOLDING_START);
        regAddrBegin = &fault_RegHoldingBuf_w[iRegIndex];
    }
    else  if( ((int16_t)regAddr >= REG_PARAM_HOLDING_START)&&( regAddr + regNum <= REG_PARAM_HOLDING_START + REG_PARAM_HOLDING_LEN ))
    {
        /* 读取的数据个数要在范围内 */
        if (regNum > REG_PARAM_HOLDING_LEN)
        {
            RspCode = RSP_ERR_VALUE;                    /* 数据值域错误 */
            goto err_ret;
        }
        iRegIndex    = (int16_t)( regAddr - REG_PARAM_HOLDING_START);
        regAddrBegin = &param_RegHoldingBuf_w[iRegIndex];
    }
    else  if( ((int16_t)regAddr >= REG_DEBUG_HOLDING_START)&&( regAddr + regNum <= REG_DEBUG_HOLDING_START + REG_DEBUG_HOLDING_LEN ))
    {
        /* 读取的数据个数要在范围内 */
        if (regNum > REG_DEBUG_HOLDING_LEN)
        {
            RspCode = RSP_ERR_VALUE;                    /* 数据值域错误 */
            goto err_ret;
        }
        iRegIndex    = (int16_t)( regAddr - REG_DEBUG_HOLDING_START);
        regAddrBegin = &debug_RegHoldingBuf_w[iRegIndex];
    }
    else  if( ((int16_t)regAddr >= REG_KEY_HOLDING_START)&&( regAddr + regNum <= REG_KEY_HOLDING_START + REG_Key_HOLDING_LEN ))
    {
        /* 读取的数据个数要在范围内 */
        if (regNum > REG_Key_HOLDING_LEN)
        {
            RspCode = RSP_ERR_VALUE;                    /* 数据值域错误 */
            goto err_ret;
        }
        iRegIndex    = (int16_t)( regAddr - REG_KEY_HOLDING_START);
        regAddrBegin = &key_RegHoldingBuf_w[iRegIndex];
    }
    else
    {
        RspCode = RSP_ERR_REG_ADDR;     /* 寄存器地址错误 */
        goto err_ret;
    }
    /** 第3步： 应答回复 =========================================================================*/
err_ret:
    if (RspCode == RSP_OK)                           /* 正确应答 */
    {
        *Mod_TxCnt = 0;
        Mod_TxBuf[(*Mod_TxCnt)++] = Mod_RxBuf[0]; /* 返回从机地址 */
        Mod_TxBuf[(*Mod_TxCnt)++] = Mod_RxBuf[1]; /* 返回从机指令 */
        Mod_TxBuf[(*Mod_TxCnt)++] = regNum * 2;    /* 返回字节数 */

        for (i = 0; i < regNum; i++)  /* 返回数据*/
        {
            Mod_TxBuf[(*Mod_TxCnt)++] = (uint16_t)(regAddrBegin[i] >> 8 );
            Mod_TxBuf[(*Mod_TxCnt)++] = (uint16_t)(regAddrBegin[i] & 0xFF );
        }

        MODS_SendWithCRC(_ch,Mod_TxBuf, *Mod_TxCnt);   /* 发送正确应答 */
    }
    else
    {
        MODS_SendAckErr(_ch,RspCode);                   /* 发送错误应答 */
    }
}

/*
*********************************************************************************************************
*   函 数 名: MODS_06H
*   功能说明: 写单个寄存器
*   形    参: 无
*   返 回 值: 无
*********************************************************************************************************
*/
/* CODEMAP_MODBUS_WRITE: write one holding register through Param_WriteDataHoldingReg(). */
static void MODS_06H(int16_t _ch)
{
    /*写保持寄存器。注意06指令只能操作单个保持寄存器，16指令可以设置单个或多个保持寄存器*/
    int16_t  RspCode;
    uint16_t reg;
    int16_t value;
    uint16_t *Mod_RxBuf;
    uint16_t *Mod_RxCnt;

    RspCode = RSP_OK;

    if(_ch == 1)
    {
        Mod_RxBuf = Mod1_RxBuf;
        Mod_RxCnt = Mod1_RxCnt;
    }
    else
    {
        Mod_RxBuf = Mod2_RxBuf;
        Mod_RxCnt = Mod2_RxCnt;
    }

    /** 第1步： 判断接到指定个数数据 ===============================================================*/
    /* 地址（8bit）+指令（8bit）+寄存器起始地址高低字节（16bit）+寄存器个数（16bit）+ CRC16 */
    if (*Mod_RxCnt != 8)
    {
        RspCode = RSP_ERR_VALUE;        /* 数据值域错误 */
        goto err_ret;
    }

    /** 第2步： 数据解析 ===========================================================================*/
    /* 数据是大端，要转换为小端 */
    reg   = BEBufToUint16(&Mod_RxBuf[2]);   /* 寄存器号 */
    value = BEBufToUint16(&Mod_RxBuf[4]);   /* 寄存器值 */

    if (Param_WriteDataHoldingReg(reg,value) == 0)    /* 该函数会把写入的值存入寄存器 */
    {
        ;
    }
    else
    {
        RspCode = RSP_ERR_REG_ADDR;     /* 寄存器地址错误 */
    }

    /** 第3步： 应答回复 =========================================================================*/
err_ret:
    if (RspCode == RSP_OK)              /* 正确应答 */
    {
        MODS_SendAckOk(_ch);
    }
    else
    {
        MODS_SendAckErr(_ch,RspCode);       /* 告诉主机命令错误 */
    }
}


/*
*********************************************************************************************************
*   函 数 名: CRC16_Modbus
*   功能说明: 计算CRC。 用于Modbus协议。
*   形    参: _pBuf : 参与校验的数据
*             _usLen : 数据长度
*   返 回 值: 16位整数值。 对于Modbus ，此结果高字节先传送，低字节后传送。
*
*   所有可能的CRC值都被预装在两个数组当中，当计算报文内容时可以简单的索引即可；
*   一个数组包含有16位CRC域的所有256个可能的高位字节，另一个数组含有低位字节的值；
*   这种索引访问CRC的方式提供了比对报文缓冲区的每一个新字符都计算新的CRC更快的方法；
*
*  注意：此程序内部执行高/低CRC字节的交换。此函数返回的是已经经过交换的CRC值；也就是说，该函数的返回值可以直接放置
*        于报文用于发送；
*********************************************************************************************************
*/
static uint16_t CRC16_Modbus(uint16_t *_pBuf, uint16_t _usLen)
{
    uint16_t ucCRCHi = 0x00FF; /* 高CRC字节初始化 */
    uint16_t ucCRCLo = 0x00FF; /* 低CRC 字节初始化 */
    uint16_t usIndex;  /* CRC循环中的索引 */

    while (_usLen--)
    {
        usIndex = ucCRCHi ^ *_pBuf++; /* 计算CRC */
        ucCRCHi = ucCRCLo ^ s_CRCHi[usIndex];
        ucCRCLo = s_CRCLo[usIndex];
    }
    return ((uint16_t)ucCRCHi << 8 | ucCRCLo);
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
uint16_t BEBufToUint16(uint8_t *_pBuf)
{
    return (((uint16_t)_pBuf[0] << 8) | _pBuf[1]);
}

/*
*********************************************************************************************************
*   函 数 名: MODS_SendWithCRC
*   功能说明: 发送一串数据, 自动追加2字节CRC
*   形    参: _pBuf 数据；
*             _ucLen 数据长度（不带CRC）
*   返 回 值: 无
*********************************************************************************************************
*/
static void MODS_SendWithCRC(int16_t _ch,uint16_t *_pBuf, uint16_t _ucLen)
{
    uint16_t crc;

    crc = CRC16_Modbus(_pBuf, _ucLen);
    _pBuf[_ucLen++] = crc >> 8;
    _pBuf[_ucLen++] = crc;

    if(_ch == 1)
        Drv_SCIC_WriteDataBuf(_ucLen,_pBuf);
    else
        Drv_SCID_WriteDataBuf(_ucLen,_pBuf);
}

/*
*********************************************************************************************************
*   函 数 名: MODS_SendAckErr
*   功能说明: 发送错误应答
*   形    参: _ucErrCode : 错误代码
*   返 回 值: 无
*********************************************************************************************************
*/
static void MODS_SendAckErr(int16_t _ch,uint8_t _ucErrCode)
{
    uint16_t txbuf[5];//留2位给CRC

    if(_ch == 1)
    {
        txbuf[0] = Mod1_RxBuf[0];                    /* 485地址 */
        txbuf[1] = Mod1_RxBuf[1] | 0x80;             /* 异常的功能码 */
        txbuf[2] = _ucErrCode;                       /* 错误代码(01,02,03,04) */
    }
    else
    {
        txbuf[0] = Mod2_RxBuf[0];                    /* 485地址 */
        txbuf[1] = Mod2_RxBuf[1] | 0x80;             /* 异常的功能码 */
        txbuf[2] = _ucErrCode;                       /* 错误代码(01,02,03,04) */
    }
    MODS_SendWithCRC(_ch,txbuf,3);

}

/*
*********************************************************************************************************
*   函 数 名: MODS_SendAckOk
*   功能说明: 发送正确的应答.
*   形    参: 无
*   返 回 值: 无
*********************************************************************************************************
*/
static void MODS_SendAckOk(int16_t _ch)
{
    uint16_t txbuf[8];
    uint16_t i;

    for (i = 0; i < 6; i++)
    {
        if(_ch == 1)
            txbuf[i] = Mod1_RxBuf[i];
        else
            txbuf[i] = Mod2_RxBuf[i];
    }
    MODS_SendWithCRC(_ch,txbuf,6);
}

//状态寄存器数据更新 0~32
/* CODEMAP_STATUS_REGS: status registers 0..39, mainly live volt/current/power/state/fault data. */
static int16_t status_upDataHoldingCBReg(void)
{
    //低压端口侧电压(0.1V)
    status_RegHoldingBuf_w[0]   = (int16_t)(DC_OutMeter.DC_VBus_Mean*10.0f);
    //低压软起电压(0.1V)
    status_RegHoldingBuf_w[1]   = (int16_t)(DC_OutMeter.DC_VBusS_Mean*10.0f);
    //低压侧电流(0.1A)
    status_RegHoldingBuf_w[2]   = (int16_t)(DC_OutMeter.DC_IBus_Mean*10.0f);
    //低压侧功率(0.01KW)
    status_RegHoldingBuf_w[3]   = (int16_t)(DC_OutMeter.DC_PowerBus*100.0f);
    //高压端口电压(0.1V)
    status_RegHoldingBuf_w[4]   = (int16_t)(DC_OutMeter.DC_VBat_Mean*10.0f);
    //高压软起电压(0.1V)
    status_RegHoldingBuf_w[5]   = (int16_t)(DC_OutMeter.DC_VBatS_Mean*10.0f);
    //高压侧电流(0.1V)
    status_RegHoldingBuf_w[6]   = (int16_t)(DC_OutMeter.DC_IBat_Mean*10.0f);
    //高压侧功率(0.01KW)
    status_RegHoldingBuf_w[7]   = (int16_t)(DC_OutMeter.DC_PowerBat*100.0f);
    //高压上半幅电压(0.1V)
    status_RegHoldingBuf_w[8]   = 0;//(int16_t)(DC_OutMeter.DC_VBatP_Mean*10.0f)
    //高压下半幅电压(0.1V)
    status_RegHoldingBuf_w[9]   = 0;//(int16_t)(DC_OutMeter.DC_VBatN_Mean*10.0f)
    //单元1输出电流(0.1A)
    status_RegHoldingBuf_w[10]  = (int16_t)(DC_OutMeter.DC_IL1_Mean*10.0f);
    //单元2输出电流(0.1A)
    status_RegHoldingBuf_w[11]  = (int16_t)(DC_OutMeter.DC_IL2_Mean*10.0f);
    //IGBT模块1温度(0.1度)
    status_RegHoldingBuf_w[12]  = (int16_t)(DC_OutMeter.DC_IL3_Mean*10.0f);
    //IGBT模块2温度(0.1度)
    status_RegHoldingBuf_w[13]  = (int16_t)DC_OutMeter.DC_IGBT_Temp1;
    //IGBT模块3温度(0.1度)
    status_RegHoldingBuf_w[14]  = (int16_t)DC_OutMeter.DC_IGBT_Temp2;
    //IGBT模块4温度(0.1度)
    status_RegHoldingBuf_w[15]  = (int16_t)DC_OutMeter.DC_IGBT_Temp3;
    //IGBT模块最高温度(0.1度)
    status_RegHoldingBuf_w[16]  = 0;
    //充电电量万度
    status_RegHoldingBuf_w[17]  = (int16_t)(DC_OutMeter.DC_IGBT_TempMax * 10.0f);
    //充电电量度
    status_RegHoldingBuf_w[18]  = DC_OutMeter.DC_HChargeKWH;
    //放电电量万度
    status_RegHoldingBuf_w[19]  = DC_OutMeter.DC_LChargeKWH;
    //放电电量度
    status_RegHoldingBuf_w[20]  = DC_OutMeter.DC_HdisChargeKWH;
    //累计运行时间 分钟
    status_RegHoldingBuf_w[21]  = DC_OutMeter.DC_LdisChargeKWH;
    //控制状态
    status_RegHoldingBuf_w[22]  = DC_OutMeter.DC_RUN_TimeM;
    //转换效率(%)
    status_RegHoldingBuf_w[23]  = DcDc_gParam.DcCtrlState;
    //允许开机状态(0:允许开机,1:不允许开机)
    status_RegHoldingBuf_w[24]  = (int16_t)(DC_OutMeter.DC_transfEffi*10000.0f);//g_param.StartEnable;
    //控制比例
    status_RegHoldingBuf_w[25]  = FaultStatus.globalFault;//(int16_t)(DC_OutMeter.DC_Vdc7_Mean * 10.0f)
    //工作模式
    status_RegHoldingBuf_w[26]  = 0;//(int16_t)(DC_OutMeter.DC_Vdc8_Mean * 10.0f)
    //电池电压
    status_RegHoldingBuf_w[27]  = 0;//CtrlStatus;
    //电池SOC
    status_RegHoldingBuf_w[28]  = 0;
    //输出电阻阻值
    status_RegHoldingBuf_w[29]  = 0;//tCla2Cpu.i16_ResOutget;

    return 0;
}

//控制寄存器数据更新 100~123
/* CODEMAP_CONTROL_REGS: control registers 100..123, mirrored from DcDc_gParam. */
static int16_t Ctrl_upDataHoldingCBReg(void)
{
    //启动(1)/停机(0)
    control_RegHoldingBuf_w[0]  = DcDc_gParam.StartEn;
    //离网(0)/并网(1)
    control_RegHoldingBuf_w[1]  = DcDc_gParam.WorkMode;
    //并网工作模式(//0:开环,1:恒压,2:恒流,3:恒功率,4:恒阻)
    control_RegHoldingBuf_w[2]  = DcDc_gParam.CtrlMode;
    //本地(0)/远程(1)
    control_RegHoldingBuf_w[3]  = DcDc_gParam.LocalRemote;
    //恒压设定(0~1800.0V)
    control_RegHoldingBuf_w[4]  = DcDc_gParam.DcConstVolSet;
    //恒流设定(0~500.0A)
    control_RegHoldingBuf_w[5]  = DcDc_gParam.DcConstCurrSet;
    //恒功率设定(0~400.0KW)
    control_RegHoldingBuf_w[6]  = DcDc_gParam.DcConstPowerSet;
    //恒阻值设定(0~1000.0R)
    control_RegHoldingBuf_w[7]  = DcDc_gParam.DcConstResSet;
    //高压侧最大电流(0~400.0A)
    control_RegHoldingBuf_w[8]  = DcDc_gParam.DcBatMaxCurrent;
    //低压侧最大电流(0~400.0A)
    control_RegHoldingBuf_w[9]  = DcDc_gParam.DcBusMaxCurrent;
    //高压侧最大电压(0~2000.0V)
    control_RegHoldingBuf_w[10]  = DcDc_gParam.DcBatMaxVol;
    //低压侧最大电压(0~2000.0V)
    control_RegHoldingBuf_w[11]  = DcDc_gParam.DcBusMaxVol;
    //电量清零(0~1)
    control_RegHoldingBuf_w[12]  = 0;
    //浮充电压设定(0~1000.0V)
    control_RegHoldingBuf_w[13]  = DcDc_gParam.MPPT_En;//Pcs_gParam.DcFloatChargeVol;
    //最大充电电流(0~300.0A)
    control_RegHoldingBuf_w[14]  = DcDc_gParam.MPPT_VdcMax;//Pcs_gParam.DcMaxChargeCurrent;
    //最大放电电流(0~300.0A)
    control_RegHoldingBuf_w[15]  = DcDc_gParam.MPPT_VdcMin;//Pcs_gParam.DcMaxDisChargeCurrent;
    //均充转浮充电流(0~300.0A)
    control_RegHoldingBuf_w[16]  = 0;//Pcs_gParam.DcEqual2FloatCurrent;
    //电池电压保护上限(200.0~1000.0V)
    control_RegHoldingBuf_w[17]  = 0;//Pcs_gParam.BattHighVolSet;
    //电池电压保护下限(200.0~1000.0V)
    control_RegHoldingBuf_w[18]  = 0;//Pcs_gParam.BattLowVolSet;
    //EOD(终止放电)电压(200.0~1000.0V)
    control_RegHoldingBuf_w[19]  = 0;//Pcs_gParam.BattEODVol;
    //并网DOD(放电深度)(0~100.0%)
    control_RegHoldingBuf_w[20]  = 0;//Pcs_gParam.DODGridConnect;
    //离网DOD(放电深度)(0~100.0%)
    control_RegHoldingBuf_w[21]  = 0;//Pcs_gParam.DODDisGridConnect;
    //接线体制(0~1)
    control_RegHoldingBuf_w[22]  = 0;//Pcs_gParam.pcsLineMode;
    //电量清零(0~1)
    control_RegHoldingBuf_w[23]  = 0;
    return 0;
}

//故障处理寄存器数据更新 200~210
/* CODEMAP_FAULT_REGS: fault registers 200..219, fault words plus protection thresholds. */
static int16_t Fault_upDataHoldingCBReg(void)
{
    //告警1
    fault_RegHoldingBuf_w[0]  = FaultStatus.DCDC_Waring1.DCDC_Waring1_All;
    //告警2
    fault_RegHoldingBuf_w[1]  = FaultStatus.DCDC_Waring2.DCDC_Waring2_All;
    //故障1
    fault_RegHoldingBuf_w[2]  = FaultStatus.DCDC_Fault1.DCDC_Fault1_All;
    //故障2
    fault_RegHoldingBuf_w[3]  = FaultStatus.DCDC_Fault2.DCDC_Fault2_All;
    //故障3
    fault_RegHoldingBuf_w[4]  = FaultStatus.DCDC_Fault3.DCDC_Fault3_All;
    //故障4
    fault_RegHoldingBuf_w[5]  = FaultStatus.DCDC_Fault4.DCDC_Fault4_All;
    //故障5
    fault_RegHoldingBuf_w[6]  = FaultStatus.DCDC_HFault.DCDC_HFault_All;
    //故障清除
    fault_RegHoldingBuf_w[7]  =  0;
    //高压侧过压保护阈值(0~2000.0V）
    fault_RegHoldingBuf_w[8]  = DcDc_Protect_Threshold.Bat_OverVol_Value;
    //高压欠压保护阈值(0~2000.0V)
    fault_RegHoldingBuf_w[9]  = DcDc_Protect_Threshold.Bat_UnderVol_Value;
    //低压过压保护阈值(0~2000.0V)
    fault_RegHoldingBuf_w[10]  = DcDc_Protect_Threshold.Bus_OverVol_Value;
    //低压欠压保护阈值(0~2000.0V)
    fault_RegHoldingBuf_w[11]  = DcDc_Protect_Threshold.Bus_UnderVol_Value;
    //高压过流保护阈值(0~400.0A)
    fault_RegHoldingBuf_w[12]  = DcDc_Protect_Threshold.Bat_OverCurr_Value;
    //低压过流保护阈值(0~400.0A)
    fault_RegHoldingBuf_w[13]  = DcDc_Protect_Threshold.Bus_OverCurr_Value;
    //电流不平衡故障阈值(0~100.0A)
    fault_RegHoldingBuf_w[14]  = DcDc_Protect_Threshold.Vol_Unbance_Value;
    //电流不平衡故障阈值(0~100.0A)
    fault_RegHoldingBuf_w[15]  = DcDc_Protect_Threshold.Curr_Unbance_Value;
    //过温保护阈值(0~200.0°)
    fault_RegHoldingBuf_w[16]  = DcDc_Protect_Threshold.Temp_Over_Value;
    //高压短路电压保护阈值(0~200.0V)
    fault_RegHoldingBuf_w[17]  = DcDc_Protect_Threshold.Bat_VShort_Value;
    //低压短路电压保护阈值(0~200.0V)
    fault_RegHoldingBuf_w[18]  = DcDc_Protect_Threshold.Bus_VShort_Value;
//    fault_RegHoldingBuf_w[8]  =  epwm1TZIntCount;
//    fault_RegHoldingBuf_w[9]  = GPIO_readPin(68);
//    fault_RegHoldingBuf_w[10]  = XBAR_getInputFlagStatus(XBAR_INPUT_FLG_INPUT1);
    return 0;
}

//参数设置寄存器数据更新 300~328
/* CODEMAP_PARAM_REGS: parameter registers 300..349, RTC/version/calibration values. */
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
    //低压侧电压校准(80.00%~120.00%)
    param_RegHoldingBuf_w[7]   = g_calibratParam.VBus_Calibrat;
    //低压侧软起电压校准(80.00%~120.00%)
    param_RegHoldingBuf_w[8]   = g_calibratParam.VBusS_Calibrat;
    //高压侧电压校准(80.00%~120.00%)
    param_RegHoldingBuf_w[9]   = g_calibratParam.VBat_Calibrat;
    //高压侧软起电压校准(80.00%~120.00%)
    param_RegHoldingBuf_w[10]  = g_calibratParam.VBatS_Calibrat;
    //高压侧正电压校准
    param_RegHoldingBuf_w[11]  = g_calibratParam.VBatP_Calibrat;
    //高压侧负电压校准
    param_RegHoldingBuf_w[12]  = g_calibratParam.VBatN_Calibrat;
    //低压侧电流校准
    param_RegHoldingBuf_w[13]  = g_calibratParam.IBus_Calibrat;
    //高压侧电流校准
    param_RegHoldingBuf_w[14]  = g_calibratParam.IBat_Calibrat;
    //单元1电流校准
    param_RegHoldingBuf_w[15]  = g_calibratParam.IL1_Calibrat;
    //单元2电流校准
    param_RegHoldingBuf_w[16]  = g_calibratParam.IL2_Calibrat;
    //单元3电流校准
    param_RegHoldingBuf_w[17]  = g_calibratParam.IL3_Calibrat;
    //低压侧电流偏置校准
    param_RegHoldingBuf_w[18]  = g_calibratParam.IBus_offset;
    //高压侧电流偏置校准
    param_RegHoldingBuf_w[19]  = g_calibratParam.IBat_offset;
    //单元1电流偏置校准
    param_RegHoldingBuf_w[20]  = g_calibratParam.IL1_offset;
    //单元2电流偏置校准
    param_RegHoldingBuf_w[21]  = g_calibratParam.IL2_offset;
    //单元3电流偏置校准
    param_RegHoldingBuf_w[22]  = g_calibratParam.IL3_offset;//g_calibratParam.VInvB_Calibrat;
    //高压端口电压偏置
    param_RegHoldingBuf_w[23]  = g_calibratParam.VBat_offset;//g_calibratParam.VInvC_Calibrat;
    //高压软起电压偏置
    param_RegHoldingBuf_w[24]  = g_calibratParam.VBatS_offset;//g_calibratParam.IN_Calibrat;
    //低压端口电压偏置
    param_RegHoldingBuf_w[25]  = g_calibratParam.VBus_offset;//g_calibratParam.Idc_offset;
    //低压软起电压偏置
    param_RegHoldingBuf_w[26]  = g_calibratParam.VBusS_offset;//g_calibratParam.Iac_Aoffset;
    //1.5V偏置电压校准
    param_RegHoldingBuf_w[27]  = g_calibratParam.Ref1v5_Calibrat;//g_calibratParam.Iac_Boffset;
    //硬件版本号
    param_RegHoldingBuf_w[28]  = Hard_Version;//g_calibratParam.Iac_Coffset;
    //软件主版本号 功能或架构有大变动时修改
    param_RegHoldingBuf_w[29]  = mSoft_Version;//g_calibratParam.Ilac_Aoffset;
    //软件子版本号 功能有一定增加时修改
    param_RegHoldingBuf_w[30]  = cSoft_Version;//g_calibratParam.Ilac_Boffset;
    //软件修订版本号 bug修复或小改动时修改
    param_RegHoldingBuf_w[31]  = rSoft_Version;//g_calibratParam.Ilac_Coffset;
    //软件版本发布年月
    param_RegHoldingBuf_w[32]  = yTime_Version;//g_calibratParam.IdcN_offset;
    //软件版本发布日时
    param_RegHoldingBuf_w[33]  = dTime_Version;//g_calibratParam.IdcN_offset;
    return 0;
}

//参数设置寄存器数据更新 400~422
/* CODEMAP_DEBUG_REGS: debug registers 400..431, relay/PWM/debug/gain visibility and control. */
static int16_t debug_upDataHoldingCBReg(void)
{
    //低压侧软起继电器
    debug_RegHoldingBuf_w[0]   = DcDc_gParam.RelaySt_DcBusSoft;
    //低压侧主继电器
    debug_RegHoldingBuf_w[1]   = DcDc_gParam.RelaySt_DcBusMaster;
    //高压侧软起继电器
    debug_RegHoldingBuf_w[2]   = DcDc_gParam.RelaySt_DcBatSoft;
    //高压侧主继电器
    debug_RegHoldingBuf_w[3]   = DcDc_gParam.RelaySt_DcBatMaster;
    //开环闭环模式
    debug_RegHoldingBuf_w[4]   = 0;
    //发波控制
    debug_RegHoldingBuf_w[5]   = DcDc_gParam.Pwm_StartEn;
    //风机启动控制
    debug_RegHoldingBuf_w[6]   = DcDc_gParam.RelaySt_FanCtrl;
    //
    debug_RegHoldingBuf_w[7]   = DcDc_gParam.PwmRatio3;
    //
    debug_RegHoldingBuf_w[8]   = DcDc_gParam.RelaySt_WorkLed;
    //
    debug_RegHoldingBuf_w[9]   = DcDc_gParam.RelaySt_FaultLed;
    //软启动使能
    debug_RegHoldingBuf_w[10]  = DcDc_gParam.SoftStart_En;
    //调试模式
    debug_RegHoldingBuf_w[11]  = DcDc_gParam.debugMode;
    //电流外环使能
    debug_RegHoldingBuf_w[12]  = DcDc_gParam.OutLoop_En;
    //交错单元1控制比
    debug_RegHoldingBuf_w[13]  = DcDc_gParam.PwmRatio1;
    //交错单元2控制比
    debug_RegHoldingBuf_w[14]  = DcDc_gParam.PwmRatio2;
    //Buck电压外环Kp
    debug_RegHoldingBuf_w[15]  = DcDc_gParam.VBuck_Kp;
    //Buck电压外环Ki
    debug_RegHoldingBuf_w[16]  = DcDc_gParam.VBuck_Ki;
    //Buck电流内环Kp
    debug_RegHoldingBuf_w[17]  = DcDc_gParam.IdcBuck_Kp;
    //Buck电流内环Ki
    debug_RegHoldingBuf_w[18]  = DcDc_gParam.IdcBuck_Ki;
    //Buck电流外环Kp
    debug_RegHoldingBuf_w[19]  = DcDc_gParam.IBuck_Kp;
    //Buck电流外环Ki
    debug_RegHoldingBuf_w[20]  = DcDc_gParam.IBuck_Ki;
    //Boost电压外环Kp
    debug_RegHoldingBuf_w[21]  = DcDc_gParam.VBoost_Kp;
    //Boost电压外环Ki
    debug_RegHoldingBuf_w[22]  = DcDc_gParam.VBoost_Ki;
    //Boost电流内环Kp
    debug_RegHoldingBuf_w[23]  = DcDc_gParam.IdcBoost_Kp;
    //Boost电流内环Ki
    debug_RegHoldingBuf_w[24]  = DcDc_gParam.IdcBoost_Ki;
    //Boost电流外环Kp
    debug_RegHoldingBuf_w[25]  = DcDc_gParam.IBoost_Kp;
    //Boost电流外环Ki
    debug_RegHoldingBuf_w[26]  = DcDc_gParam.IBoost_Ki;
    debug_RegHoldingBuf_w[27]  = 0;
    debug_RegHoldingBuf_w[28]  = 0;
    debug_RegHoldingBuf_w[29]  = 0;
    debug_RegHoldingBuf_w[30]  = 0;
    debug_RegHoldingBuf_w[31]  = 0;
    return 0;
}

//设置寄存器处理
/* CODEMAP_PARAM_WRITE
 * Central write dispatcher. This is where Modbus writes become StartEn, WorkMode, CtrlMode,
 * relay debug commands, Pwm_StartEn, fault reset or EEPROM-backed parameter updates.
 */
static int16_t Param_WriteDataHoldingReg(uint16_t reg,int16_t value)
{
//    int16_t i = 0;
//    int16_t k = 0;
    int16_t reg_buf[2];
    int16_t regData;
    uint16_t eeAddr;
    int16_t index=800;
    int16_t ret = 0;
    if (reg == 500 && value == 10624)
    {
        I2C_EE_WriteBuf(EE_ADDR_KEY, 8, u8_UID_REGS);
        DEVICE_DELAY_US(1000);
        I2C_EE_WriteBuf(EE_ADDR_KEY+8, 8, &u8_UID_REGS[8]);
        DEVICE_DELAY_US(1000);
        I2C_EE_WriteBuf(EE_ADDR_KEY+16, 8, &u8_UID_REGS[16]);
        DEVICE_DELAY_US(1000);
        I2C_EE_WriteBuf(EE_ADDR_KEY+24, 8, &u8_UID_REGS[24]);
        DEVICE_DELAY_US(1000);
    }
    else
    {
        switch(reg)
        {
        case 100://启动使能
            if((value == 0)||(value == 1))
            {
                DcDc_gParam.StartEn  = value;
            }
            break;
        case 101: index = 26; break;
        case 102: index = 27; break;
        case 103: index = 28; break;
        case 104: index = 29; break;
        case 105: index = 30; break;
        case 106: index = 31; break;
        case 107: index = 32; break;
        case 108: index = 37; break;
        case 109: index = 38; break;
        case 110: index = 39; break;
        case 111: index = 40; break;
        case 112:
            if(value == 1)
            {
                uint16_t reg_buf[8]={0};
                I2C_EE_WriteBuf(EE_ADDR_CHARGE, 8, reg_buf);
//                I2C_EE_ReadBuf(EE_ADDR_CHARGE, 8, reg_buf);
                DC_OutMeter.DC_LdisChargeKWH     = 0;
                DC_OutMeter.DC_HdisChargeKWH     = 0;
                DC_OutMeter.DC_LChargeKWH        = 0;
                DC_OutMeter.DC_HChargeKWH        = 0;
            }
            break;
        case 113: index = 49; break;
        case 207://故障清除
            if(value == 1)
            {
                App_AllFault_Reset();
            }
            break;
        case 208: index = 18; break;
        case 209: index = 19; break;
        case 210: index = 20; break;
        case 211: index = 21; break;
        case 212: index = 22; break;
        case 213: index = 23; break;
        case 214: index = 24; break;
        case 215: index = 25; break;
        case 216: index = 17; break;
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
//        case 311: index = 4; break;
//        case 312: index = 5; break;
        case 313: index = 4; break;
        case 314: index = 5; break;
        case 315: index = 6; break;
        case 316: index = 7; break;
        case 317: index = 8;break;
        case 318: index = 9;break;
        case 319: index = 10;break;
        case 320: index = 11;break;
        case 321: index = 12;break;
        case 322: index = 13;break;
        case 327: index = 14;break;
        case 323: index = 15;break;
        case 324: index = 16;break;
//        case 326: index = 49;break;
        case 325: index = 50;break;
        case 326: index = 51;break;
        case 114: index = 52;break;
        case 115: index = 53;break;
        case 331: index = 54;break;
        case 332: index = 55;break;
        case 400://母线软起继电器 低压侧
            if(((value == 0)||(value == 1))&&(DcDc_gParam.debugMode))
                DcDc_gParam.RelaySt_DcBusSoft   = value;
            break;
        case 401://母线主继电器 低压侧
            if(((value == 0)||(value == 1))&&(DcDc_gParam.debugMode))
                DcDc_gParam.RelaySt_DcBusMaster = value;
            break;
        case 402://电池软起继电器 高压侧
            if(((value == 0)||(value == 1))&&(DcDc_gParam.debugMode))
                DcDc_gParam.RelaySt_DcBatSoft   = value;
            break;
        case 403://电池主继电器 高压侧
            if(((value == 0)||(value == 1))&&(DcDc_gParam.debugMode))
                DcDc_gParam.RelaySt_DcBatMaster = value;
            break;
        case 404://
            if (value == 1)
            {
                DcDc_gParam.WorkMode = 1;
                DcDc_gParam.CtrlMode = 2;
                DcDc_gParam.LocalRemote = 0;
                DcDc_gParam.DcConstVolSet = 3000;
                DcDc_gParam.DcConstCurrSet = 200;
                DcDc_gParam.DcConstPowerSet = 20;
                DcDc_gParam.DcConstResSet = 410;
                DcDc_gParam.DcBatMaxCurrent = 4200;
                DcDc_gParam.DcBusMaxCurrent = 4200;
                DcDc_gParam.DcBatMaxVol = 15000;
                DcDc_gParam.DcBusMaxVol = 15000;
                g_calibratParam.VBus_Calibrat = 9996;
                g_calibratParam.VBusS_Calibrat = 10006;
                g_calibratParam.VBat_Calibrat = 9958;
                g_calibratParam.VBatS_Calibrat = 10001;
                g_calibratParam.VBatP_Calibrat = 9944;
                g_calibratParam.VBatN_Calibrat = 9966;
                g_calibratParam.IBus_Calibrat = 10040;
                g_calibratParam.IBat_Calibrat = 10070;
                g_calibratParam.IL1_Calibrat = 10370;
                g_calibratParam.IL2_Calibrat = 10370;
                g_calibratParam.IBus_offset = 9996;
                g_calibratParam.IBat_offset = 10017;
                g_calibratParam.IL1_offset = 10010;
                g_calibratParam.IL2_offset = 10033;
                g_calibratParam.Ref1v5_Calibrat = 10000;
                DcDc_gParam.VBuck_Kp = 275;
                DcDc_gParam.VBuck_Ki = 7500;
                DcDc_gParam.IdcBuck_Kp = 100;
                DcDc_gParam.IdcBuck_Ki = 350;
                DcDc_gParam.IBuck_Kp = 10;
                DcDc_gParam.IBuck_Ki = 4900;
                DcDc_gParam.VBoost_Kp = 260;
                DcDc_gParam.VBoost_Ki = 4600;
                DcDc_gParam.IdcBoost_Kp = 150;
                DcDc_gParam.IdcBoost_Ki = 1000;
                DcDc_gParam.IBoost_Kp = 10;
                DcDc_gParam.IBoost_Ki = 7500;
                DcDc_Protect_Threshold.Bat_OverVol_Value = 15500;
                DcDc_Protect_Threshold.Bat_UnderVol_Value = 50;
                DcDc_Protect_Threshold.Bus_OverVol_Value = 12500;
                DcDc_Protect_Threshold.Bus_UnderVol_Value = 200;
                DcDc_Protect_Threshold.Bat_OverCurr_Value = 4300;
                DcDc_Protect_Threshold.Bus_OverCurr_Value = 4300;
                DcDc_Protect_Threshold.Vol_Unbance_Value = 300;
                DcDc_Protect_Threshold.Curr_Unbance_Value = 300;
                DcDc_Protect_Threshold.Temp_Over_Value = 1000;

            }
            break;
        case 405://手动发波
           if((value == 0)||(value == 1))
           {
               DcDc_gParam.Pwm_StartEn = value;
           }
           break;
        case 406://风扇继电器
            if(((value == 0)||(value == 1))&&(DcDc_gParam.debugMode))
                DcDc_gParam.RelaySt_FanCtrl = value;
            break;
        case 408://工作指示灯
            if(((value == 0)||(value == 1))&&(DcDc_gParam.debugMode))
                DcDc_gParam.RelaySt_WorkLed = value;
            break;
        case 409://故障指示灯
            if(((value == 0)||(value == 1))&&(DcDc_gParam.debugMode))
            {
                DcDc_gParam.RelaySt_FaultLed = value;
            }
            break;
        case 410://软起动使能
            if((value == 0)||(value == 1))
            {
                DcDc_gParam.SoftStart_En = value;
            }
            break;
        case 411://调试模式
            if((value == 0)||(value == 1))
            {
                DcDc_gParam.debugMode = value;
            }
            break;
        case 412://外环使能模式
             if((value == 0)||(value == 1))
             {
                 DcDc_gParam.OutLoop_En = value;
             }
             break;
        case 415: index = 33; break;
        case 416: index = 34; break;
        case 417: index = 35; break;
        case 418: index = 36; break;
        case 419: index = 41; break;
        case 420: index = 42; break;
        case 421: index = 43; break;
        case 422: index = 44; break;
        case 423: index = 45; break;
        case 424: index = 46; break;
        case 425: index = 47; break;
        case 426: index = 48; break;
        default:ret = -1;break;
        }
    }

    if(ret == 0)
    {
        if(reg == store_param[index].ModAddr)
        {
          regData = value;
          eeAddr  = store_param[index].eeAddr;
          if((regData>=store_param[index].min)&&(regData<=store_param[index].max))
          {
              reg_buf[0] = regData&0x0ff;
              reg_buf[1] = (regData>>8)&0x0ff;
              *(store_param[index].g_param) = regData;
              I2C_EE_WriteBuf(eeAddr, 2, reg_buf);
          }
        }
            return 0;
    }
    else
        return -1;
}

//保持寄存器数据更新
/* CODEMAP_UPDATE_HOLDING: refresh all Modbus mirror arrays from current runtime variables. */
int16_t upDataHoldingCBReg(void)
{
    //状态寄存器数据更新
    status_upDataHoldingCBReg();
    //控制寄存器数据更新
    Ctrl_upDataHoldingCBReg();
    //故障状态寄存器更新
    Fault_upDataHoldingCBReg();
    //参数寄存器数据更新
    Param_upDataHoldingCBReg();
    //调试寄存器数据更新
    debug_upDataHoldingCBReg();

    return 0;
}


