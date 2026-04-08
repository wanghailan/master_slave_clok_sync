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


// Function prototypes used in this case
Ethernet_Pkt_Desc* Ethernet_getPacketBufferCustom(void);

void fromInternalTime(TimeInternal * internal, Timestamp * external);

void sendMessage(Octet *msg, Enumeration4 msgType, void *ptpState, Ethernet_Pkt_Desc *pktDesc);

void msgUnpackHeader(Octet * buf, MsgHeader * header);

void msgPackSync(Octet * buf, void *ptpState);

void msgPackFollowUp(Octet * buf, void *ptpState);

void msgPackHeader(Octet * buf, void *ptpState);

void msgPackDelayResp(Octet * buf, void *ptpState);

void msgPackDelayReq(Octet * buf, void *ptpState);

void toInternalTime(TimeInternal * internal, Timestamp * external);

void subTime(TimeInternal * r, const TimeInternal * x, const TimeInternal * y);

void addTime(TimeInternal * r, const TimeInternal * x, const TimeInternal * y);

void div2Time(TimeInternal *r);

void getTime(TimeInternal *time);

void setTime(TimeInternal *time);

void updateClock(void);

void normalizeTime(TimeInternal * r);


//*****************************************************************************
//
//  This function is a callback function called by the example to
//  get a Packet Buffer. Has to return a ETHERNET_Pkt_Desc Structure.
//  Rewrite this API for custom use case.
//
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
                                      &Ethernet_device_struct.rxBuffer [
               (ETHERNET_MAX_PACKET_LENGTH*Ethernet_device_struct.rxBuffIndex)];

    //
    // Update the receive buffer pool index.
    //
    Ethernet_device_struct.rxBuffIndex += 1U;
    Ethernet_device_struct.rxBuffIndex  =
            (Ethernet_device_struct.rxBuffIndex%ETHERNET_NO_OF_RX_PACKETS);

    //
    // Receive buffer is usable from Address 0
    //
    pktDescriptorRXCustom[shortIndex].dataOffset = 0U;

    //
    // Return this new descriptor to the driver.
    //
    return (&(pktDescriptorRXCustom[shortIndex]));
}

//*****************************************************************************
//
//  This is a hook function and called by the driver when it receives a
//  packet. Application is expected to replenish the buffer after consuming it.
//  Has to return a ETHERNET_Pkt_Desc Structure.
//  Rewrite this API for custom use case.
//
//*****************************************************************************
void fromInternalTime(TimeInternal * internal, Timestamp * external)
{
    external->secondsField.lsb = internal->seconds;
    external->nanosecondsField = internal->nanoseconds;
    external->secondsField.msb = 0;
}

void Ethernet_releaseTxPacketBufferPtp(
        Ethernet_Handle handleApplication,
        Ethernet_Pkt_Desc *pPacket)
{
    if(g_ptpMode == 0) // Master模式
    {
        // 捕获DelayResp时间戳
        if(gPtpMasterState.sendingDelayResp == TRUE)
        {
            gPtpMasterState.sendingDelayResp = FALSE;
        }
        else if(gPtpMasterState.syncTimestampAvailable == FALSE)
        {
            gPtpMasterState.syncTimestamp.nanosecondsField = pPacket->timeStampLow;
            gPtpMasterState.syncTimestamp.secondsField.lsb = pPacket->timeStampHigh;
            gPtpMasterState.syncTimestamp.secondsField.msb = 0;
            gPtpMasterState.syncTimestampAvailable = TRUE;
        }
    }
    else  // Slave模式, 捕获Delay_Req发送时间戳
    {
        gPtpSlaveState.delayReqSentTimestamp.nanosecondsField = pPacket->timeStampLow;
        gPtpSlaveState.delayReqSentTimestamp.secondsField.lsb = pPacket->timeStampHigh;
        gPtpSlaveState.delayReqSentTimestamp.secondsField.msb = 0;
        gPtpSlaveState.delayRespReceived = TRUE;
    }

    // Increment the book-keeping counter.
#if ETHERNET_DEBUG
    releaseTxCount++;
#endif
}


