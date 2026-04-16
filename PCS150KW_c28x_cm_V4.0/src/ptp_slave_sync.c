/*
 * ptp_slave_sync.c
 *
 * TMS320F28388D PTP Slave 实现
 *
 *  功能：
 *    1. PTP从时钟初始化
 *    2. 接收Sync消息（记录t2）
 *    3. 接收FollowUp消息（获取t1）
 *    4. 发送DelayReq消息（记录t3）
 *    5. 接收DelayResp消息（获取t4）
 *    6. 计算offset并调整MAC系统时间
 *    7. 输出同步PPS信号（通过MAC PPS功能到GPIO47）
 *
 *  Created on: 2026年3月31日
 *      Author: whl
 */

#include "ptp_slave_sync.h"
#include "Eth_mii.h"
#include "bsp.h"

// 纳秒每秒
#define NS_PER_SEC                  1000000000LL
#define OFFSET_THRESHOLD_NS         100000      // 100us，大于此值直接调整相位
#define LOCK_THRESHOLD_NS           1000        // 1us，小于此值认为已锁定

//PTPSlaveState gPtpSlaveState;

static uint8_t  delayReqMsg[PACKET_LENGTH] = {0};


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

    ptpSlaveState->portIdentity.portNumber = 1;
    ptpSlaveState->portNumber = 1;
    ptpSlaveState->delayReqSeqId = 0;
    ptpSlaveState->syncReceived = FALSE;
    ptpSlaveState->followUpReceived = FALSE;
    ptpSlaveState->delayRespReceived = FALSE;
    ptpSlaveState->clockUpdateCount = 0;
    ptpSlaveState->lockCount = 0;
    ptpSlaveState->isLocked = FALSE;
}

/*============================ 时间操作函数 ============================*/
// 参考TI官方PTP定义
static void toInternalTime(TimeInternal *internal, Timestamp *external)
{
    internal->seconds = external->secondsField.lsb;
    internal->nanoseconds = external->nanosecondsField;
}

static void normalizeTime(TimeInternal *r)
{
    r->seconds += r->nanoseconds / NS_PER_SEC;
    r->nanoseconds -= (r->nanoseconds / NS_PER_SEC) * NS_PER_SEC;

    if (r->seconds > 0 && r->nanoseconds < 0) {
        r->seconds -= 1;
        r->nanoseconds += NS_PER_SEC;
    } else if (r->seconds < 0 && r->nanoseconds > 0) {
        r->seconds += 1;
        r->nanoseconds -= NS_PER_SEC;
    }
}

static void subTime(TimeInternal *r, const TimeInternal *x, const TimeInternal *y)
{
    r->seconds = x->seconds - y->seconds;
    r->nanoseconds = x->nanoseconds - y->nanoseconds;
    normalizeTime(r);
}

static void addTime(TimeInternal *r, const TimeInternal *x, const TimeInternal *y)
{
    r->seconds = x->seconds + y->seconds;
    r->nanoseconds = x->nanoseconds + y->nanoseconds;
    normalizeTime(r);
}

static void div2Time(TimeInternal *r)
{
    r->nanoseconds += (r->seconds % 2) * NS_PER_SEC;
    r->seconds /= 2;
    r->nanoseconds /= 2;
    normalizeTime(r);
}

static void getTime(TimeInternal *time)
{
    Ethernet_getSysTimePTP(EMAC_BASE, (uint32_t *)&time->seconds, (uint32_t *)&time->nanoseconds);
}

/*============================ PTP消息打包函数 ============================*/
static void msgPackHeader(Octet *buf, void *ptpState)
{
    *(UInteger8 *)(buf + 0) = 0x80;
    *(UInteger4 *)(buf + 1) = 0x02;
    *(UInteger8 *)(buf + 4) = 0;
    *(UInteger8 *)(buf + 6) = 0;
    *(UInteger8 *)(buf + 7) = 0;
    memset((buf + 8), 0, 8);

    memcpy((buf + 20), ((PTPSlaveState*)ptpState)->portIdentity.clockIdentity, CLOCK_IDENTITY_LENGTH);
    *(UInteger16 *)(buf + 28) = flip16(((PTPSlaveState*)ptpState)->portIdentity.portNumber);
    *(Integer8 *)(buf + 33) = 0x7F;
}

static void msgPackDelayReq(Octet *buf, void *ptpState)
{
    msgPackHeader(buf, ptpState);
    *(char *)(buf + 0) = (*(char *)(buf + 0) & 0xF0) | 0x01;
    *(UInteger16 *)(buf + 2) = flip16(DELAY_REQ_LENGTH);
    *(UInteger16 *)(buf + 30) = flip16(((PTPSlaveState*)ptpState)->delayReqSeqId);
    *(UInteger8 *)(buf + 32) = 0x01;
    *(Integer8 *)(buf + 33) = 0x7F;
}

