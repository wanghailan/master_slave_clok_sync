/*
 * ModbusTcp_Net1.c
 *
 *  Created on: 2025年6月24日
 *      Author: guowei
 */

#include <stdio.h>
#include <string.h>
#include "ipc.h"
#include "bsp.h"
#include "utils/lwiplib.h"
#include "lwipopts.h"
#include "driverlib_cm.h"
#include "cm.h"

struct udp_pcb *g_upcb;


void udp_rx_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    u8_t buf_rx[256]={0};
    //int16_t rxLen;
     if(p->len < 256)
     {
         memcpy(buf_rx,p->payload,p->len);
//         rxLen = p->len;
         if ((buf_rx[0] == 'S') && (buf_rx[1] == 'T') && (buf_rx[2] == 'O') && (buf_rx[3] == 'P'))
         {
             CmIpc_cm2cpu.debugData_TxEn = 0;
             udp_disconnect(upcb);
         }
         else if ((buf_rx[0] == 'S') && (buf_rx[1] == 'T') && (buf_rx[2] == 'A') && (buf_rx[3] == 'R') && (buf_rx[4] == 'T'))
         {
             CmIpc_cm2cpu.debugData_TxEn = 1;
             udp_connect(upcb, addr, port); /* connect to the remote host */
         }
//         udp_sendto(g_upcb, p, addr, port);
     }
     pbuf_free(p);

//     if((CmIpc_cm2cpu.debugData_TxEn == 1)&&(rxLen > 0))
//     {
//         udpDebug_TxData(buf_rx,rxLen);
//         rxLen = 0;
//     }
}

/* UDP initialization ......................................................*/
void udpDebug_Init(void)
{
    g_upcb = udp_new();//创建udp控制块
    udp_bind(g_upcb, IP_ADDR_ANY, 6788);//绑定端口
    udp_recv(g_upcb, udp_rx_callback, NULL);//设置接收回调
}

void udpDebug_TxData(uint8_t *buf,int16_t len)
{
    struct pbuf *q = NULL;
//    err_t err_tx;
    if (CmIpc_cm2cpu.debugData_TxEn == 1)
    {
        q = pbuf_alloc(PBUF_TRANSPORT, len+2, PBUF_RAM);
        if(!q) //alloc error
        {
            return;
        }
        memset(q->payload, 0 , q->len);
        q->tot_len = len;
        q->len = len;
        memcpy(q->payload, buf, len);
        udp_sendto(g_upcb, q, (ip_addr_t *)&g_upcb->remote_ip, g_upcb->remote_port);
//        if(err_tx < 0)
//        {
//            CmIpc_cm2cpu.debugData_TxEn = 0;
//            udp_disconnect(g_upcb);
//        }
//        SysCtl_delay(100);
        pbuf_free(q);
    }
}










