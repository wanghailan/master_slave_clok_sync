/*
 * ModbusTcp_Net1.c
 *
 *  Created on: 2025年6月24日
 *      Author: guowei
 */

#include <stdio.h>
#include <string.h>
#include "lwip/opt.h"
#include "lwip/api.h"
#include "lwip/tcp.h"
#include "ipc.h"
#include "bsp.h"

#include "socket.h"

#define  MBTCP_ADDR        0x01

//mb1 init
#define TIMEOUT_INTERVAL 10 // 超时时间间隔（秒）

int16_t mb1_TimeOut = 0;

uint8_t MbTcp1_RxBuf[512]={0};
int16_t MbTcp1_RxCnt;

struct tcp_pcb *lwipMb_pcb;

//mb2 init
#define  mb2_w5500Port      0   //端口0
#define  mb2_TcpPort        502

int8_t   mb2_ConStatus;     //mb2 Tcp连接状态
int16_t  mb2_TimeOut;       //mb2 连接超时

uint8_t  MbTcp2_RxBuf[512]={0};
int16_t  MbTcp2_RxCnt=0;

int16_t ModbusTcp1_Poll(void);


/******************************************ENET1接口初始化*********************************************************/

int16_t   LwipTcp_send(uint8_t *_pBuf,int16_t _pLen)
{
    // 发送响应
    struct pbuf *resp_buf = pbuf_alloc(PBUF_TRANSPORT, _pLen, PBUF_RAM);
    memcpy(resp_buf->payload, _pBuf, _pLen);
    tcp_write(lwipMb_pcb, resp_buf->payload, resp_buf->len, TCP_WRITE_FLAG_COPY);
    tcp_output(lwipMb_pcb);
    pbuf_free(resp_buf);

    return 0;
}


// 处理接收到的数据
static err_t tcp_server_recv_cb(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (err != ERR_OK || p == NULL) {
        // 如果发生错误或数据为空，则关闭连接
        if (p != NULL) {
            tcp_recved(tpcb, p->tot_len);
            pbuf_free(p);
        }
        tcp_close(tpcb);
        return err;
    }
    mb1_TimeOut = 0;
    // 释放接收缓冲区
    tcp_recved(tpcb, p->tot_len);
    MbTcp1_RxCnt = p->len;
    memcpy( MbTcp1_RxBuf, p->payload, (p->len) );
    lwipMb_pcb = tpcb;
 //   LwipTcp_send(MbTcp1_RxBuf,MbTcp1_RxCnt);
    ModbusTcp1_Poll();

    pbuf_free(p);

    return ERR_OK;
}

// 连接关闭时的回调函数
static void tcp_server_closed(void *arg, struct tcp_pcb *tpcb, err_t err) {
    tcp_arg(tpcb, NULL);
    tcp_sent(tpcb, NULL);
    tcp_recv(tpcb, NULL);
    tcp_err(tpcb, NULL);
    memp_free(MEMP_TCP_PCB, tpcb);
}

// 错误回调函数
static void tcp_error_cb(void *arg, err_t err) {
    ;
}

static err_t tcp_server_poll_cb(void *arg, struct tcp_pcb *tpcb) {

    // 每次调用 poll_count 增加
    mb1_TimeOut++;
    // 如果超过超时时间间隔，则关闭连接
    if (mb1_TimeOut >= TIMEOUT_INTERVAL) {
        tcp_abort(tpcb);
        return ERR_ABRT;
    }

    return ERR_OK;
}

// 连接建立时的回调函数
static err_t tcp_server_accept_cb(void *arg, struct tcp_pcb *newpcb, err_t err) {
    if (err != ERR_OK || newpcb == NULL) {
        return ERR_VAL;
    }

    // 设置接收回调函数
    tcp_accepted(newpcb);
    tcp_recv(newpcb, tcp_server_recv_cb);
    //设置错误回调函数
    tcp_err(newpcb, tcp_error_cb);
    //设置轮询回调函数 轮询间隔1s
    mb1_TimeOut = 0;
    tcp_poll(newpcb, tcp_server_poll_cb, 1);
//    //开启keep alive 初始间隔1s，最多3次，每次10s
//    tcp_keepalive(newpcb, 1, 10, 3);

    return ERR_OK;
}