// 收发消息回调
Ethernet_Pkt_Desc* Ethernet_receivePacketCallbackPtp(
        Ethernet_Handle handleApplication,
        Ethernet_Pkt_Desc *pPacket)
{
    MsgHeader msgHeader;
    TimeInternal recvTime;
    TimeInternal sendTime;
    uint32_t i;

    // 模式选择，Master or Slave
    if (g_ptpMode == 0) // Master模式
    {
        msgUnpackHeader((Octet*)(pPacket->dataBuffer + PTP_HEADER_OFFSET),
                        &gPtpMasterState.delayReqHeader);

        switch(gPtpMasterState.delayReqHeader.messageType)
        {
        case DELAY_REQ:
            // Simply get the timestamp and send the delay response packet asap.
            gPtpMasterState.delayReqRecvTimestamp.nanosecondsField = pPacket->timeStampLow;
            gPtpMasterState.delayReqRecvTimestamp.secondsField.lsb = pPacket->timeStampHigh;
            gPtpMasterState.delayReqRecvTimestamp.secondsField.msb = 0;
            gPtpMasterState.sendingDelayResp = TRUE;

            // Send the corresponding Delay Response packet.
            sendMessage((Octet *)gMsgBuf, DELAY_RESP, &gPtpMasterState, &gPktDesc);
            break;
        default:
            break;
        }
    }
    else  // Slave模式
    {
        msgUnpackHeader((Octet*)(pPacket->dataBuffer + PTP_HEADER_OFFSET), &msgHeader);
        switch(msgHeader.messageType)
        {
        case SYNC:  // 记录Sync接收时间t2
            // 1. Save the Sync receive timestamp
            gPtpSlaveState.syncRecvTimestamp.nanosecondsField =
                    pPacket->timeStampLow;
            gPtpSlaveState.syncRecvTimestamp.secondsField.lsb =
                    pPacket->timeStampHigh;
            gPtpSlaveState.syncRecvTimestamp.secondsField.msb = 0;
            gPtpSlaveState.lastSyncSeqId = msgHeader.sequenceId;
            gPtpSlaveState.syncReceived = TRUE;
            break;
        case FOLLOW_UP: // 解析Follow_Up中的t1
            // 1. Retrieve the Sync send timestamp.
            // 2. Calculate the Master to Slave delay
            // 3. Subtract this from Mean Path Delay to get "Offset from Master"
            // 4. Update the clock based on this value. Course correction.
            if (msgHeader.sequenceId == gPtpSlaveState.lastSyncSeqId)
            {
                gPtpSlaveState.syncOriginTimestamp.secondsField.msb =
                        flip16(*(UInteger16 *) (pPacket->dataBuffer + PTP_HEADER_OFFSET + 34 ));
                gPtpSlaveState.syncOriginTimestamp.secondsField.lsb =
                        flip32(*(UInteger32 *) (pPacket->dataBuffer + PTP_HEADER_OFFSET + 36));
                gPtpSlaveState.syncOriginTimestamp.nanosecondsField =
                        flip32(*(UInteger32 *) (pPacket->dataBuffer + PTP_HEADER_OFFSET + 40));

                while (gPtpSlaveState.syncOriginTimestamp.secondsField.lsb == 0x2000CE44)
                {
                    //no task
                }

                // converting the origin timestamp to internal time
                toInternalTime(&sendTime, &gPtpSlaveState.syncOriginTimestamp);

                // converting the sync receive timestamp to internal time
                toInternalTime(&recvTime, &gPtpSlaveState.syncRecvTimestamp);

                // Calculate Master to slave delay
                subTime(&gPtpSlaveState.delayMS, &recvTime, &sendTime);

                // Calculate offset from master
                subTime(&gPtpSlaveState.offsetFromMaster,
                        &gPtpSlaveState.delayMS,
                        &gPtpSlaveState.meanPathDelay);

                updateClock();

                // If sufficient sync/followups have been received, then issue a
                // delay request packet.
                if (!((gPtpSlaveState.lastSyncSeqId + 1) % 10))
                {
                    // We need to set a standard defined Multicast address : 01:1B:19:00:00:00
                    // as the Destination address in the ethernet frame and that is how the
                    // receiver will recognize it as a valid PTP over Ethernet packet.
                    i=0; *((uint32_t *)delayReqMsg + i) = 0x00191B01;
                    i++; *((uint32_t *)delayReqMsg + i)  = 0xF7880000;

                    // Reset the buffer next sync packet.
                    memset(delayReqMsg+8, 0, PACKET_LENGTH-8);
                    msgPackDelayReq((Octet *)delayReqMsg + 8, &gPtpSlaveState);

                    gPktDesc.bufferLength = PACKET_LENGTH;
                    gPktDesc.dataOffset = 0;
                    gPktDesc.dataBuffer = delayReqMsg;
                    gPktDesc.nextPacketDesc = 0;
                    gPktDesc.flags = ETHERNET_PKT_FLAG_TTSE |
                                    ETHERNET_PKT_FLAG_SOP |
                                    ETHERNET_PKT_FLAG_EOP |
                                    ETHERNET_PKT_FLAG_SA_INS |
                                    ETHERNET_PKT_FLAG_CRC_PAD_INS;
                    gPktDesc.pktChannel = ETHERNET_DMA_CHANNEL_NUM_0;
                    gPktDesc.pktLength = DELAY_REQ_LENGTH + 6 + 2;
                    gPktDesc.validLength = gPktDesc.pktLength;
                    gPktDesc.numPktFrags = 1;

                    Ethernet_sendPacket(emac_handle, &gPktDesc);
                    gPtpSlaveState.delayReqSeqId++;
                }
            }
            break;
        case DELAY_RESP: // 解析Delay_Resp中的t4
            // 1.Get the "Delay Request receive timestamp.
            // 2.Calculate the Slave to Master Delay.
            // 3.Calculate the Mean Path Delay and save it.
            if (gPtpSlaveState.delayRespReceived == TRUE)
            {
                gPtpSlaveState.delayRespReceived = FALSE;

                gPtpSlaveState.delayReqRecvTimestamp.secondsField.msb =
                    flip16(*(UInteger16 *) (pPacket->dataBuffer + PTP_HEADER_OFFSET + 34 ));
                gPtpSlaveState.delayReqRecvTimestamp.secondsField.lsb =
                    flip32(*(UInteger32 *) (pPacket->dataBuffer + PTP_HEADER_OFFSET + 36));
                gPtpSlaveState.delayReqRecvTimestamp.nanosecondsField =
                    flip32(*(UInteger32 *) (pPacket->dataBuffer + PTP_HEADER_OFFSET + 40));

                // Update delay and calculate Mean Path Delay
                toInternalTime(&sendTime, &gPtpSlaveState.delayReqSentTimestamp);
                toInternalTime(&recvTime, &gPtpSlaveState.delayReqRecvTimestamp);

                subTime(&gPtpSlaveState.delaySM, &recvTime, &sendTime);
                addTime(&gPtpSlaveState.meanPathDelay,
                        &gPtpSlaveState.delaySM,
                        &gPtpSlaveState.delayMS);

                div2Time(&gPtpSlaveState.meanPathDelay);
            }
            break;
        }
    }

    // Book-keeping to maintain number of callbacks received.
#ifdef ETHERNET_DEBUG
    Ethernet_numRxCallbackCustom++;
#endif
    return Ethernet_getPacketBufferCustom();
}

