/*
 * ptp_master_sync.c
 *
 * TMS320F28388D PTP Master 实现
 *
 *  功能：
 *    1. PTP主时钟初始化
 *    2. 周期性发送Sync消息（捕获t1）
 *    3. 发送FollowUp消息（携带t1）
 *    4. 响应DelayReq消息发送DelayResp（携带t4）
 *    5. 输出PPS信号（通过MAC PPS功能到GPIO47）
 *
 *  Created on: 2026年3月31日
 *      Author: whl
 */
#include "ptp_master_sync.h"
#include "Eth_mii.h"
#include "bsp.h"

#define ETHERNET_MAC_TIMESTAMP_CONTROL_TSCFUPDT 0x00000020U

#define ETHERNET_O_MAC_PPS_TARGET_TIME_SECONDS  0x00000704U
#define ETHERNET_O_MAC_PPS_TARGET_TIME_NANOSECONDS  0x00000708U
#define ETHERNET_O_MAC_PPS_INTERVAL             0x0000070CU
#define ETHERNET_O_MAC_PPS_WIDTH                0x00000710U

static uint32_t gSyncIntervalNs = 1000000000UL; // 1s
static uint32_t gLastSyncTimeNs = 0;


static void InitConstants(PTPMasterState *ptpMasterState)
{
    uint32_t mac_low,mac_high, i, j;
    uint8_t *pucTemp;

    Ethernet_getMACAddr(EMAC_BASE, 0, &mac_high, &mac_low);

    pucTemp = (uint8_t *)&mac_low;
    ptpMasterState->port_uuid_field[0] = pucTemp[0];
    ptpMasterState->port_uuid_field[1] = pucTemp[1];
    ptpMasterState->port_uuid_field[2] = pucTemp[2];
    ptpMasterState->port_uuid_field[3] = pucTemp[3];

    pucTemp = (uint8_t *)&mac_high;
    ptpMasterState->port_uuid_field[4] = pucTemp[0];
    ptpMasterState->port_uuid_field[5] = pucTemp[1];

    // Init global constants.
    for (i = 0, j = 0; i < CLOCK_IDENTITY_LENGTH; i++)
    {
       if (i == 3) ptpMasterState->portIdentity.clockIdentity[i] = 0xFF;
       else if (i == 4) ptpMasterState->portIdentity.clockIdentity[i] = 0xFE;
       else
       {
           ptpMasterState->portIdentity.clockIdentity[i] =
                   ptpMasterState->port_uuid_field[j];
           j++;
       }
    }

    ptpMasterState->portIdentity.portNumber = 1;
    ptpMasterState->syncSeqId = 0;
    ptpMasterState->syncTimestampAvailable = false;
    ptpMasterState->sendingDelayResp = false;
}

// PTP消息打包函数
static void msgPackHeader(Octet *buf, void *ptpState)
{
    *(UInteger8 *)(buf + 0) = 0x80;
    *(UInteger4 *)(buf + 1) = 0x02;
    *(UInteger8 *)(buf + 4) = 0;
    *(UInteger8 *)(buf + 6) = PTP_TWO_STEP ? 0x02 : 0;
    *(UInteger8 *)(buf + 7) = 0;
    memset((buf + 8), 0, 8);

    memcpy((buf + 20), ((PTPMasterState*)ptpState)->portIdentity.clockIdentity, CLOCK_IDENTITY_LENGTH);
    *(UInteger16 *)(buf + 28) = flip16(((PTPMasterState*)ptpState)->portIdentity.portNumber);
    *(Integer8 *)(buf + 33) = 0x7F;
}

static void msgPackSync(Octet *buf, void *ptpState)
{
    msgPackHeader(buf, ptpState);
    *(char *)(buf + 0) = (*(char *)(buf + 0) & 0xF0) | 0x00;
    *(UInteger16 *)(buf + 2) = flip16(SYNC_LENGTH);
    *(UInteger16 *)(buf + 30) = flip16(((PTPMasterState*)ptpState)->syncSeqId);
    *(UInteger8 *)(buf + 32) = 0x00;
    *(Integer8 *)(buf + 33) = 0;

    if (!PTP_TWO_STEP) {
        *(UInteger16 *)(buf + 34) = flip16(((PTPMasterState*)ptpState)->syncTimestamp.secondsField.msb);
        *(UInteger32 *)(buf + 36) = flip32(((PTPMasterState*)ptpState)->syncTimestamp.secondsField.lsb);
        *(UInteger32 *)(buf + 40) = flip32(((PTPMasterState*)ptpState)->syncTimestamp.nanosecondsField);
    }
}