// 初始化 TCP 服务器
void Lwip_TcpServerInit(void)
{
    struct tcp_pcb *server_pcb;
    err_t err;

    // 创建一个新的 TCP 控制块
    server_pcb = tcp_new();
    if (server_pcb == NULL){
        return;//失败返回
    }

    // 绑定到指定端口
    err = tcp_bind(server_pcb, IP_ADDR_ANY, 502);
    if (err != ERR_OK) {
        tcp_close(server_pcb);
        return;
    }

    // 监听传入连接
    server_pcb = tcp_listen(server_pcb);
    if (server_pcb == NULL) {
        tcp_close(server_pcb);
        return;
    }
    // 设置接受回调函数
    tcp_accept(server_pcb, tcp_server_accept_cb);
}

/*
*********************************************************************************************************
*   函 数 名: MODS_SendAckErr
*   功能说明: 发送错误应答
*   形    参: _ucErrCode : 错误代码
*   返 回 值: 无
*********************************************************************************************************
*/
static void MbTcp_SendAckErr(int8_t _ch,uint8_t *Mb_RxBuf,uint16_t Mb_RxCnt,uint8_t _ucErrCode)
{
    uint8_t txbuf[9];

    txbuf[0] = Mb_RxBuf[0]; /* 返回事务标识 */
    txbuf[1] = Mb_RxBuf[1];
    txbuf[2] = Mb_RxBuf[2]; /* 返回协议标识 */
    txbuf[3] = Mb_RxBuf[3];
    txbuf[4] = 0x00;              /* 返回字节数 */
    txbuf[5] = 3;
    txbuf[6] = Mb_RxBuf[6];      //单元标识符  设备地址
    txbuf[7] = Mb_RxBuf[7] | 0x80;      //功能码 异常的功能码
    txbuf[8] = _ucErrCode;
    if(_ch==1)
        LwipTcp_send(txbuf,9);   /* 发送应答 */
    else
        send(mb2_w5500Port,txbuf,9);
}
/*
*********************************************************************************************************
*   函 数 名: MODS_03H
*   功能说明: 读取保持寄存器 在一个或多个保持寄存器中取得当前的二进制值
*   形    参: 无
*   返 回 值: 无
*********************************************************************************************************
*/
void MbTcp_03H(int8_t _ch,uint8_t *Mb_RxBuf,uint16_t Mb_RxCnt)
{
    int16_t  RspCode;
    uint16_t regAddr;//读寄存器开始地址
    uint16_t regNum;//读寄存器寄存器数
    uint32_t regAddrBegin;
    uint16_t *regAddr_begin;
    uint16_t i;
    uint8_t  Mb_TxBuf[Mod_TXBUF_SIZE];
    int16_t  Mb_TxCnt = 0;


    RspCode = RSP_OK;

    /** 第1步： 判断接到指定个数数据 ===============================================================*/
    /* (7-1) 字节 MBAP +地址（1字节）+指令（1字节）+寄存器起始地址高低字节（2字节）+寄存器个数（2字节）*/
    if (Mb_RxCnt != 12)                               /* 03H命令必须是8个字节 */
    {
        RspCode = RSP_ERR_VALUE;                    /* 数据值域错误 */
        goto err_ret;
    }

    /** 第2步： 数据解析 ===========================================================================*/
    /* 数据是大端，要转换为小端 */
    regAddr = BEBufToUint16(Mb_RxBuf[8],Mb_RxBuf[9]);                 /* 寄存器号 */
    regNum  = BEBufToUint16(Mb_RxBuf[10],Mb_RxBuf[11]);                 /* 寄存器个数 */

    RspCode = Mb03_RegAddrLookUp(regAddr,regNum,&regAddrBegin);
    regAddr_begin = (uint16_t *)regAddrBegin;
    /** 第3步： 应答回复 =========================================================================*/
err_ret:
    if (RspCode == RSP_OK)                           /* 正确应答 */
    {
        Mb_TxCnt = regNum*2+3;//要返回的寄存器个数加标识符和功能码
        Mb_TxBuf[0] = Mb_RxBuf[0]; /* 返回事务标识 */
        Mb_TxBuf[1] = Mb_RxBuf[1];
        Mb_TxBuf[2] = Mb_RxBuf[2]; /* 返回协议标识 */
        Mb_TxBuf[3] = Mb_RxBuf[3];
        Mb_TxBuf[4] = (Mb_TxCnt>>8)&0xff;  /* 返回字节数 */
        Mb_TxBuf[5] = (Mb_TxCnt)&0xff;
        Mb_TxBuf[6] = Mb_RxBuf[6];  //设备地址
        Mb_TxBuf[7] = Mb_RxBuf[7];  //功能码
        Mb_TxBuf[8] = Mb_TxCnt-3;  //03数据前要加数据字节数
        Mb_TxCnt = 9;
        for (i = 0; i < regNum; i++)  /* 返回数据*/
        {
            Mb_TxBuf[Mb_TxCnt++] = (uint8_t)((regAddr_begin[i] >> 8)&0xFF);
            Mb_TxBuf[Mb_TxCnt++] = (uint8_t)(regAddr_begin[i] & 0xFF );
        }
        if(_ch == 1)
            LwipTcp_send(Mb_TxBuf,Mb_TxCnt);   /* 发送应答 */
        else
            send(mb2_w5500Port,Mb_TxBuf,Mb_TxCnt);
    }
    else
    {
        MbTcp_SendAckErr(_ch,Mb_RxBuf,Mb_RxCnt,RspCode);
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
void MbTcp_06H(int8_t _ch,uint8_t *Mb_RxBuf,uint16_t Mb_RxCnt)
{
    /*写保持寄存器。注意06指令只能操作单个保持寄存器，16指令可以设置单个或多个保持寄存器*/
    int16_t  RspCode;
    uint16_t reg;
    int16_t value;

    /** 第1步： 判断接到指定个数数据 ===============================================================*/
    /* (7-1) 字节 MBAP +地址（1字节）+指令（1字节）+寄存器起始地址高低字节（2字节）+寄存器个数（2字节） */
    if (Mb_RxCnt != 12)
    {
        RspCode = RSP_ERR_VALUE;        /* 数据值域错误 */
        goto err_ret;
    }

    /** 第2步： 数据解析 ===========================================================================*/
    /* 数据是大端，要转换为小端 */
    reg   = BEBufToUint16(Mb_RxBuf[8],Mb_RxBuf[9]);   /* 寄存器号 */
    value = BEBufToUint16(Mb_RxBuf[10],Mb_RxBuf[11]);   /* 寄存器值 */

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
        if(_ch == 1)
            LwipTcp_send(Mb_RxBuf,Mb_RxCnt);   /* 发送应答 */
        else
            send(mb2_w5500Port,Mb_RxBuf,Mb_RxCnt);
    }
    else
    {/* 告诉主机命令错误 */
        MbTcp_SendAckErr(_ch,Mb_RxBuf,Mb_RxCnt,RspCode);
    }
}


/*
*********************************************************************************************************
*   函 数 名: MODS_AnalyzeApp
*   功能说明: 分析应用层协议
*   形    参: 无
*   返 回 值: 无
*********************************************************************************************************
*/
static void ModbusTcp_AnalyzeApp(int8_t _ch,uint8_t *Mb_RxBuf,uint16_t Mb_RxCnt)
{
    switch (Mb_RxBuf[7])                     /* 第2个字节 功能码 */
    {
//        case 0x01:                          /* 读取线圈状态*/
//            break;
//        case 0x02:                          /* 读取输入状态*/
//            break;
        case 0x03:                            /* 读取保持寄存器（此例程存在g_tVar中）*/
            MbTcp_03H(_ch,Mb_RxBuf,Mb_RxCnt);
            break;
//        case 0x04:                          /* 读取输入寄存器*/
//            break;
//        case 0x05:                          /* 强制单线圈*/
//            break;
        case 0x06:                            /* 写单个保存寄存器*/
            MbTcp_06H(_ch,Mb_RxBuf,Mb_RxCnt);
            break;
//        case 0x10:                          /* 写多个保存寄存器*/
//            break;
        default:
            MbTcp_SendAckErr(_ch,Mb_RxBuf,Mb_RxCnt,RSP_ERR_CMD);   /* 告诉主机命令错误 */
            break;
    }
}

int16_t ModbusTcp1_Poll(void)
{
    uint8_t addr;

    /* 接收到的数据小于10个字节就认为错误，7 字节 MBAP 报文头+指令（8bit）+操作寄存器（16bit） */
    if (MbTcp1_RxCnt < 10)
    {
        return -1;
    }
    //ModbusTcp 使用tcp自带校验，不需要自己校验
    addr = MbTcp1_RxBuf[6];        /* 第1字节 站号 */
    if (addr != MBTCP_ADDR)     /* 判断主机发送的命令地址是否符合 */
    {
        goto err_ret;
    }
    /* 分析应用层协议 */
    ModbusTcp_AnalyzeApp(1,MbTcp1_RxBuf,MbTcp1_RxCnt);

err_ret:
    MbTcp1_RxCnt = 0;           /* 必须清零计数器，方便下次帧同步 */

    return 0;
}

int16_t ModbusTcp2_Poll(void)
{
    uint8_t addr;

    /* 接收到的数据小于10个字节就认为错误，7 字节 MBAP 报文头+指令（8bit）+操作寄存器（16bit） */
    if (MbTcp2_RxCnt < 10)
    {
        return -1;
    }
    //ModbusTcp 使用tcp自带校验，不需要自己校验
    addr = MbTcp2_RxBuf[6];        /* 第1字节 站号 */
    if (addr != MBTCP_ADDR)     /* 判断主机发送的命令地址是否符合 */
    {
        goto err_ret;
    }
    /* 分析应用层协议 */
    ModbusTcp_AnalyzeApp(2,MbTcp2_RxBuf,MbTcp2_RxCnt);

err_ret:
    MbTcp2_RxCnt = 0;           /* 必须清零计数器，方便下次帧同步 */

    return 0;
}


void MbTcp1_Init(void)
{
    Lwip_TcpServerInit();
}

/******************************************ENET2接口初始化*********************************************************/
/*
 * LocalNum 代W5500的端口0~7
 * */
void w5500_TcpMainServer(void)
{
    uint16_t len=0;

    switch(getSn_SR(mb2_w5500Port))//获取W5500端口0的状态寄存器
    {
        case SOCK_CLOSED://W5500端口0处于关闭状态
            socket(mb2_w5500Port,Sn_MR_TCP,mb2_TcpPort,Sn_MR_ND);//打开Socket0，打开一个本地端口
            mb2_ConStatus = 0;
            mb2_TimeOut   = 1;
            break;
        case SOCK_INIT://W5500端口已经初始化
        case SOCK_LISTEN:
            listen(mb2_w5500Port);
          break;
        case SOCK_ESTABLISHED://W5500端口已经处于连接成功
            if(getSn_IR(mb2_w5500Port) & Sn_IR_CON)
            {//读端口0的Sn_IR中断标志寄存器的bit0
                setSn_IR(mb2_w5500Port, Sn_IR_CON);//回写清除中断标志
                mb2_ConStatus = 1;
            }
            len=getSn_RX_RSR(mb2_w5500Port);
            //读端口接收缓冲区的数据长度
            if(len > 0)
            {
                mb2_TimeOut   = 1;
                MbTcp2_RxCnt = recv(mb2_w5500Port,MbTcp2_RxBuf,len);//读"W5500端口0"的数据,长度为len个字节,保存到gDATABUF
//                send(mb2_w5500Port,rxBuf,len);//将gDATABUF的前len个字节通过"W5500端口0"发送出去
                ModbusTcp2_Poll();
            }
            else
                mb2_TimeOut++;

            if(mb2_TimeOut > 300) //3S没有收到数据，断开连接 10ms定时查询一次
            {
                mb2_ConStatus = 0;
                mb2_TimeOut   = 1;
                disconnect(mb2_w5500Port);
                close(mb2_w5500Port);
            }
            break;

        case SOCK_CLOSE_WAIT://W5500端口0处于等待关闭状态
            mb2_ConStatus = 0;
            mb2_TimeOut   = 1;
            close(mb2_w5500Port);
          break;
        default:break;
    }
}










