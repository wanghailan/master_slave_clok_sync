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
    uint32_t tsCtrl;
    uint32_t ptpClkHz;
    uint32_t ppsPeriodTicks;
    uint32_t ppsWidthTicks;
    uint32_t sec, nsec;

    // 1.关闭时间戳
    tsCtrl = HWREG(EMAC_BASE + ETHERNET_O_MAC_TIMESTAMP_CONTROL);
    HWREG(EMAC_BASE + ETHERNET_O_MAC_TIMESTAMP_CONTROL) = tsCtrl & ~ETHERNET_MAC_TIMESTAMP_CONTROL_TSENA;

    // 2.PTP时钟配置（配置PTP时间戳为Slave模式）
    varPtpConfig = (0U << ETHERNET_MAC_TIMESTAMP_CONTROL_SNAPTYPSEL_S) |
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
    // 3.初始化系统时间
    Ethernet_setSysTimePTP(EMAC_BASE, 0x00000000U, 0x00000000U);

    // We need to program this standard multicast address so that this device
    // identifies PTP over Ethernet packets correctly. "01:1B:19:00:00:00"
    // 4.配置PTP多播MAC地址
    Ethernet_setMACAddr(EMAC_BASE,
                        1U,
                        0x00000000U,
                        0x00191B01U, // 01:1B:19:00:00:00
                        ETHERNET_CHANNEL_0);

    // 5.初始化PTP组播地址到报文缓冲区
    i=0U; *((uint32_t *)gMsgBuf + i) = 0x00191B01;
    i++; *((uint32_t *)gMsgBuf + i)  = 0xF7880000;

    // 6.初始化Slave状态
    memset(&gPtpSlaveState, 0, sizeof(PTPSlaveState));
    InitSlaveConstants(&gPtpSlaveState);

    gPtpSlaveState.portNumber          = 1;
    gPtpSlaveState.delayReqSeqId       = 0;
    gPtpSlaveState.syncReceived        = FALSE;
    gPtpSlaveState.followUpReceived    = FALSE;
    gPtpSlaveState.delayRespReceived   = FALSE;

    // 7.配置PPS输出为脉冲模式
    Ethernet_selectTargetInterruptOrPulsePPS(
            EMAC_BASE,
            ETHERNET_MAC_PPS_OUT_INSTANCE_0,
            ETHERNET_MAC_PPS_CONTROL_TRGTMODSEL_PULSE);

    // 8.PPS周期与脉宽
    ptpClkHz = (uint32_t)(1.0f / PTP_REF_CLOCK_PERIOD);
    ppsPeriodTicks = ptpClkHz * 1U;
    ppsWidthTicks  = ptpClkHz / 100U;

    HWREG(EMAC_BASE + ETHERNET_MAC_PPS_INTERVAL) = ppsPeriodTicks;
    HWREG(EMAC_BASE + ETHERNET_MAC_PPS_WIDTH)    = ppsWidthTicks;

    // 9.开启时间戳模块
    HWREG(EMAC_BASE + ETHERNET_O_MAC_TIMESTAMP_CONTROL) = tsCtrl | ETHERNET_MAC_TIMESTAMP_CONTROL_TSENA;

    // 10.设置初始PPS目标时间为当前秒+1
    Ethernet_getSysTimePTP(EMAC_BASE, &sec, &nsec);
    Ethernet_setTargetTimePPS(EMAC_BASE,
                              ETHERNET_MAC_PPS_OUT_INSTANCE_0,
                              sec + 1U,
                              0U);

    // 11.打开PP0输出
    HWREG(EMAC_BASE + ETHERNET_O_MAC_PPS_CONTROL) |= ETHERNET_MAC_PPS_CONTROL_PPSEN0;

    // 状态清零
    gPtpSlaveState.lastSyncSeqId = 0;
    gPtpSlaveState.followUpSeqId = 0;
    gPtpSlaveState.delayReqSeqId = 0;
    gPtpSlaveState.syncReceived = false;
    gPtpSlaveState.followUpReceived = false;
    gPtpSlaveState.delayRespReceived = false;
}

