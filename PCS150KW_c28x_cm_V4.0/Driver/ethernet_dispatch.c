/*
 * ethernet_dispatch.c
 *
 *  Created on: 2026年3月31日
 *      Author: whl
 */
#include "ethernet_dispatch.h"
#include "driverlib_cm/ethernet.h"
#include "limits.h"

#define PTP_OFM_NANOSECONDS_CUTOFF  10000U
#define ONE_BILLION                 1000000000

uint32_t Ethernet_numRxCallbackCustom = 0;
uint32_t releaseTxCount = 0;

PTPMasterState gPtpMasterState = {0};
PTPSlaveState gPtpSlaveState = {0};

uint8_t gMsgBuf[PACKET_LENGTH] = {0};

Ethernet_Handle emac_handle = NULL;
Ethernet_Device Ethernet_device_struct;

//uint32_t Ethernet_numGetPacketBufferCallback = 0;

uint32_t sendPacketFailedCount = 0;

Ethernet_Pkt_Desc pktDescriptorRXCustom[NUM_PACKET_DESC_RX_APPLICATION] = {0};

uint8_t delayReqMsg[PACKET_LENGTH] = {0};

Ethernet_Pkt_Desc gPktDesc;


/*========================== 时间处理函数 =========================*/
static void fromInternalTime(TimeInternal *internal, Timestamp *external);
static void msgUnpackHeader(Octet *buf, MsgHeader *header);
static void toInternalTime(TimeInternal *internal, Timestamp *external);
static void getTime(TimeInternal *time);
static void setTime(TimeInternal *time);
static void subTime(TimeInternal *r, const TimeInternal *x, const TimeInternal *y);
static void addTime(TimeInternal *r, const TimeInternal *x, const TimeInternal *y);
static void div2Time(TimeInternal *r);
static void normalizeTime(TimeInternal *r);

/*========================== PTP消息打包函数 =========================*/
static void msgPackHeader(Octet *buf, void *ptpState);
static void msgPackSync(Octet *buf, void *ptpState);
static void msgPackFollowUp(Octet *buf, void *ptpState);
static void msgPackDelayResp(Octet *buf, void *ptpState);
static void msgPackDelayReq(Octet *buf, void *ptpState);

/*========================== 消息发送函数 =========================*/
static void sendMessage(Octet *msg, uint8_t msgType, void *ptpState, Ethernet_Pkt_Desc *pktDesc);



//*****************************************************************************
//
//  This function is a callback function called by the example to
//  get a Packet Buffer. Has to return a ETHERNET_Pkt_Desc Structure.
//  Rewrite this API for custom use case.
//  报文缓冲区管理
//*****************************************************************************
Ethernet_Pkt_Desc* Ethernet_getPacketBufferCustom(void)
{
    //
    // Get the next packet descriptor from the descriptor pool
    //
    uint32_t shortIndex = (Ethernet_numGetPacketBufferCallback + 3)
                % NUM_PACKET_DESC_RX_APPLICATION;

    //
    // Increment the book-keeping pointer which acts as a head pointer
    // to the circular array of packet descriptor pool.
    //
    Ethernet_numGetPacketBufferCallback++;

    //
    // Update buffer length information to the newly procured packet
    // descriptor.
    //
    pktDescriptorRXCustom[shortIndex].bufferLength =
                                  ETHERNET_MAX_PACKET_LENGTH;

    //
    // Update the receive buffer address in the packer descriptor.
    //
    pktDescriptorRXCustom[shortIndex].dataBuffer =
            &Ethernet_device_struct.rxBuffer[(ETHERNET_MAX_PACKET_LENGTH*Ethernet_device_struct.rxBuffIndex)];

    //
    // Update the receive buffer pool index.
    //
    Ethernet_device_struct.rxBuffIndex += 1U;
    Ethernet_device_struct.rxBuffIndex  =
            (Ethernet_device_struct.rxBuffIndex % ETHERNET_NO_OF_RX_PACKETS);

    //
    // Receive buffer is usable from Address 0
    //
    pktDescriptorRXCustom[shortIndex].dataOffset = 0U;

    //
    // Return this new descriptor to the driver.
    //
    return (&(pktDescriptorRXCustom[shortIndex]));
}

