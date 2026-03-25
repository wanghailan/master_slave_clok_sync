//#############################################################################
//
// FILE:   Drv_GPIO.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

#include  "bsp.h"

uint16_t   Mod1_RxBuf[Mod_TXBUF_SIZE];
int16_t    Mod1_RxCnt;

uint16_t   Mod2_RxBuf[Mod_TXBUF_SIZE];
int16_t    Mod2_RxCnt;

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

static void ModbusRtu_AnalyzeApp(int16_t _ch,uint16_t *Mb_RxBuf,uint16_t Mb_RxCnt);
static void MODS_03H(int16_t _ch,uint16_t *Mb_RxBuf,uint16_t Mb_RxCnt);
static void MODS_06H(int16_t _ch,uint16_t *Mb_RxBuf,uint16_t Mb_RxCnt);
static uint16_t CRC16_Modbus(uint16_t *_pBuf, uint16_t _usLen);
static void MODS_SendWithCRC(int16_t _ch,uint16_t *_pBuf, uint16_t _ucLen);
static void MODS_SendAckErr(int16_t _ch,uint8_t _ucErrCode);
static void MODS_SendAckOk(int16_t _ch);


int16_t ModbusRtu1_Poll(void)
{
    uint16_t addr;
    uint16_t crc1;

    /* 接收到的数据小于4个字节就认为错误，地址（8bit）+指令（8bit）+操作寄存器（16bit） */
    if (Mod1_RxCnt < 4)
    {
        return -1;
    }

    /* 计算CRC校验和，这里是将接收到的数据包含CRC16值一起做CRC16，结果是0，表示正确接收 */
    crc1 = CRC16_Modbus(Mod1_RxBuf, Mod1_RxCnt);
    if (crc1 != 0)
    {
        goto err_ret;
    }

    /* 站地址 (1字节） */
    addr = Mod1_RxBuf[0];     /* 第1字节 站号 */
    if (addr != RTU_ADDR)     /* 判断主机发送的命令地址是否符合 */
    {
        goto err_ret;
    }
    /* 分析应用层协议 */
    ModbusRtu_AnalyzeApp(1,Mod1_RxBuf,Mod1_RxCnt);

err_ret:
    Mod1_RxCnt = 0;           /* 必须清零计数器，方便下次帧同步 */

    return 0;
}