void sendMessage(Octet *msg, Enumeration4 msgType, void *ptpState, Ethernet_Pkt_Desc *pktDesc)
{
    uint32_t pktLen;

    // We can't trust if the supplied descriptor is clean. Hence reset it.
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

    // Reset the buffer
    memset(msg + 8, 0, PACKET_LENGTH - 8);

    switch(msgType)
    {
    case SYNC:      // Master发送sync
        pktDesc->flags |= ETHERNET_PKT_FLAG_TTSE;
        msgPackSync(msg + 8,ptpState);
        pktLen = SYNC_LENGTH;
        break;
    case FOLLOW_UP: // Master发送followUp
        msgPackFollowUp(msg + 8, ptpState);
        pktLen = FOLLOW_UP_LENGTH;
        ((PTPMasterState*)ptpState)->syncSeqId++;
        break;
    case DELAY_RESP: // Master收到DelayResp
        msgPackDelayResp(msg + 8, ptpState);
        pktLen = DELAY_RESP_LENGTH;
        break;
    case DELAY_REQ:  // Slave发送Delay_Req
        msgPackDelayReq(msg + 8, ptpState);
        pktLen = DELAY_REQ_LENGTH;
        ((PTPSlaveState*)ptpState)->delayReqSeqId++;
        break;
    default:
        return;
    }

    pktDesc->pktLength = pktLen + 6 + 2;
    pktDesc->validLength = pktDesc->pktLength;

    // 4.发送报文
    Ethernet_sendPacket(emac_handle, pktDesc);
}