void Ethernet_releaseTxPacketBufferPtp(
        Ethernet_Handle handleApplication,
        Ethernet_Pkt_Desc *pPacket)
{
    uint8_t *data;
    uint8_t msgType;

    if (pPacket == NULL)
    {
        return;
    }

    // get message type
    data = pPacket->dataBuffer + pPacket->dataOffset;
    msgType = data[PTP_HEADER_OFFSET] & 0x0F;

    if(g_ptpMode == 0) // Master模式
    {
        // Master: 捕获Sync发送时间戳t1
        if (msgType == SYNC && gPtpMasterState.syncTimestampAvailable == FALSE)
        {
            gPtpMasterState.syncTimestamp.nanosecondsField = pPacket->timeStampLow;
            gPtpMasterState.syncTimestamp.secondsField.lsb = pPacket->timeStampHigh;
            gPtpMasterState.syncTimestamp.secondsField.msb = 0;
            gPtpMasterState.syncTimestampAvailable = TRUE;
        }
        else if(msgType == DELAY_RESP) // Master: DelayResp发送完成，清除标志
        {
            gPtpMasterState.sendingDelayResp = FALSE;
        }
    }
    else  // Slave模式
    {
        // 捕获Delay_Req发送时间戳t3
        if (msgType == DELAY_REQ)
        {
            gPtpSlaveState.delayReqSentTimestamp.nanosecondsField = pPacket->timeStampLow;
            gPtpSlaveState.delayReqSentTimestamp.secondsField.lsb = pPacket->timeStampHigh;
            gPtpSlaveState.delayReqSentTimestamp.secondsField.msb = 0;
        }
    }

    // Increment the book-keeping counter.
#if ETHERNET_DEBUG
    releaseTxCount++;
#endif
}