int16_t ModbusRtu2_Poll(void)
{
    uint16_t addr;
    uint16_t crc1;

    /* 接收到的数据小于4个字节就认为错误，地址（8bit）+指令（8bit）+操作寄存器（16bit） */
    if (Mod2_RxCnt < 4)
    {
        return -1;
    }

    /* 计算CRC校验和，这里是将接收到的数据包含CRC16值一起做CRC16，结果是0，表示正确接收 */
    crc1 = CRC16_Modbus(Mod2_RxBuf, Mod2_RxCnt);
    if (crc1 != 0)
    {
        goto err_ret;
    }

    /* 站地址 (1字节） */
    addr = Mod2_RxBuf[0];     /* 第1字节 站号 */
    if (addr != RTU_ADDR)     /* 判断主机发送的命令地址是否符合 */
    {
        goto err_ret;
    }
    /* 分析应用层协议 */
    ModbusRtu_AnalyzeApp(2,Mod2_RxBuf,Mod2_RxCnt);

err_ret:
    Mod2_RxCnt = 0;           /* 必须清零计数器，方便下次帧同步 */

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
static void ModbusRtu_AnalyzeApp(int16_t _ch,uint16_t *Mb_RxBuf,uint16_t Mb_RxCnt)
{
    switch (Mb_RxBuf[1])                  /* 第2个字节 功能码 */
    {
//        case 0x01:                          /* 读取线圈状态*/
//            break;
//        case 0x02:                          /* 读取输入状态*/
//            break;
        case 0x03:                          /* 读取保持寄存器（此例程存在g_tVar中）*/
            MODS_03H(_ch,Mb_RxBuf,Mb_RxCnt);
            break;
//        case 0x04:                          /* 读取输入寄存器*/
//            break;
//        case 0x05:                          /* 强制单线圈*/
//            break;
        case 0x06:                          /* 写单个保存寄存器*/
            MODS_06H(_ch,Mb_RxBuf,Mb_RxCnt);
            break;
//        case 0x10:                          /* 写多个保存寄存器*/
//            break;
        default:
            MODS_SendAckErr(_ch,RSP_ERR_CMD);   /* 告诉主机命令错误 */
            break;
    }
}

/*
*********************************************************************************************************
*   函 数 名: MODS_03H
*   功能说明: 读取保持寄存器 在一个或多个保持寄存器中取得当前的二进制值
*   形    参: 无
*   返 回 值: 无
*********************************************************************************************************
*/
static void MODS_03H(int16_t _ch,uint16_t *Mb_RxBuf,uint16_t Mb_RxCnt)
{
    int16_t  RspCode;
    uint16_t regAddr;//读寄存器开始地址
    uint16_t regNum;//读寄存器寄存器数
    uint32_t regAddrBegin;
    uint16_t *regAddr_begin;
    uint16_t i;
    uint16_t Mb_TxBuf[Mod_TXBUF_SIZE];
    int16_t  Mb_TxCnt = 0;


    RspCode = RSP_OK;

    /** 第1步： 判断接到指定个数数据 ===============================================================*/
    /* 地址（8bit）+指令（8bit）+寄存器起始地址高低字节（16bit）+寄存器个数（16bit）+ CRC16 */
    if (Mb_RxCnt != 8)                               /* 03H命令必须是8个字节 */
    {
        RspCode = RSP_ERR_VALUE;                    /* 数据值域错误 */
        goto err_ret;
    }

    /** 第2步： 数据解析 ===========================================================================*/
    /* 数据是大端，要转换为小端 */
    regAddr = BEBufToUint16(Mb_RxBuf[2],Mb_RxBuf[3]);                 /* 寄存器号 */
    regNum  = BEBufToUint16(Mb_RxBuf[4],Mb_RxBuf[5]);                 /* 寄存器个数 */

    RspCode = Mb03_RegAddrLookUp(regAddr,regNum,&regAddrBegin);
    regAddr_begin = (uint16_t *)regAddrBegin;
    /** 第3步： 应答回复 =========================================================================*/
err_ret:
    if (RspCode == RSP_OK)                           /* 正确应答 */
    {
        Mb_TxCnt = 0;
        Mb_TxBuf[Mb_TxCnt++] = Mb_RxBuf[0]; /* 返回从机地址 */
        Mb_TxBuf[Mb_TxCnt++] = Mb_RxBuf[1]; /* 返回从机指令 */
        Mb_TxBuf[Mb_TxCnt++] = regNum * 2;    /* 返回字节数 */

        for (i = 0; i < regNum; i++)  /* 返回数据*/
        {
            Mb_TxBuf[Mb_TxCnt++] = (uint16_t)(regAddr_begin[i] >> 8 );
            Mb_TxBuf[Mb_TxCnt++] = (uint16_t)(regAddr_begin[i] & 0xFF );
        }

        MODS_SendWithCRC(_ch,Mb_TxBuf, Mb_TxCnt);   /* 发送正确应答 */
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
static void MODS_06H(int16_t _ch,uint16_t *Mb_RxBuf,uint16_t Mb_RxCnt)
{
    /*写保持寄存器。注意06指令只能操作单个保持寄存器，16指令可以设置单个或多个保持寄存器*/
    int16_t  RspCode;
    uint16_t reg;
    int16_t value;

    /** 第1步： 判断接到指定个数数据 ===============================================================*/
    /* 地址（8bit）+指令（8bit）+寄存器起始地址高低字节（16bit）+寄存器个数（16bit）+ CRC16 */
    if (Mb_RxCnt != 8)
    {
        RspCode = RSP_ERR_VALUE;        /* 数据值域错误 */
        goto err_ret;
    }

    /** 第2步： 数据解析 ===========================================================================*/
    /* 数据是大端，要转换为小端 */
    reg   = BEBufToUint16(Mb_RxBuf[2],Mb_RxBuf[3]);   /* 寄存器号 */
    value = BEBufToUint16(Mb_RxBuf[4],Mb_RxBuf[5]);   /* 寄存器值 */

    if(CmIpc_cm2cpu.LocalRemote == 0) //本地控制
    {
        int16_t ret;
        ret = Param_WriteDataHoldingReg(reg,value);
        if(ret == -2)    /* 该函数会把写入的值存入寄存器 */
        {
            RspCode = RSP_ERR_VALUE;
        }
        else if(ret == -1)
        {
            RspCode = RSP_ERR_REG_ADDR;     /* 寄存器地址错误 */
        }
        else
        {
            RspCode = RSP_OK;
        }
    }
    else
    {
        if(reg == 103)//
            Param_WriteDataHoldingReg(reg,value);
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
    _pBuf[_ucLen++] = (crc >> 8)&0x00ff;
    _pBuf[_ucLen++] = crc&0x00ff;

    if(_ch == 1)
        ipc_TxData_CmToCpu2(ipcCmd_SciC,_pBuf,_ucLen);
    else
        ipc_TxData_CmToCpu2(ipcCmd_SciD,_pBuf,_ucLen);
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




