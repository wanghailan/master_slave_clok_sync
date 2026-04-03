/*
 * ptp_slave_sync.c
 *
 *  Created on: 2026年3月31日
 *      Author: whl
 */

#include "ptp_slave_sync.h"
#include "Eth_mii.h"
#include "bsp.h"

//PTPSlaveState gPtpSlaveState;

// 初始化常量
static void InitSlaveConstants(PTPSlaveState *ptpSlaveState)
{
    uint32_t mac_low, mac_high, i, j;
    uint8_t *pucTemp;

    Ethernet_getMACAddr(EMAC_BASE, 0, &mac_high, &mac_low);

    pucTemp = (uint8_t *)&mac_low;
    ptpSlaveState->port_uuid_field[0] = pucTemp[0];
    ptpSlaveState->port_uuid_field[1] = pucTemp[1];
    ptpSlaveState->port_uuid_field[2] = pucTemp[2];
    ptpSlaveState->port_uuid_field[3] = pucTemp[3];

    pucTemp = (uint8_t *)&mac_high;
    ptpSlaveState->port_uuid_field[4] = pucTemp[0];
    ptpSlaveState->port_uuid_field[5] = pucTemp[1];

    // 初始化ClockIdentity
    for (i = 0, j = 0; i < CLOCK_IDENTITY_LENGTH; i++)
    {
        if (i == 3) ptpSlaveState->portIdentity.clockIdentity[i] = 0xFF;
        else if (i == 4) ptpSlaveState->portIdentity.clockIdentity[i] = 0xFE;
        else
        {
            ptpSlaveState->portIdentity.clockIdentity[i] = ptpSlaveState->port_uuid_field[j];
            j++;
        }
    }
}

// Slave端PTP初始化
void ptp_slave_init(void)
{
    uint32_t i;
    uint32_t varPtpConfig = 0;
    float subSecondInc;

    // ========== 新增：先禁用时间戳，避免配置PPS时状态冲突 ==========
    uint32_t tsCtrl = HWREG(EMAC_BASE + ETHERNET_O_MAC_TIMESTAMP_CONTROL);
    HWREG(EMAC_BASE + ETHERNET_O_MAC_TIMESTAMP_CONTROL) = tsCtrl & ~ETHERNET_MAC_TIMESTAMP_CONTROL_TSENA;

    // PTP时钟配置（启用从机模式时间戳）
    varPtpConfig = 0x0 |
                    (0 << ETHERNET_MAC_TIMESTAMP_CONTROL_SNAPTYPSEL_S) |
                    ETHERNET_MAC_TIMESTAMP_CONTROL_TSCTRLSSR |
                    ETHERNET_MAC_TIMESTAMP_CONTROL_TSEVNTENA |
                    ETHERNET_MAC_TIMESTAMP_CONTROL_TSVER2ENA |
                    ETHERNET_MAC_TIMESTAMP_CONTROL_TSIPENA;

    // Subsecond increment is added to the systime counter every ptp clock tick
    // hence for Digital rollover, it is simply the time period of the clock tick.
    subSecondInc = PTP_REF_CLOCK_PERIOD;

    Ethernet_setConfigTimestampPTP(EMAC_BASE, varPtpConfig, subSecondInc);
    Ethernet_enableSysTimePTP(EMAC_BASE);

    // Start the system with a random value.
    Ethernet_setSysTimePTP(EMAC_BASE, 0x4132EDCA, 0x25a5a5a5);

    // We need to program this standard multicast address so that this device
    // identifies PTP over Ethernet packets correctly. "01:1B:19:00:00:00"
    Ethernet_setMACAddr(EMAC_BASE,
                        1,
                        0x00000000,
                        0x00191B01, // 01:1B:19:00:00:00
                        ETHERNET_CHANNEL_0);

    // 初始化PTP组播地址到报文缓冲区
    i=0; *((uint32_t *)gMsgBuf + i) = 0x00191B01;
    i++; *((uint32_t *)gMsgBuf + i)  = 0xF7880000;

    // 初始化Slave状态
    memset(&gPtpSlaveState, 0, sizeof(PTPSlaveState));

    InitSlaveConstants(&gPtpSlaveState);

    gPtpSlaveState.portNumber = 1;
    gPtpSlaveState.delayReqSeqId = 0;

    // 配置PPS输出为脉冲模式
    Ethernet_selectTargetInterruptOrPulsePPS(
            EMAC_BASE,
            ETHERNET_MAC_PPS_OUT_INSTANCE_0,
            ETHERNET_MAC_PPS_CONTROL_TRGTMODSEL_PULSE);

    // ========== 新增：重新使能时间戳 ==========
    HWREG(EMAC_BASE + ETHERNET_O_MAC_TIMESTAMP_CONTROL) = tsCtrl | ETHERNET_MAC_TIMESTAMP_CONTROL_TSENA;

    // 设置初始目标时间，当前秒+1
    uint32_t sec, nsec;
    Ethernet_getSysTimePTP(EMAC_BASE, &sec, &nsec);
    Ethernet_setTargetTimePPS(EMAC_BASE,
                              ETHERNET_MAC_PPS_OUT_INSTANCE_0,
                              sec + 1,
                              0);

}