/*============================ 接收回调 ============================*/
Ethernet_Pkt_Desc* Ethernet_receivePacketCallbackPtp(
        Ethernet_Handle handleApplication,
        Ethernet_Pkt_Desc *pPacket)
{
    MsgHeader msgHeader;
    TimeInternal recvTime;
    TimeInternal sendTime;
    uint8_t *data;
    uint16_t ethertype;

    if (pPacket == NULL)
    {
        return pPacket;
    }

    data = pPacket->dataBuffer + pPacket->dataOffset;

    // 检查Ethertype是否为PTP(0x88F7)
    ethertype = ((uint16_t)data[12] << 8) | data[13];
    if (ethertype != 0x88F7)
    {
        // 非PTP报文，直接返回
        return pPacket;
    }

    if (g_ptpMode == 0)
    {
        // ==================== Master模式 ====================
        msgUnpackHeader((Octet *)(data + PTP_HEADER_OFFSET), &gPtpMasterState.delayReqHeader);

        switch (gPtpMasterState.delayReqHeader.messageType) {
            case DELAY_REQ: // 捕获DelayReq接收时间戳t4
                gPtpMasterState.delayReqRecvTimestamp.nanosecondsField = pPacket->timeStampLow;
                gPtpMasterState.delayReqRecvTimestamp.secondsField.lsb = pPacket->timeStampHigh;
                gPtpMasterState.delayReqRecvTimestamp.secondsField.msb = 0;
                gPtpMasterState.sendingDelayResp = TRUE;

                // 发送DelayResp
                sendMessage((Octet *)gMsgBuf, DELAY_RESP, &gPtpMasterState, &gPktDesc);
                break;

            default:
                break;
        }
    }
    else
    {
        // ==================== Slave模式 ====================
        msgUnpackHeader((Octet *)(data + PTP_HEADER_OFFSET), &msgHeader);

        switch (msgHeader.messageType)
        {
            case SYNC:  // 捕获Sync接收时间戳t2
                gPtpSlaveState.syncRecvTimestamp.nanosecondsField = pPacket->timeStampLow;
                gPtpSlaveState.syncRecvTimestamp.secondsField.lsb = pPacket->timeStampHigh;
                gPtpSlaveState.syncRecvTimestamp.secondsField.msb = 0;
                gPtpSlaveState.lastSyncSeqId = msgHeader.sequenceId;
                gPtpSlaveState.syncReceived = TRUE;
                gPtpSlaveState.followUpReceived = FALSE;
                break;

            case FOLLOW_UP:  // 提取FollowUp中的t1时间戳
                if (msgHeader.sequenceId == gPtpSlaveState.lastSyncSeqId)
                {
                    gPtpSlaveState.syncOriginTimestamp.secondsField.msb =
                        flip16(*(UInteger16 *)(pPacket->dataBuffer + PTP_HEADER_OFFSET + 34));
                    gPtpSlaveState.syncOriginTimestamp.secondsField.lsb =
                        flip32(*(UInteger32 *)(pPacket->dataBuffer + PTP_HEADER_OFFSET + 36));
                    gPtpSlaveState.syncOriginTimestamp.nanosecondsField =
                        flip32(*(UInteger32 *)(pPacket->dataBuffer + PTP_HEADER_OFFSET + 40));

                    toInternalTime(&sendTime, &gPtpSlaveState.syncOriginTimestamp);
                    toInternalTime(&recvTime, &gPtpSlaveState.syncRecvTimestamp);
                    subTime(&gPtpSlaveState.delayMS, &recvTime, &sendTime);

                    gPtpSlaveState.followUpReceived = TRUE;
                }
                break;

            case DELAY_RESP:  // 提取DelayResp中的t4时间戳
                if (gPtpSlaveState.followUpReceived)
                {
                    gPtpSlaveState.delayReqRecvTimestamp.secondsField.msb =
                        flip16(*(UInteger16 *)(pPacket->dataBuffer + PTP_HEADER_OFFSET + 34));
                    gPtpSlaveState.delayReqRecvTimestamp.secondsField.lsb =
                        flip32(*(UInteger32 *)(pPacket->dataBuffer + PTP_HEADER_OFFSET + 36));
                    gPtpSlaveState.delayReqRecvTimestamp.nanosecondsField =
                        flip32(*(UInteger32 *)(pPacket->dataBuffer + PTP_HEADER_OFFSET + 40));

                    toInternalTime(&sendTime, &gPtpSlaveState.delayReqSentTimestamp);
                    toInternalTime(&recvTime, &gPtpSlaveState.delayReqRecvTimestamp);
                    subTime(&gPtpSlaveState.delaySM, &recvTime, &sendTime);

                    addTime(&gPtpSlaveState.meanPathDelay, &gPtpSlaveState.delaySM, &gPtpSlaveState.delayMS);
                    div2Time(&gPtpSlaveState.meanPathDelay);

                    gPtpSlaveState.delayRespReceived = TRUE;
                }
                break;

            default:
                break;
        }
    }
#ifdef ETHERNET_DEBUG
    Ethernet_numRxCallbackCustom++;
#endif
    return Ethernet_getPacketBufferCustom();
}

void msgUnpackHeader(Octet *buf, MsgHeader *header)
{
    header->transportSpecific = (*(Nibble *)(buf + 0)) >> 4;
    header->messageType = (*(Enumeration4 *)(buf + 0)) & 0x0F;
    header->versionPTP = (*(UInteger4 *)(buf + 1)) & 0x0F;
    header->messageLength = flip16(*(UInteger16 *)(buf + 2));
    header->domainNumber = (*(UInteger8 *)(buf + 4));
    memcpy(header->flagField, (buf + 6), FLAG_FIELD_LENGTH);

    memcpy(&header->correctionfield, (buf + 8), 8);

    memcpy(header->sourcePortIdentity.clockIdentity, (buf + 20), CLOCK_IDENTITY_LENGTH);
    header->sourcePortIdentity.portNumber = flip16(*(UInteger16 *)(buf + 28));
    header->sequenceId = flip16(*(UInteger16 *)(buf + 30));
    header->controlField = (*(UInteger8 *)(buf + 32));
    header->logMessageInterval = (*(Integer8 *)(buf + 33));
}