// Slave处理报文交互+时钟同步
void ptp_slave_run(void)
{
    const uint32_t TIMEOUT_MAX = 2000000U;
    uint32_t timeout = 0U;

    // 1.已收到Sync, 需要等待Follow_Up
    if (gPtpSlaveState.syncReceived && !gPtpSlaveState.followUpReceived)
    {
        // 等待Follow_Up报文（携带t1）
        timeout = 0U;
        while (!gPtpSlaveState.followUpReceived && (timeout < TIMEOUT_MAX))
        {
            timeout++;
        }
        if (timeout >= TIMEOUT_MAX)
        {
            CmIpc_cm2cpu.IpcCpu2Cm_Fault = 1U;
            gPtpSlaveState.syncReceived = FALSE;
            return;
        }
    }

    // 2.已收到 Follow_Up, 开始发送Delay_Req报文（并记录发送时间t3）
    if (gPtpSlaveState.followUpReceived && !gPtpSlaveState.delayRespReceived)
    {
        gPtpSlaveState.delayReqSeqId++;

        // 记录Delay_Req发送时间（t3）
        Ethernet_getSysTimePTP(EMAC_BASE,
                               &gPtpSlaveState.delayReqSentTimestamp.secondsField.lsb,
                               &gPtpSlaveState.delayReqSentTimestamp.nanosecondsField);

        // 发送Delay_Req报文
        sendMessage((Octet *)gMsgBuf, DELAY_REQ, &gPtpSlaveState, &gPktDesc);
        gPtpSlaveState.delayRespReceived = TRUE;
    }

    // 3.等待Delay_Resp报文（携带t4）
    if (gPtpSlaveState.delayRespReceived)
    {
        timeout = 0U;
        while (!gPtpSlaveState.delayRespReceived && (timeout < TIMEOUT_MAX))
        {
            timeout++;
        }
        if (timeout >= TIMEOUT_MAX)
        {
            CmIpc_cm2cpu.IpcCpu2Cm_Fault = 1U;
            gPtpSlaveState.delayRespReceived = FALSE;
            return;
        }
    }

    // 4.如果收到Delay_Resp，调整时钟
    if (gPtpSlaveState.delayRespReceived)
    {
        ptp_slave_adjust_clock();
        gPtpSlaveState.syncReceived      = FALSE;
        gPtpSlaveState.followUpReceived  = FALSE;
        gPtpSlaveState.delayRespReceived = FALSE;
    }

}

// 时钟调整函数,根据offset调整Slave本地PTP时钟
void ptp_slave_adjust_clock(void)
{
    int64_t t1_ns, t2_ns, t3_ns, t4_ns;
    int64_t offset_ns, delay_ns;
    uint32_t curr_sec, curr_nsec;

    // 1. 转换所有时间戳为纳秒（统一单位）
    // t1: Master发送Sync的时间 (from Follow_Up)
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

    // 2.计算偏移量和路径延迟
    offset_ns = ((t2_ns - t1_ns) + (t3_ns - t4_ns)) / 2;
    delay_ns  = ((t2_ns - t1_ns) - (t3_ns - t4_ns)) / 2;

    // 3.读取当前Slave的PTP系统时间
    Ethernet_getSysTimePTP(EMAC_BASE, &curr_sec, &curr_nsec);
    int64_t curr_ns = (int64_t)curr_sec * 1000000000LL + curr_nsec;

    // 4.纠正offse, 调整时间（减去offset，使Slave时钟对齐Master）
    int64_t new_ns = curr_ns - offset_ns;
    uint32_t new_sec = (uint32_t)(new_ns / 1000000000LL);
    uint32_t new_nsec = (uint32_t)(new_ns % 1000000000LL);

    // 5.写入调整后的时间到PTP系统时钟
    Ethernet_setSysTimePTP(EMAC_BASE, new_sec, new_nsec);

    // 5.1 首次同步, 通过 IPC告知CPU1
    static bool firstSyncDone = false;
    if (!firstSyncDone)
    {
        CmIpc_cm2cpu.PtpSynced = 1U;
        firstSyncDone = true;
    }

    // 6.同步后，更新下一次PPS的targetTime (保持1PPS对齐)
    uint32_t sec, nsec;
    Ethernet_getSysTimePTP(EMAC_BASE, &sec, &nsec);
    // 如果当前纳秒数大于500ms
    if (nsec > 500000000U)
    {
        sec++;
    }

    Ethernet_setTargetTimePPS(EMAC_BASE,
                              ETHERNET_MAC_PPS_OUT_INSTANCE_0,
                              sec + 1,
                              0);

}