/*============================ 消息发送函数 ============================*/
static void sendDelayReqMessage(void)
{
    Ethernet_Pkt_Desc pktDesc;

    memset(&pktDesc, 0, sizeof(Ethernet_Pkt_Desc));
    pktDesc.bufferLength = PACKET_LENGTH;
    pktDesc.dataOffset = 0;
    pktDesc.dataBuffer = (uint8_t *)delayReqMsg;
    pktDesc.nextPacketDesc = 0;
    pktDesc.flags = ETHERNET_PKT_FLAG_TTSE |
                    ETHERNET_PKT_FLAG_SOP |
                    ETHERNET_PKT_FLAG_EOP |
                    ETHERNET_PKT_FLAG_SA_INS |
                    ETHERNET_PKT_FLAG_CRC_PAD_INS;
    pktDesc.pktChannel = ETHERNET_DMA_CHANNEL_NUM_0;
    pktDesc.numPktFrags = 1;

    memset(delayReqMsg + 8, 0, PACKET_LENGTH - 8);
    msgPackDelayReq(delayReqMsg + 8, &gPtpSlaveState);

    pktDesc.pktLength = DELAY_REQ_LENGTH + 6 + 2;
    pktDesc.validLength = pktDesc.pktLength;

    Ethernet_sendPacket(emac_handle, &pktDesc);
}

/*============================ 时钟调整函数 ============================*/
// 时钟调整函数,根据offset调整Slave本地PTP时钟
void ptp_slave_adjust_clock(void)
{
    int64_t t1_ns, t2_ns, t3_ns, t4_ns;
    int64_t offset_ns, delay_ns;
    uint32_t curr_sec, curr_nsec;

    // 1. 转换所有时间戳为纳秒（统一单位）
    // t1: Master发送Sync的时间 (from Follow_Up)
    t1_ns = (int64_t)gPtpSlaveState.syncOriginTimestamp.secondsField.lsb * NS_PER_SEC +
            (int64_t)gPtpSlaveState.syncOriginTimestamp.nanosecondsField;

    // t2: Slave接收Sync的时间
    t2_ns = (int64_t)gPtpSlaveState.syncRecvTimestamp.secondsField.lsb * NS_PER_SEC +
            (int64_t)gPtpSlaveState.syncRecvTimestamp.nanosecondsField;

    // t3: Slave发送Delay_Req的时间
    t3_ns = (int64_t)gPtpSlaveState.delayReqSentTimestamp.secondsField.lsb * NS_PER_SEC +
            (int64_t)gPtpSlaveState.delayReqSentTimestamp.nanosecondsField;

    // t4: Master接收Delay_Req的时间
    t4_ns = (int64_t)gPtpSlaveState.delayReqRecvTimestamp.secondsField.lsb * NS_PER_SEC +
            (int64_t)gPtpSlaveState.delayReqRecvTimestamp.nanosecondsField;

    // 2.计算偏移量和路径延迟
    // MeanPathDelay = [(T2 - T1) + (T4 - T3)] / 2
    delay_ns  = ((t2_ns - t1_ns) - (t3_ns - t4_ns)) / 2;

    // Offset = T2 - T1 - MeanPathDelay
    offset_ns = (t2_ns - t1_ns) - delay_ns;

    // 3.读取当前Slave的PTP系统时间
    Ethernet_getSysTimePTP(EMAC_BASE, &curr_sec, &curr_nsec);
    int64_t curr_ns = (int64_t)curr_sec * NS_PER_SEC + curr_nsec;

    // 4.调整时间（减去offset，使Slave时钟对齐Master）
    int64_t new_ns = curr_ns - offset_ns;
    uint32_t new_sec = (uint32_t)(new_ns / NS_PER_SEC);
    uint32_t new_nsec = (uint32_t)(new_ns % NS_PER_SEC);

    // 5.写入调整后的时间到PTP系统时钟
    Ethernet_setSysTimePTP(EMAC_BASE, new_sec, new_nsec);

    // 6.更新统计
    gPtpSlaveState.clockUpdateCount++;

    // 7.更新锁定状态
    if (offset_ns < LOCK_THRESHOLD_NS && offset_ns > - LOCK_THRESHOLD_NS) {
        gPtpSlaveState.lockCount++;
        if (gPtpSlaveState.lockCount >= 10)
        {
            gPtpSlaveState.isLocked = true;
        }
    }
    else
    {
        gPtpSlaveState.lockCount = 0;
        gPtpSlaveState.isLocked = false;
    }

    // 8.首次同步完成标志
    static bool firstSyncDone = false;
    if (!firstSyncDone)
    {
        CmIpc_cm2cpu.PtpSynced = 1U;
        firstSyncDone = true;
    }

    // 9.更新PPS目标时间（确保PPS与整秒对齐）
    uint32_t sec, nsec;
    Ethernet_getSysTimePTP(EMAC_BASE, &sec, &nsec);

    // 设置下一个整秒为目标时间
    if (nsec > 500000000U)
    {
        sec++;
    }

    Ethernet_setTargetTimePPS(EMAC_BASE,
                              ETHERNET_MAC_PPS_OUT_INSTANCE_0,
                              sec + 1,
                              0);
}