void msgPackHeader(Octet *buf, void *ptpState)
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

void msgPackSync(Octet *buf, void *ptpState)
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

void msgPackFollowUp(Octet *buf, void *ptpState)
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

void msgPackDelayReq(Octet *buf, void *ptpState)
{
    msgPackHeader(buf, ptpState);
    *(char *)(buf + 0) = (*(char *)(buf + 0) & 0xF0) | 0x01;
    *(UInteger16 *)(buf + 2) = flip16(DELAY_REQ_LENGTH);
    *(UInteger16 *)(buf + 30) = flip16(((PTPSlaveState*)ptpState)->delayReqSeqId);
    *(UInteger8 *)(buf + 32) = 0x01;
    *(Integer8 *)(buf + 33) = 0x7F;
}

void msgPackDelayResp(Octet *buf, void *ptpState)
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


/*============================ 消息发送函数 ============================*/
void sendMessage(Octet *msg, uint8_t msgType, void *ptpState, Ethernet_Pkt_Desc *pktDesc)
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

    switch(msgType)
    {
        case SYNC:      // Master发送sync
            pktDesc->flags |= ETHERNET_PKT_FLAG_TTSE;
            msgPackSync(msg + 8, ptpState);
            pktLen = SYNC_LENGTH;
            break;
        case FOLLOW_UP:  // Master发送followUp
            msgPackFollowUp(msg + 8, ptpState);
            pktLen = FOLLOW_UP_LENGTH;
            ((PTPMasterState*)ptpState)->syncSeqId++;
            break;
        case DELAY_REQ:  // Slave发送Delay_Req
            msgPackDelayReq(msg + 8, ptpState);
            pktLen = DELAY_REQ_LENGTH;
            ((PTPSlaveState*)ptpState)->delayReqSeqId++;
            break;
        case DELAY_RESP: // Master发送DelayResp
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


/*============================ 时间操作函数 ============================*/

void fromInternalTime(TimeInternal *internal, Timestamp *external)
{
    external->secondsField.lsb = internal->seconds;
    external->nanosecondsField = internal->nanoseconds;
    external->secondsField.msb = 0;
}

void toInternalTime(TimeInternal *internal, Timestamp *external)
{
    internal->seconds = external->secondsField.lsb;
    internal->nanoseconds = external->nanosecondsField;
}

void getTime(TimeInternal *time)
{
    Ethernet_getSysTimePTP(EMAC_BASE, (uint32_t *)&time->seconds, (uint32_t *)&time->nanoseconds);
}

void setTime(TimeInternal *time)
{
    Ethernet_setSysTimePTP(EMAC_BASE, (uint32_t)time->seconds, (uint32_t)time->nanoseconds);
}

#define ONE_BILLION 1000000000

void normalizeTime(TimeInternal *r)
{
    r->seconds += r->nanoseconds / ONE_BILLION;
    r->nanoseconds -= (r->nanoseconds / ONE_BILLION) * ONE_BILLION;

    if (r->seconds > 0 && r->nanoseconds < 0) {
        r->seconds -= 1;
        r->nanoseconds += ONE_BILLION;
    } else if (r->seconds < 0 && r->nanoseconds > 0) {
        r->seconds += 1;
        r->nanoseconds -= ONE_BILLION;
    }
}

void subTime(TimeInternal *r, const TimeInternal *x, const TimeInternal *y)
{
    r->seconds = x->seconds - y->seconds;
    r->nanoseconds = x->nanoseconds - y->nanoseconds;
    normalizeTime(r);
}

void addTime(TimeInternal *r, const TimeInternal *x, const TimeInternal *y)
{
    r->seconds = x->seconds + y->seconds;
    r->nanoseconds = x->nanoseconds + y->nanoseconds;
    normalizeTime(r);
}

void div2Time(TimeInternal *r)
{
    r->nanoseconds += (r->seconds % 2) * ONE_BILLION;
    r->seconds /= 2;
    r->nanoseconds /= 2;
    normalizeTime(r);
}