static void msgPackFollowUp(Octet *buf, void *ptpState)
{
    msgPackHeader(buf, ptpState);
    *(char *)(buf + 0) = (*(char *)(buf + 0) & 0xF0) | 0x08;
    *(UInteger16 *)(buf + 2) = flip16(FOLLOW_UP_LENGTH);
    *(UInteger16 *)(buf + 30) = flip16(((PTPMasterState*)ptpState)->syncSeqId);
    *(UInteger8 *)(buf + 32) = 0x02;
    *(Integer8 *)(buf + 33) = 0;

    *(UInteger16 *)(buf + 34) = flip16(((PTPMasterState*)ptpState)->syncTimestamp.secondsField.msb);
    *(UInteger32 *)(buf + 36) = flip32(((PTPMasterState*)ptpState)->syncTimestamp.secondsField.lsb);
    *(UInteger32 *)(buf + 40) = flip32(((PTPMasterState*)ptpState)->syncTimestamp.nanosecondsField);
}

static void msgPackDelayResp(Octet *buf, void *ptpState)
{
    PTPMasterState *master = (PTPMasterState*)ptpState;

    msgPackHeader(buf, ptpState);
    *(char *)(buf + 0) = (*(char *)(buf + 0) & 0xF0) | 0x09;
    *(UInteger16 *)(buf + 2) = flip16(DELAY_RESP_LENGTH);
    *(UInteger8 *)(buf + 4) = master->delayReqHeader.domainNumber;
    *(UInteger16 *)(buf + 30) = flip16(master->delayReqHeader.sequenceId);
    *(UInteger8 *)(buf + 32) = 0x03;
    *(Integer8 *)(buf + 33) = 0;

    *(UInteger16 *)(buf + 34) = flip16(master->delayReqRecvTimestamp.secondsField.msb);
    *(UInteger32 *)(buf + 36) = flip32(master->delayReqRecvTimestamp.secondsField.lsb);
    *(UInteger32 *)(buf + 40) = flip32(master->delayReqRecvTimestamp.nanosecondsField);

    memcpy((buf + 44), master->delayReqHeader.sourcePortIdentity.clockIdentity, CLOCK_IDENTITY_LENGTH);
    *(UInteger16 *)(buf + 52) = flip16(master->delayReqHeader.sourcePortIdentity.portNumber);
}

// 消息发送函数
static void sendMessage(Octet *msg, uint8_t msgType, void *ptpState, Ethernet_Pkt_Desc *pktDesc)
{
    uint32_t pktLen;

    memset(pktDesc, 0, sizeof(Ethernet_Pkt_Desc));
    pktDesc->bufferLength = PACKET_LENGTH;
    pktDesc->dataOffset = 0;
    pktDesc->dataBuffer = (uint8_t *)msg;
    pktDesc->nextPacketDesc = 0;
    pktDesc->flags = ETHERNET_PKT_FLAG_SOP |
                     ETHERNET_PKT_FLAG_EOP |
                     ETHERNET_PKT_FLAG_SA_INS |
                     ETHERNET_PKT_FLAG_CRC_PAD_INS;
    pktDesc->pktChannel = ETHERNET_DMA_CHANNEL_NUM_0;
    pktDesc->numPktFrags = 1;

    memset(msg + 8, 0, PACKET_LENGTH - 8);

    switch(msgType) {
        case SYNC:
            pktDesc->flags |= ETHERNET_PKT_FLAG_TTSE;
            msgPackSync(msg + 8, ptpState);
            pktLen = SYNC_LENGTH;
            break;
        case FOLLOW_UP:
            msgPackFollowUp(msg + 8, ptpState);
            pktLen = FOLLOW_UP_LENGTH;
            ((PTPMasterState*)ptpState)->syncSeqId++;
            break;
        case DELAY_RESP:
            msgPackDelayResp(msg + 8, ptpState);
            pktLen = DELAY_RESP_LENGTH;
            break;
        default:
            return;
    }

    pktDesc->pktLength = pktLen + 6 + 2;
    pktDesc->validLength = pktDesc->pktLength;

    Ethernet_sendPacket(emac_handle, pktDesc);
}

/*============================ PTP Master API ============================*/