// Unpack Header from IN buffer to msgTmpHeader field
void msgUnpackHeader(Octet * buf, MsgHeader * header)
{
    header->transportSpecific = (*(Nibble *) (buf + 0)) >> 4;
    header->messageType = (*(Enumeration4 *) (buf + 0)) & 0x0F;
    header->versionPTP = (*(UInteger4 *) (buf + 1)) & 0x0F;

    // force reserved bit to zero if not
    header->messageLength = flip16(*(UInteger16 *) (buf + 2));
    header->domainNumber = (*(UInteger8 *) (buf + 4));
    memcpy(header->flagField, (buf + 6), FLAG_FIELD_LENGTH);
    memcpy(&header->correctionfield.msb, (buf + 8), 4);
    memcpy(&header->correctionfield.lsb, (buf + 12), 4);
    header->correctionfield.msb = flip32(header->correctionfield.msb);
    header->correctionfield.lsb = flip32(header->correctionfield.lsb);
    memcpy(header->sourcePortIdentity.clockIdentity, (buf + 20),
           CLOCK_IDENTITY_LENGTH);

    header->sourcePortIdentity.portNumber =
        flip16(*(UInteger16 *) (buf + 28));

    header->sequenceId = flip16(*(UInteger16 *) (buf + 30));
    header->controlField = (*(UInteger8 *) (buf + 32));
    header->logMessageInterval = (*(Integer8 *) (buf + 33));
}

// Pack SYNC message into OUT buffer of ptpClock
void msgPackSync(Octet * buf, void *ptpState)
{
    msgPackHeader(buf, ptpState);

    *(char *)(buf + 0) = *(char *)(buf + 0) & 0xF0;
    *(char *)(buf + 0) = *(char *)(buf + 0) | 0x00; /* Table 19 */

    *(UInteger16 *) (buf + 2) = flip16(SYNC_LENGTH);
    *(UInteger16 *) (buf + 30) = flip16(((PTPMasterState*)ptpState)->syncSeqId);
    *(UInteger8 *) (buf + 32) = 0x00;
    *(Integer8 *) (buf + 33) = 0;  // We'll send sync every second

    if(!PTP_TWO_STEP)
    {
        // Sync message
        *(UInteger16 *) (buf + 34) =
                flip16(((PTPMasterState*)ptpState)->syncTimestamp.secondsField.msb);
        *(UInteger32 *) (buf + 36) =
                flip32(((PTPMasterState*)ptpState)->syncTimestamp.secondsField.lsb);
        *(UInteger32 *) (buf + 40) =
                flip32(((PTPMasterState*)ptpState)->syncTimestamp.nanosecondsField);
    }
}