/*============================ PTP Slave API ============================*/
// Slave端PTP初始化
void ptp_slave_init(void)
{
    uint32_t varPtpConfig = 0;
    uint32_t timeSec;
    uint32_t timeNanosec;
    float subSecondInc;

    // ptp configuration time control register
    varPtpConfig = (0U << ETHERNET_MAC_TIMESTAMP_CONTROL_SNAPTYPSEL_S) |
                    ETHERNET_MAC_TIMESTAMP_CONTROL_TSCTRLSSR |
                    ETHERNET_MAC_TIMESTAMP_CONTROL_TSMSTRENA |
                    ETHERNET_MAC_TIMESTAMP_CONTROL_TSEVNTENA |
                    ETHERNET_MAC_TIMESTAMP_CONTROL_TSVER2ENA |
                    ETHERNET_MAC_TIMESTAMP_CONTROL_TSIPENA;

    //
    // Subsecond increment is added to the systime counter every ptp clock tick
    // hence for Digital rollover, it is simply the time period of the clock tick.
    //
    subSecondInc = PTP_REF_CLOCK_PERIOD;

    Ethernet_setConfigTimestampPTP(EMAC_BASE, varPtpConfig, subSecondInc);
    Ethernet_enableSysTimePTP(EMAC_BASE);

    // Set Digital Rollover mode
    HWREG(EMAC_BASE + ETHERNET_O_MAC_TIMESTAMP_CONTROL) |= ETHERNET_MAC_TIMESTAMP_CONTROL_TSCTRLSSR;

    //
    // Start the system with a random value.
    //
    Ethernet_setSysTimePTP(EMAC_BASE, 0x4132EDCA, 0x25a5a5a5);

    //
    // We need to program this standard multicast address so that this device
    // identifies PTP over Ethernet packets correctly. "01:1B:19:00:00:00"
    //
    Ethernet_setMACAddr(EMAC_BASE,
                        1U,
                        0x00000000U,
                        0x00191B01U, // 01:1B:19:00:00:00
                        ETHERNET_CHANNEL_0);

    // Set PPS output to Purlse mode.
    // Interrupt mode: ETHERNET_MAC_PPS_CONTROL_TRGTMODSEL_INTERRUPT
    // Purlse mode: ETHERNET_MAC_PPS_CONTROL_TRGTMODSEL_PULSE
    Ethernet_selectTargetInterruptOrPulsePPS(
                        EMAC_BASE,
                        ETHERNET_MAC_PPS_OUT_INSTANCE_0,
                        ETHERNET_MAC_PPS_CONTROL_TRGTMODSEL_PULSE);

    // Forbbiden using PPS
    HWREG(EMAC_BASE + ETHERNET_O_MAC_PPS_CONTROL) &= ~ETHERNET_MAC_PPS_CONTROL_PPSEN0;

    // Set PPS Interval and Width(1Hz,10ms)
    HWREG(EMAC_BASE + ETHERNET_MAC_PPS_INTERVAL) = PTP_REF_CLOCK_FREQ - 1;
    HWREG(EMAC_BASE + ETHERNET_MAC_PPS_WIDTH) = PTP_REF_CLOCK_FREQ / 100;

    // Set PPSCTRL=1,PPSEN0=0, TRGTMODSEL=3
    uint32_t ppsCtrl = ETHERNET_MAC_PPS_CONTROL_PPSCTRL_PPS_OUTPUT_1HZ;
    ppsCtrl |= (0x3U << ETHERNET_MAC_PPS_CONTROL_TRGTMODSEL0_S);
    HWREG(EMAC_BASE + ETHERNET_O_MAC_PPS_CONTROL) = ppsCtrl; // 0x61

    // // init Slave state
    memset(&gPtpSlaveState, 0, sizeof(PTPSlaveState));
    InitSlaveConstants(&gPtpSlaveState);
}

void ptp_slave_run(void)
{
    // 1.已收到Sync, 需要等待Follow_Up
    if (gPtpSlaveState.syncReceived && !gPtpSlaveState.followUpReceived)
    {
        return;
    }

    // 2.已收到Follow_Up, 还没Delay_Resp, 开始发送Delay_Req报文（并记录发送时间t3）
    if (gPtpSlaveState.followUpReceived && !gPtpSlaveState.delayRespReceived)
    {

        // 记录Delay_Req发送时间（t3）
        TimeInternal t3;
        getTime(&t3);
        gPtpSlaveState.delayReqSentTimestamp.secondsField.lsb = t3.seconds;
        gPtpSlaveState.delayReqSentTimestamp.secondsField.msb = 0;
        gPtpSlaveState.delayReqSentTimestamp.nanosecondsField = t3.nanoseconds;

        // 发送Delay_Req报文
        sendDelayReqMessage();
        gPtpSlaveState.delayReqSeqId++;
        return;
    }

    // 3.如果收到Delay_Resp，调整时钟
    if (gPtpSlaveState.delayRespReceived)
    {
        ptp_slave_adjust_clock();

        // 复位状态，准备下一轮
        gPtpSlaveState.syncReceived      = FALSE;
        gPtpSlaveState.followUpReceived  = FALSE;
        gPtpSlaveState.delayRespReceived = FALSE;
    }

}