void ptp_master_init()
{
    uint32_t i;
    uint32_t varPtpConfig = 0;
    uint32_t timeSec;
    uint32_t timeNanosec;
    float subSecondInc;

    // ptp configuration time control register
    varPtpConfig = (0 << ETHERNET_MAC_TIMESTAMP_CONTROL_SNAPTYPSEL_S) |
                    ETHERNET_MAC_TIMESTAMP_CONTROL_TSCTRLSSR |   // Timestamp Digital or Binary Rollover
                    ETHERNET_MAC_TIMESTAMP_CONTROL_TSMSTRENA |   // Master or Slave mode
                    ETHERNET_MAC_TIMESTAMP_CONTROL_TSEVNTENA |   // ptp enable
                    ETHERNET_MAC_TIMESTAMP_CONTROL_TSVER2ENA |   // IEEE1588 v2 support
                    ETHERNET_MAC_TIMESTAMP_CONTROL_TSIPENA;      // timestamp insert enable

    // Subsecond incrSement is added to the systime counter every ptp clock tick
    // hence for Digital rollover, it is simply the time period of the clock tick.
    subSecondInc = PTP_REF_CLOCK_PERIOD;

    Ethernet_setConfigTimestampPTP(EMAC_BASE, varPtpConfig, subSecondInc);
    Ethernet_enableSysTimePTP(EMAC_BASE);

    // Set Digital Rollover mode
    HWREG(EMAC_BASE + ETHERNET_O_MAC_TIMESTAMP_CONTROL) |= ETHERNET_MAC_TIMESTAMP_CONTROL_TSCTRLSSR;

    // Start the system with a random value
    Ethernet_setSysTimePTP(EMAC_BASE, 0x4132EDCA, 0x25a5a5a5);

    // configuration normal ptp mac addr: 01:1B:19:00:00:00
    Ethernet_setMACAddr(EMAC_BASE,
                        1,
                        0x00000000,
                        0x00191B01,
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
    ppsCtrl |= (0x3U << ETHERNET_MAC_PPS_CONTROL_TRGTMODSEL0_S);  // 0x61
    HWREG(EMAC_BASE + ETHERNET_O_MAC_PPS_CONTROL) = ppsCtrl;      // 0x61

    //
    // We need to set a standard defined Multicast address : 01:1B:19:00:00:00
    // as the Destination address in the ethernet frame and that is how the
    // receiver will recognize it as a valid PTP over Ethernet packet.
    //
    i=0; *((uint32_t *)gMsgBuf + i) = 0x00191B01;
    i++; *((uint32_t *)gMsgBuf + i)  = 0xF7880000;

    // init Master state
    InitConstants(&gPtpMasterState);
    gLastSyncTimeNs = 0;

}

void ptp_master_run()
{
    static uint32_t lastSyncTime = 0ULL;
    uint32_t timeSec, timeNanosec;
    const uint32_t TIMEOUT_MAX = 2000000U;
    uint32_t timeout = 0U;

    //
    // Use the system time counter to send the sync + followup messages
    // every one second.
    //
    Ethernet_getSysTimePTP(EMAC_BASE, &timeSec, &timeNanosec);
    Ethernet_setTargetTimePPS(EMAC_BASE, ETHERNET_MAC_PPS_OUT_INSTANCE_0,
                              timeSec + 1, timeNanosec);

    // Set Digital Rollover mode
    HWREG(EMAC_BASE + ETHERNET_O_MAC_TIMESTAMP_CONTROL) |= ETHERNET_MAC_TIMESTAMP_CONTROL_TSCTRLSSR;

    //
    // Waiting till the target time that we set above is reached.
    // We're using the PPSOUT instance 0 as the timer.
    //
    while((((HWREG(EMAC_BASE + ETHERNET_O_MAC_TIMESTAMP_STATUS)) &
             (ETHERNET_MAC_TIMESTAMP_STATUS_TSTARGT0)) == 0) &&
             (timeout < TIMEOUT_MAX))
    {
        timeout++;
    }

//    if(timeout >= TIMEOUT_MAX)
//    {
//        CmIpc_cm2cpu.IpcCpu2Cm_Fault = 1;
//        return;
//    }

    // clear flag
    HWREG(EMAC_BASE + ETHERNET_O_MAC_TIMESTAMP_STATUS) = ETHERNET_MAC_TIMESTAMP_STATUS_TSTARGT0;

    // 3.发送SYNC报文（捕获t1）
    gPtpMasterState.syncTimestampAvailable = FALSE;
    sendMessage((Octet *)gMsgBuf, SYNC, &gPtpMasterState, &gPktDesc);

    // 4.等待时间戳捕获
    timeout = 0;
    while((gPtpMasterState.syncTimestampAvailable == FALSE) && (timeout < TIMEOUT_MAX))
    {
        timeout++;
    }

    // 超时后填充估算时间戳到syncTimestampdd
    if(timeout >= TIMEOUT_MAX)
    {
        CmIpc_cm2cpu.IpcCpu2Cm_Fault = 1U;
        uint32_t estSec, estNs;
        Ethernet_getSysTimePTP(EMAC_BASE, &estSec, &estNs);

        gPtpMasterState.syncTimestamp.secondsField.lsb = estSec;
        gPtpMasterState.syncTimestamp.secondsField.msb = 0;
        gPtpMasterState.syncTimestamp.nanosecondsField = estNs;
        gPtpMasterState.syncTimestampAvailable = TRUE;
    }

    // 5.发送FOLLOW_UP（携带t1）
    sendMessage((Octet *)gMsgBuf, FOLLOW_UP, &gPtpMasterState, &gPktDesc);
}