// pack Follow_up message into OUT buffer of ptpClock
void msgPackFollowUp(Octet * buf, void *ptpState)
{
    msgPackHeader(buf, ptpState);

    // changes in header
    *(char *)(buf + 0) = *(char *)(buf + 0) & 0xF0;

    // RAZ messageType
    *(char *)(buf + 0) = *(char *)(buf + 0) | 0x08; /* Table 19 */

    // messageLength
    *(UInteger16 *) (buf + 2) = flip16(FOLLOW_UP_LENGTH);

    // Sequence Id
    *(UInteger16 *) (buf + 30) = flip16(((PTPMasterState*)ptpState)->syncSeqId);

    // controlField
    *(UInteger8 *) (buf + 32) = 0x02;   /* Table 23 */

    // logMessageInterval
    *(Integer8 *) (buf + 33) = 0;   // we're sending sync every one second.

    // Follow_up message
    *(UInteger16 *) (buf + 34) =
            flip16(((PTPMasterState*)ptpState)->syncTimestamp.secondsField.msb);
    *(UInteger32 *) (buf + 36) =
            flip32(((PTPMasterState*)ptpState)->syncTimestamp.secondsField.lsb);
    *(UInteger32 *) (buf + 40) =
            flip32(((PTPMasterState*)ptpState)->syncTimestamp.nanosecondsField);
}

// Pack header message into OUT buffer of ptpClock
void msgPackHeader(Octet * buf, void *ptpState)
{
    Nibble transport = 0x80;

    // (spec annex D)
    *(UInteger8 *) (buf + 0) = transport;

    // PTPv2
    *(UInteger4 *) (buf + 1) = 0x2;

    // Default domain number is 0.
    *(UInteger8 *) (buf + 4) = 0;

    if (PTP_TWO_STEP)
        *(UInteger8 *) (buf + 6) = PTP_TWO_STEP;

    // correctionField
    memset((buf + 8), 0, 8);

    if (g_ptpMode == 0)  // Master
    {
        // sourcePortIdentity first 8 octets
        memcpy((buf + 20), ((PTPMasterState*)ptpState)->portIdentity.clockIdentity,
               CLOCK_IDENTITY_LENGTH);

        // sourcePortIdentity last 2 octets
        *(UInteger16 *) (buf + 28) =
                flip16(((PTPMasterState*)ptpState)->portIdentity.portNumber);
    }
    else  // Slave
    {
        memcpy((buf + 20), ((PTPSlaveState*)ptpState)->portIdentity.clockIdentity,
               CLOCK_IDENTITY_LENGTH);
        *(UInteger16 *) (buf + 28) =
                flip16(((PTPSlaveState*)ptpState)->portIdentity.portNumber);
    }

    // Default value(spec Table 24)
    *(UInteger8 *) (buf + 33) = 0x7F;
}

// pack delayResp message into OUT buffer of ptpClock
void msgPackDelayResp(Octet * buf, void *ptpState)
{
    msgPackHeader(buf, ptpState);

    // changes in header
    // Transport | messageType
    *(char *)(buf + 0) = *(char *)(buf + 0) & 0xF0;

    // RAZ messageType
    *(char *)(buf + 0) = *(char *)(buf + 0) | 0x09; /* Table 19 */

    // messageLength
    *(UInteger16 *) (buf + 2) = flip16(DELAY_RESP_LENGTH);

    // domain Number
    *(UInteger8 *) (buf + 4) = ((PTPMasterState*)ptpState)->delayReqHeader.domainNumber;

    // Sequence Id
    *(UInteger16 *) (buf + 30) =
            flip16(((PTPMasterState*)ptpState)->delayReqHeader.sequenceId);

    // controlField
    *(UInteger8 *) (buf + 32) = 0x03; /* Table 23 */

    // logMessageInterval
    *(Integer8 *) (buf + 33) = 0; /* Table 24 */

    *(UInteger16 *) (buf + 34) =
        flip16(((PTPMasterState*)ptpState)->delayReqRecvTimestamp.secondsField.msb);

    *(UInteger32 *) (buf + 36) =
        flip32(((PTPMasterState*)ptpState)->delayReqRecvTimestamp.secondsField.lsb);

    *(UInteger32 *) (buf + 40) =
       flip32(((PTPMasterState*)ptpState)->delayReqRecvTimestamp.nanosecondsField);

    memcpy((buf + 44),
           ((PTPMasterState*)ptpState)->delayReqHeader.sourcePortIdentity.clockIdentity,
           CLOCK_IDENTITY_LENGTH);

    *(UInteger16 *) (buf + 52) =
        flip16(((PTPMasterState*)ptpState)->delayReqHeader.sourcePortIdentity.portNumber);
}