// Slave处理报文交互+时钟同步
void ptp_slave_run(void)
{
    const uint32_t TIMEOUT_MAX = 2000000;
    uint32_t timeout = 0;

    // 1.等待接收Sync报文
    if (gPtpSlaveState.syncReceived && !gPtpSlaveState.waitingForFollowup)
    {
        // 等待Follow_Up报文（携带t1）
        timeout = 0;
        while (!gPtpSlaveState.waitingForFollowup && (timeout < TIMEOUT_MAX))
        {
            timeout++;
        }
        if (timeout >= TIMEOUT_MAX)
        {
            CmIpc_cm2cpu.IpcCpu2Cm_Fault = 1;
            gPtpSlaveState.syncReceived = FALSE;
            return;
        }
    }

    // 2.发送Delay_Req报文（并记录发送时间t3）
    if (gPtpSlaveState.waitingForFollowup && !gPtpSlaveState.waitingForDelayResp)
    {
        gPtpSlaveState.delayReqSeqId++;

        // 记录Delay_Req发送时间（t3）
        Ethernet_getSysTimePTP(EMAC_BASE,
                               &gPtpSlaveState.delayReqSentTimestamp.secondsField.lsb,
                               &gPtpSlaveState.delayReqSentTimestamp.nanosecondsField);

        // 发送Delay_Req报文
        sendMessage((Octet *)gMsgBuf, DELAY_REQ, &gPtpSlaveState, &gPktDesc);
        gPtpSlaveState.waitingForDelayResp = TRUE;
    }

    // 3.等待Delay_Resp报文（携带t4）
    if (gPtpSlaveState.waitingForDelayResp)
    {
        timeout = 0;
        while (!gPtpSlaveState.waitingForDelayResp && (timeout < TIMEOUT_MAX))
        {
            timeout++;
        }
        if (timeout >= TIMEOUT_MAX)
        {
            CmIpc_cm2cpu.IpcCpu2Cm_Fault = 1;
            gPtpSlaveState.waitingForDelayResp = FALSE;
            return;
        }
    }

    // 4.如果收到 Delay_Resp，调整时钟
    if (gPtpSlaveState.waitingForDelayResp)
    {
        ptp_slave_adjust_clock();
    }
}

// 时钟调整函数,根据offset调整Slave本地PTP时钟
void ptp_slave_adjust_clock(void)
{
    int64_t t1_ns, t2_ns, t3_ns, t4_ns;
    int64_t offset_ns, delay_ns;
    uint32_t curr_sec, curr_nsec;

    // 1. 转换所有时间戳为纳秒（统一单位）
    // t1: Master发送Sync的时间
    t1_ns = (int64_t)gPtpSlaveState.syncOriginTimestamp.secondsField.lsb * 1000000000LL +
            gPtpSlaveState.syncOriginTimestamp.nanosecondsField;

    // t2: Slave接收Sync的时间
    t2_ns = (int64_t)gPtpSlaveState.syncRecvTimestamp.secondsField.lsb * 1000000000LL +
            gPtpSlaveState.syncRecvTimestamp.nanosecondsField;

    // t3: Slave发送Delay_Req的时间
    t3_ns = (int64_t)gPtpSlaveState.delayReqSentTimestamp.secondsField.lsb * 1000000000LL +
            gPtpSlaveState.delayReqSentTimestamp.nanosecondsField;

    // t4: Master接收Delay_Req的时间
    t4_ns = (int64_t)gPtpSlaveState.delayReqRecvTimestamp.secondsField.lsb * 1000000000LL +
            gPtpSlaveState.delayReqRecvTimestamp.nanosecondsField;

    // 2. 计算偏移量和路径延迟
    offset_ns = ((t2_ns - t1_ns) + (t3_ns - t4_ns)) / 2;
    delay_ns  = ((t2_ns - t1_ns) - (t3_ns - t4_ns)) / 2;

    // 3. 读取当前Slave的PTP系统时间
    Ethernet_getSysTimePTP(EMAC_BASE, &curr_sec, &curr_nsec);
    int64_t curr_ns = (int64_t)curr_sec * 1000000000LL + curr_nsec;

    // 4. 调整时间（减去offset，使Slave时钟对齐Master）
    int64_t new_ns = curr_ns - offset_ns;
    uint32_t new_sec = (uint32_t)(new_ns / 1000000000LL);
    uint32_t new_nsec = (uint32_t)(new_ns % 1000000000LL);

    // 5. 写入调整后的时间到PTP系统时钟
    Ethernet_setSysTimePTP(EMAC_BASE, new_sec, new_nsec);

    // 6.重新设置PPS目标时间为下一个整秒(新增)
    Ethernet_getSysTimePTP(EMAC_BASE, &curr_sec, &curr_nsec);
    Ethernet_setTargetTimePPS(EMAC_BASE,
                              ETHERNET_MAC_PPS_OUT_INSTANCE_0,
                              curr_sec + 1,
                              0);

    // 重置状态，准备下一次同步
    gPtpSlaveState.syncReceived = FALSE;
    gPtpSlaveState.waitingForDelayResp = FALSE;
    gPtpSlaveState.waitingForDelayResp = FALSE;
}