// pack delayReq message into OUT buffer of ptpClock
void msgPackDelayReq(Octet * buf, void *ptpState)
{
    msgPackHeader(buf, ptpState);

    // changes in header
    *(char *)(buf + 0) = *(char *)(buf + 0) & 0xF0;

    // RAZ messageType
    *(char *)(buf + 0) = *(char *)(buf + 0) | 0x01; /* Table 19 */

    // messageLength
    *(UInteger16 *) (buf + 2) = flip16(DELAY_REQ_LENGTH);

    // Sequence Id
    *(UInteger16 *) (buf + 30) = flip16(((PTPSlaveState*)ptpState)->delayReqSeqId);
    // controlField
    *(UInteger8 *) (buf + 32) = 0x01; /* Table 23 */
    // logMessageInterval
    *(Integer8 *) (buf + 33) = 0x7F; /* Table 24 */
}

void toInternalTime(TimeInternal * internal, Timestamp * external)
{
    // Program will not run after 2038
    if (external->secondsField.lsb < INT_MAX) {
        internal->seconds = external->secondsField.lsb;
        internal->nanoseconds = external->nanosecondsField;
    }
    else
    {
        //
        // Error condition.
        // Clock servo canno't be executed :
        // seconds field is higher than signed integer (32bits))
        //
        return;
    }
}

void getTime(TimeInternal *time)
{
    // Fetch the current system time.
    Ethernet_getSysTimePTP(EMAC_BASE, (uint32_t *)&time->seconds,
                                (uint32_t *)&time->nanoseconds);
}

void setTime(TimeInternal *time)
{
    // Set the new system time.
    Ethernet_setSysTimePTP(EMAC_BASE, (uint32_t)time->seconds,
                            (uint32_t)time->nanoseconds);
}

void updateClock(void)
{
    TimeInternal timeTmp;

    // Update the clock if either
    if((gPtpSlaveState.offsetFromMaster.seconds > 0) ||
       (abs(gPtpSlaveState.offsetFromMaster.nanoseconds) >
        PTP_OFM_NANOSECONDS_CUTOFF))
    {
        getTime(&timeTmp);
        subTime(&timeTmp, &timeTmp, &gPtpSlaveState.offsetFromMaster);
        setTime(&timeTmp);
        gPtpSlaveState.clockUpdateCount++;
    }
}

void subTime(TimeInternal * r, const TimeInternal * x, const TimeInternal * y)
{
    r->seconds = x->seconds - y->seconds;
    r->nanoseconds = x->nanoseconds - y->nanoseconds;

    normalizeTime(r);
}

void addTime(TimeInternal * r, const TimeInternal * x, const TimeInternal * y)
{
    r->seconds = x->seconds + y->seconds;
    r->nanoseconds = x->nanoseconds + y->nanoseconds;

    normalizeTime(r);
}

void div2Time(TimeInternal *r)
{
    r->nanoseconds += r->seconds % 2 * ONE_BILLION;
    r->seconds /= 2;
    r->nanoseconds /= 2;

    normalizeTime(r);
}

void normalizeTime(TimeInternal * r)
{
    r->seconds += r->nanoseconds / ONE_BILLION;
    r->nanoseconds -= r->nanoseconds / ONE_BILLION * ONE_BILLION;

    if (r->seconds > 0 && r->nanoseconds < 0) {
        r->seconds -= 1;
        r->nanoseconds += ONE_BILLION;
    } else if (r->seconds < 0 && r->nanoseconds > 0) {
        r->seconds += 1;
        r->nanoseconds -= ONE_BILLION;
    }
}
