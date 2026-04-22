/*
 * Eth_mii.c
 *
 *  Created on: 2025锟斤拷6锟斤拷26锟斤拷
 *      Author: guowei
 */

#include "driverlib_cm/ethernet.h"
#include "lwipopts.h"
#include "bsp.h"
#include "utils/lwiplib.h"
#include "lwipopts.h"
#include "eth_common.h"

#define ETHERNET_NO_OF_RX_PACKETS   2U
#define ETHERNET_MAX_PACKET_LENGTH 1538U

#define NUM_PACKET_DESC_RX_APPLICATION PBUF_POOL_SIZE //8 - same as PBUF_POOL_SIZE

#define MAKE_IP_ADDRESS(a3,a2,a1,a0) (((a3<<24) & 0xFF000000) | ((a2<<16) & 0x00FF0000) | ((a1<<8)  & 0x0000FF00) | (a0 & 0x000000FF) )

Ethernet_Handle emac_handle;
Ethernet_InitConfig *pInitCfg;

PTPMasterState gPtpMasterState = {0};
PTPSlaveState gPtpSlaveState = {0};

uint32_t Ethernet_numRxCallbackCustom = 0;
uint32_t releaseTxCount = 0;
uint32_t genericISRCustomcount = 0;
uint32_t genericISRCustomRBUcount = 0;
uint32_t genericISRCustomROVcount = 0;
uint32_t genericISRCustomRIcount = 0;

Ethernet_Pkt_Desc  pktDescriptorRXCustom[NUM_PACKET_DESC_RX_APPLICATION];

uint8_t Ethernet_rxBuffer[ETHERNET_NO_OF_RX_PACKETS *
                          ETHERNET_MAX_PACKET_LENGTH];

uint32_t sendPacketFailedCount = 0;

uint8_t g_ptpMode;
uint8_t delayReqMsg[PACKET_LENGTH] = {0};
uint8_t gMsgBuf[PACKET_LENGTH] = {0};
Ethernet_Pkt_Desc gPktDesc;


#define ETHERNET_DEBUG
#ifdef ETHERNET_DEBUG
volatile uint32_t debug_tx_callback_cnt = 0;      // 发送回调次数
volatile uint32_t debug_delayreq_tx_cnt = 0;      // DelayReq发送次数
volatile uint32_t debug_delayresp_rx_cnt = 0;     // DelayResp接收次数
volatile uint8_t  debug_last_msg_type = 0;        // 最后发送的消息类型
volatile uint16_t debug_delayreq_seqid = 0;       // DelayReq的sequenceId

volatile TimeInternal debug_t1;                   // 主机发送Sync时间
volatile TimeInternal debug_t2;                   // 从机接收Sync时间
volatile TimeInternal debug_t3;                   // 从机发送DelayReq时间
volatile TimeInternal debug_t4;                   // 主机接收DelayReq时间
volatile TimeInternal debug_delayMS;              // t2 - t1
volatile TimeInternal debug_delaySM;              // t4 - t3;
volatile TimeInternal debug_meanPathDelay;        // 路径延迟时间
#endif


//uint32_t IPAddr =  0xC0A80004; // 0xC0A80004; //192.168.0.4
//uint32_t NetMask = 0xFFFFFF00;
//uint32_t GWAddr = 0x00000000;


extern uint32_t Ethernet_numGetPacketBufferCallback;
extern Ethernet_Device Ethernet_device_struct;


extern Ethernet_Pkt_Desc*
lwIPEthernetIntHandler(Ethernet_Pkt_Desc *pPacket);

//
// Function prototypes used in this example
//
static void msgPackHeader(Octet * buf, void *ptpState);
static void msgPackSync(Octet * buf, void *ptpState);
static void msgPackFollowUp(Octet * buf, void *ptpState);
static void msgPackDelayResp(Octet * buf, void *ptpState);
static void msgUnpackHeader(Octet * buf, MsgHeader * header);
static void msgPackDelayReq(Octet * buf, void *ptpState);
static void sendMessage(Octet *msg, uint32_t messageType, void *ptpState, Ethernet_Pkt_Desc *pktDesc);

static void fromInternalTime(TimeInternal * internal, Timestamp * external);
static void normalizeTime(TimeInternal * r);
static void subTime(TimeInternal * r, const TimeInternal * x, const TimeInternal * y);
static void addTime(TimeInternal * r, const TimeInternal * x, const TimeInternal * y);
static void div2Time(TimeInternal *r);
static void toInternalTime(TimeInternal * internal, Timestamp * external);
static void getTime(TimeInternal *time);
static void setTime(TimeInternal *time);
static void updateClock(void);


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
                                      &Ethernet_device_struct.rxBuffer [ \
               (ETHERNET_MAX_PACKET_LENGTH*Ethernet_device_struct.rxBuffIndex)];

    //
    // Update the receive buffer pool index.
    //
    Ethernet_device_struct.rxBuffIndex += 1U;
    Ethernet_device_struct.rxBuffIndex  = \
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
Ethernet_Pkt_Desc* Ethernet_receivePacketCallbackCustom(
        Ethernet_Handle handleApplication,
        Ethernet_Pkt_Desc *pPacket)
{

    Ethernet_Pkt_Desc* temp_eth_pkt;
    //
    // Book-keeping to maintain number of callbacks received.
    //
#ifdef ETHERNET_DEBUG
    Ethernet_numRxCallbackCustom++;
#endif

      Ethernet_disableRxDMAReception(EMAC_BASE,0);

    //
    // This is a placeholder for Application specific handling
    // We are replenishing the buffer received with another buffer
    //
    // return lwIPEthernetIntHandler(pPacket);

      temp_eth_pkt=lwIPEthernetIntHandler(pPacket);


      Ethernet_enableRxDMAReception(EMAC_BASE,0);

      return temp_eth_pkt;
}

void Ethernet_releaseTxPacketBufferCustom(
        Ethernet_Handle handleApplication,
        Ethernet_Pkt_Desc *pPacket)
{
    //
    // Once the packet is sent, reuse the packet memory to avoid
    // memory leaks. Call this interrupt handler function which will take care
    // of freeing the memory used by the packet descriptor.
    //
    lwIPEthernetIntHandler(pPacket);

    //
    // Increment the book-keeping counter.
    //
#ifdef ETHERNET_DEBUG
    releaseTxCount++;
#endif
}

Ethernet_Pkt_Desc *Ethernet_performPopOnPacketQueueCustom(
            Ethernet_PKT_Queue_T *pktQueuePtr)
{
    Ethernet_Pkt_Desc *pktDescHdrPtr;

    pktDescHdrPtr = pktQueuePtr->head;

    if(0U != pktDescHdrPtr)
    {
        pktQueuePtr->head = pktDescHdrPtr->nextPacketDesc;
        pktQueuePtr->count--;
    }

    return(pktDescHdrPtr);
}
void Ethernet_performPushOnPacketQueueCustom(
        Ethernet_PKT_Queue_T *pktQueuePtr,
        Ethernet_Pkt_Desc *pktDescHdrPtr)
{
    pktDescHdrPtr->nextPacketDesc = 0U;

    if(0U == pktQueuePtr->head)
    {
        //
        // Queue is empty - Initialize it with this one packet
        //
        pktQueuePtr->head = pktDescHdrPtr;
        pktQueuePtr->tail = pktDescHdrPtr;
    }
    else
    {
        //
        // Queue is not empty - Push onto END
        //
        pktQueuePtr->tail->nextPacketDesc = pktDescHdrPtr;
        pktQueuePtr->tail        = pktDescHdrPtr;
    }
    pktQueuePtr->count++;
}
void Ethernet_setMACConfigurationCustom(uint32_t base, uint32_t flags)
{
    HWREG(base + ETHERNET_O_MAC_CONFIGURATION) |= flags;
}
void Ethernet_clearMACConfigurationCustom(uint32_t base, uint32_t flags)
{
    HWREG(base + ETHERNET_O_MAC_CONFIGURATION) &= ~flags;

}

// PTP Release TX PackerBuffer
void Ethernet_releaseTxPacketBufferPTP(
        Ethernet_Handle handleApplication,
        Ethernet_Pkt_Desc *pPacket)
{
    uint8_t *data;
    uint8_t msgType;
    if (pPacket == NULL)
        return;

    if (g_ptpMode == 0)  // Master mode
    {
        // capture the timestamp for the SYNC packet only
        if (gPtpMasterState.sendingDelayResp == TRUE)
        {
            gPtpMasterState.sendingDelayResp = FALSE;
        }
        else if (gPtpMasterState.syncTimestampAvailable == FALSE)
        {
            gPtpMasterState.syncTimestamp.nanosecondsField = pPacket->timeStampLow;
            gPtpMasterState.syncTimestamp.secondsField.lsb = pPacket->timeStampHigh;
            gPtpMasterState.syncTimestamp.secondsField.msb = 0;

            gPtpMasterState.syncTimestampAvailable = TRUE;
        }
    }
    else  // Slave mode
    {
        uint8_t *data = pPacket->dataBuffer + pPacket->dataOffset;
        uint8_t msgType = data[8] & 0x0F;

        if (msgType == DELAY_REQ)  // 0x01
        {
            gPtpSlaveState.delayReqSentTimestamp.nanosecondsField = pPacket->timeStampLow;
            gPtpSlaveState.delayReqSentTimestamp.secondsField.lsb = pPacket->timeStampHigh;
            gPtpSlaveState.delayReqSentTimestamp.secondsField.msb = 0;

            gPtpSlaveState.waitingForDelayResp = TRUE;
        }

#ifdef ETHERNET_DEBUG
    debug_tx_callback_cnt++;
    if (msgType == DELAY_REQ)  // 0x01
    {
        debug_delayreq_tx_cnt++;
        debug_delayreq_seqid = gPtpSlaveState.delayReqSeqId;
    }
    debug_last_msg_type = msgType;
#endif
    }

    // Increment the book-keeping counter.
#ifdef ETHERNET_DEBUG
    releaseTxCount++;
#endif
}

// PTP Receive message callback func
Ethernet_Pkt_Desc* Ethernet_receivePacketCallbackPTP(
        Ethernet_Handle handleApplication,
        Ethernet_Pkt_Desc *pPacket)
{
    uint8_t *data = pPacket->dataBuffer;

    if (pPacket == NULL)
    {
        return;
    }

    if (g_ptpMode == 0)  // Master mode
    {
        msgUnpackHeader((Octet*)(data + PTP_HEADER_OFFSET), &gPtpMasterState.delayReqHeader);
        switch(gPtpMasterState.delayReqHeader.messageType)
        {
        case DELAY_REQ:
            gPtpMasterState.delayReqRecvTimestamp.nanosecondsField =
                    pPacket->timeStampLow;
            gPtpMasterState.delayReqRecvTimestamp.secondsField.lsb =
                    pPacket->timeStampHigh;
            gPtpMasterState.delayReqRecvTimestamp.secondsField.msb = 0;

            gPtpMasterState.sendingDelayResp = TRUE;
            sendMessage((Octet *)gMsgBuf, DELAY_RESP, &gPtpMasterState, &gPktDesc);
            break;
        default:
            break;
        }
    }
    else  // Slave mode
    {
        MsgHeader header;
        TimeInternal recvTime;
        TimeInternal sendTime;
        uint32_t i;

        msgUnpackHeader((Octet*)(data + PTP_HEADER_OFFSET), &header);

        switch(header.messageType)
        {
        case SYNC:
            gPtpSlaveState.syncRecvTimestamp.nanosecondsField =
                    pPacket->timeStampLow;
            gPtpSlaveState.syncRecvTimestamp.secondsField.lsb =
                    pPacket->timeStampHigh;
            gPtpSlaveState.syncRecvTimestamp.secondsField.msb = 0;

            gPtpSlaveState.lastSyncSeqId = header.sequenceId;
            break;
        case FOLLOW_UP:  // Retrieve Sync origin timestamp t1
            if (header.sequenceId == gPtpSlaveState.lastSyncSeqId)
            {
                gPtpSlaveState.syncOriginTimestamp.secondsField.msb =
                    flip16(*(UInteger16 *) (pPacket->dataBuffer + PTP_HEADER_OFFSET + 34 ));
                gPtpSlaveState.syncOriginTimestamp.secondsField.lsb =
                    flip32(*(UInteger32 *) (pPacket->dataBuffer + PTP_HEADER_OFFSET + 36));
                gPtpSlaveState.syncOriginTimestamp.nanosecondsField =
                    flip32(*(UInteger32 *) (pPacket->dataBuffer + PTP_HEADER_OFFSET + 40));

                //
                // converting the origin timestamp to internal time.
                //
                toInternalTime(&sendTime, &gPtpSlaveState.syncOriginTimestamp);
#ifdef ETHERNET_DEBUG
                debug_t1.seconds     = sendTime.seconds;
                debug_t1.nanoseconds = sendTime.nanoseconds;
#endif

                //
                // converting the sync receive timestamp to internal time (t2)
                //
                toInternalTime(&recvTime, &gPtpSlaveState.syncRecvTimestamp);
#ifdef ETHERNET_DEBUG
                debug_t2.seconds     = recvTime.seconds;
                debug_t2.nanoseconds = recvTime.nanoseconds;
#endif

                //
                // Calculate Master to slave delay. (delayMS=t2 - t1)
                //
                subTime(&gPtpSlaveState.delayMS, &recvTime, &sendTime);
#ifdef ETHERNET_DEBUG
                debug_delayMS.seconds     = gPtpSlaveState.delayMS.seconds;
                debug_delayMS.nanoseconds = gPtpSlaveState.delayMS.nanoseconds;
#endif

                //
                // Calculate offset from master only after meanPathDelay is valid.
                //
                if (gPtpSlaveState.meanPathDelayValid == TRUE)
                {
                    subTime(&gPtpSlaveState.offsetFromMaster,
                            &gPtpSlaveState.delayMS,
                            &gPtpSlaveState.meanPathDelay);

                    updateClock();
                }

                //
                // Send a DelayReq every 10 Syncs.
                //
                if(!((gPtpSlaveState.lastSyncSeqId + 1) % 10))
                {
                    i=0; *((uint32_t *)delayReqMsg + i) = 0x00191B01;
                    i++; *((uint32_t *)delayReqMsg + i)  = 0xF7880000;

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

                    Ethernet_sendPacket(emac_handle,&gPktDesc);

                    gPtpSlaveState.delayReqSeqId++;
                }
            }
            break;

        case DELAY_RESP:
            //
            // Match only the expected DelayResp for the latest DelayReq.
            //
            if(gPtpSlaveState.waitingForDelayResp == TRUE &&
              (header.sequenceId == (uint16_t)(gPtpSlaveState.delayReqSeqId - 1U)))
            {
                gPtpSlaveState.waitingForDelayResp = FALSE;
#ifdef ETHERNET_DEBUG
                debug_delayresp_rx_cnt++;
#endif

                gPtpSlaveState.delayReqRecvTimestamp.secondsField.msb =
                    flip16(*(UInteger16 *) (pPacket->dataBuffer + PTP_HEADER_OFFSET + 34 ));
                gPtpSlaveState.delayReqRecvTimestamp.secondsField.lsb =
                    flip32(*(UInteger32 *) (pPacket->dataBuffer + PTP_HEADER_OFFSET + 36));
                gPtpSlaveState.delayReqRecvTimestamp.nanosecondsField =
                    flip32(*(UInteger32 *) (pPacket->dataBuffer + PTP_HEADER_OFFSET + 40));

                //
                // t3 = DelayReq transmit timestamp
                //
                toInternalTime(&sendTime, &gPtpSlaveState.delayReqSentTimestamp);
#ifdef ETHERNET_DEBUG
                debug_t3.seconds     = sendTime.seconds;
                debug_t3.nanoseconds = sendTime.nanoseconds;
#endif

                //
                // t4 = Master receive timestamp carried in DelayResp
                //
                toInternalTime(&recvTime, &gPtpSlaveState.delayReqRecvTimestamp);
#ifdef ETHERNET_DEBUG
                debug_t4.seconds     = recvTime.seconds;
                debug_t4.nanoseconds = recvTime.nanoseconds;
#endif

                //
                // delaySM = t4 - t3
                //
                subTime(&gPtpSlaveState.delaySM, &recvTime, &sendTime);
#ifdef ETHERNET_DEBUG
                debug_delaySM.seconds     = gPtpSlaveState.delaySM.seconds;
                debug_delaySM.nanoseconds = gPtpSlaveState.delaySM.nanoseconds;
#endif

                //
                // meanPathDelay = (delayMS + delaySM) / 2
                //
                addTime(&gPtpSlaveState.meanPathDelay,
                        &gPtpSlaveState.delaySM,
                        &gPtpSlaveState.delayMS);

                div2Time(&gPtpSlaveState.meanPathDelay);

                //
                // Mean path delay must not be negative.
                // Clamp it to zero if numerical error or timestamp skew makes it
                // slightly negative.
                //
                if ((gPtpSlaveState.meanPathDelay.seconds < 0) ||
                     ((gPtpSlaveState.meanPathDelay.seconds == 0) &&
                      (gPtpSlaveState.meanPathDelay.nanoseconds < 0)))
                {
                    gPtpSlaveState.meanPathDelay.seconds = 0;
                    gPtpSlaveState.meanPathDelay.nanoseconds = 0;
                }

#ifdef ETHERNET_DEBUG
                debug_meanPathDelay.seconds     = gPtpSlaveState.meanPathDelay.seconds;
                debug_meanPathDelay.nanoseconds = gPtpSlaveState.meanPathDelay.nanoseconds;
#endif

                // 标记meanPathDelayValid已有效
                gPtpSlaveState.meanPathDelayValid = TRUE;
            }
            break;

        default:
            break;
        }
    }

    //
    // Book-keeping to maintain number of callbacks received.
    //
#ifdef ETHERNET_DEBUG
    Ethernet_numRxCallbackCustom++;
#endif
    return Ethernet_getPacketBufferCustom();
}


void Ethernet_ReleaseTxPacketBufferCombine(
        Ethernet_Handle handleApplication,
        Ethernet_Pkt_Desc *pPacket)
{
    if (pPacket == NULL)
        return;

    // Lwip release tx buffer
    Ethernet_releaseTxPacketBufferCustom(handleApplication, pPacket);

    // PTP Capture sending massage
    Ethernet_releaseTxPacketBufferPTP(handleApplication, pPacket);
}

Ethernet_Pkt_Desc* Ethernet_receivePacketCallbackCombine(
        Ethernet_Handle handleApplication,
        Ethernet_Pkt_Desc *pPacket)
{
    if (pPacket == NULL)
        return NULL;

    // Lwip receive callback
    pPacket = Ethernet_receivePacketCallbackCustom(handleApplication, pPacket);

    // PTP receive callback
    pPacket = Ethernet_receivePacketCallbackPTP(handleApplication, pPacket);
    return pPacket;
}


interrupt void Ethernet_genericISRCustom(void)
{
    genericISRCustomcount++;
    Ethernet_RxChDesc *rxChan;
    Ethernet_TxChDesc *txChan;
//    Ethernet_HW_descriptor    *descPtr;
//    Ethernet_HW_descriptor    *tailPtr;
    uint16_t i=0;
    Ethernet_clearMACConfigurationCustom(Ethernet_device_struct.baseAddresses.enet_base,ETHERNET_MAC_CONFIGURATION_RE);
    Ethernet_clearMACConfigurationCustom(Ethernet_device_struct.baseAddresses.enet_base,ETHERNET_MAC_CONFIGURATION_TE);
    for(i = 0U;i < Ethernet_device_struct.initConfig.numChannels;i++)
     {
         Ethernet_disableRxDMAReception(
               Ethernet_device_struct.baseAddresses.enet_base,
               i);
     }
    if(((ETHERNET_DMA_CH0_STATUS_AIS |
                         ETHERNET_DMA_CH0_STATUS_RBU) ==
                       (HWREG(Ethernet_device_struct.baseAddresses.enet_base +
                              ETHERNET_O_DMA_CH0_STATUS) &
                              (uint32_t)(ETHERNET_DMA_CH0_STATUS_AIS |
                                         ETHERNET_DMA_CH0_STATUS_RBU))) ||
          (ETHERNET_MTL_Q0_INTERRUPT_CONTROL_STATUS_RXOVFIS) ==
                                   (HWREG(Ethernet_device_struct.baseAddresses.enet_base +
                                          ETHERNET_O_MTL_Q0_INTERRUPT_CONTROL_STATUS) &
                                          (uint32_t)(ETHERNET_MTL_Q0_INTERRUPT_CONTROL_STATUS_RXOVFIS
                                                     )))
      {
          if((ETHERNET_DMA_CH0_STATUS_AIS |
                             ETHERNET_DMA_CH0_STATUS_RBU) ==
                           (HWREG(Ethernet_device_struct.baseAddresses.enet_base +
                                  ETHERNET_O_DMA_CH0_STATUS) &
                                  (uint32_t)(ETHERNET_DMA_CH0_STATUS_AIS |
                                             ETHERNET_DMA_CH0_STATUS_RBU)))
          {
          genericISRCustomRBUcount++;
          }
          if((ETHERNET_MTL_Q0_INTERRUPT_CONTROL_STATUS_RXOVFIS) ==
                  (HWREG(Ethernet_device_struct.baseAddresses.enet_base +
                         ETHERNET_O_MTL_Q0_INTERRUPT_CONTROL_STATUS) &
                         (uint32_t)(ETHERNET_MTL_Q0_INTERRUPT_CONTROL_STATUS_RXOVFIS
                                    )))
          {
              genericISRCustomROVcount++;
              Ethernet_enableMTLInterrupt(Ethernet_device_struct.baseAddresses.enet_base,0,
                                          ETHERNET_MTL_Q0_INTERRUPT_CONTROL_STATUS_RXOVFIS);
          }

        /*
             * Clear the AIS and RBU status bit. These MUST be
             * cleared together!
             */
            Ethernet_clearDMAChannelInterrupt(
                    Ethernet_device_struct.baseAddresses.enet_base,
                    ETHERNET_DMA_CHANNEL_NUM_0,
                    ETHERNET_DMA_CH0_STATUS_AIS |
                    ETHERNET_DMA_CH0_STATUS_RBU);

            /*
           *Recover from Receive Buffer Unavailable (and hung DMA)
         *
         * All descriptor buffers are owned by the application, and
         * in result the DMA cannot transfer incoming frames to the
         * buffers (RBU condition). DMA has also entered suspend
         * mode at this point, too.
         *
         * Drain the RX queues
         */

            /* Upon RBU error, discard all previously received packets */
            if(Ethernet_device_struct.initConfig.pfcbDeletePackets != NULL)
                (*Ethernet_device_struct.initConfig.pfcbDeletePackets)();

            rxChan =
               &Ethernet_device_struct.dmaObj.rxDma[ETHERNET_DMA_CHANNEL_NUM_0];
            txChan=
               &Ethernet_device_struct.dmaObj.txDma[ETHERNET_DMA_CHANNEL_NUM_0];

    /*
     * Need to disable multiple interrupts, so protect the code to do so within
     * a global disable block (to prevent getting interrupted in between)
     */

            if(NULL!= Ethernet_device_struct.ptrPlatformInterruptDisable)
            {
                (*Ethernet_device_struct.ptrPlatformInterruptDisable)(
                    Ethernet_device_struct.interruptNum[
                        ETHERNET_RX_INTR_CH0 + rxChan->chInfo->chNum]);

                (*Ethernet_device_struct.ptrPlatformInterruptDisable)(
                    Ethernet_device_struct.interruptNum[
                        ETHERNET_GENERIC_INTERRUPT]);
            }
            /* verify we have full capacity in the descriptor queue */
            if(rxChan->descQueue.count < rxChan->descMax) {
              /* The queue is not at full capacity due to OOM errors.
              Try to fill it again */
                Ethernet_addPacketsIntoRxQueue(rxChan);
            }
            Ethernet_initRxChannel(
                    &Ethernet_device_struct.initConfig.chInfo[ETHERNET_CH_DIR_RX][0]);

            Ethernet_writeRxDescTailPointer(
                Ethernet_device_struct.baseAddresses.enet_base,
                0,
                (&Ethernet_device_struct.rxDesc[
                 ((uint32_t)ETHERNET_DESCRIPTORS_NUM_RX_PER_CHANNEL) *
                  (0 + (uint32_t)1U)]));

            if(NULL!= Ethernet_device_struct.ptrPlatformInterruptEnable)
            {
                (*Ethernet_device_struct.ptrPlatformInterruptEnable)(
                    Ethernet_device_struct.interruptNum[
                        ETHERNET_RX_INTR_CH0 + rxChan->chInfo->chNum]);
                (*Ethernet_device_struct.ptrPlatformInterruptEnable)(
                    Ethernet_device_struct.interruptNum[
                        ETHERNET_GENERIC_INTERRUPT]);
            }


    }
    if(0U != (HWREG(Ethernet_device_struct.baseAddresses.enet_base +
                                 ETHERNET_O_DMA_CH0_STATUS) &
                           (uint32_t) ETHERNET_DMA_CH0_STATUS_RI))
    {
        genericISRCustomRIcount++;
        Ethernet_clearDMAChannelInterrupt(
                        Ethernet_device_struct.baseAddresses.enet_base,
                        ETHERNET_DMA_CHANNEL_NUM_0,
                        ETHERNET_DMA_CH0_STATUS_NIS | ETHERNET_DMA_CH0_STATUS_RI);
    }

    for(i = 0U;i < Ethernet_device_struct.initConfig.numChannels;i++)
     {
         Ethernet_enableRxDMAReception(
               Ethernet_device_struct.baseAddresses.enet_base,
               i);
     }
    Ethernet_setMACConfigurationCustom(Ethernet_device_struct.baseAddresses.enet_base,ETHERNET_MAC_CONFIGURATION_RE);
    Ethernet_setMACConfigurationCustom(Ethernet_device_struct.baseAddresses.enet_base,ETHERNET_MAC_CONFIGURATION_TE);
}

void Ethernet_init(const unsigned char *mac)
{
    Ethernet_InitInterfaceConfig initInterfaceConfig;
    uint32_t macLower;
    uint32_t macHigher;
    uint8_t *temp;

    initInterfaceConfig.ssbase = EMAC_SS_BASE;
    initInterfaceConfig.enet_base = EMAC_BASE;
    initInterfaceConfig.phyMode = ETHERNET_SS_PHY_INTF_SEL_MII;

    //
    // Assign SoC specific functions for Enabling,Disabling interrupts
    // and for enabling the Peripheral at system level
    //
    initInterfaceConfig.ptrPlatformInterruptDisable = &Platform_disableInterrupt;
    initInterfaceConfig.ptrPlatformInterruptEnable  = &Platform_enableInterrupt;
    initInterfaceConfig.ptrPlatformPeripheralEnable = &Platform_enablePeripheral;
    initInterfaceConfig.ptrPlatformPeripheralReset  = &Platform_resetPeripheral;

    //
    // Assign the peripheral number at the SoC
    //
    initInterfaceConfig.peripheralNum = SYSCTL_PERIPH_CLK_ENET;

    //
    // Assign the default SoC specific interrupt numbers of Ethernet interrupts
    //
    initInterfaceConfig.interruptNum[0] = INT_EMAC;
    initInterfaceConfig.interruptNum[1] = INT_EMAC_TX0;
    initInterfaceConfig.interruptNum[2] = INT_EMAC_TX1;
    initInterfaceConfig.interruptNum[3] = INT_EMAC_RX0;
    initInterfaceConfig.interruptNum[4] = INT_EMAC_RX1;

    pInitCfg = Ethernet_initInterface(initInterfaceConfig);

    Ethernet_getInitConfig(pInitCfg);
    pInitCfg->dmaMode.InterruptMode = ETHERNET_DMA_MODE_INTM_MODE2;

    //
    // Assign the callbacks for Getting packet buffer when needed
    // Releasing the TxPacketBuffer on Transmit interrupt callbacks
    // Receive packet callback on Receive packet completion interrupt
    //
    pInitCfg->pfcbRxPacket = &Ethernet_receivePacketCallbackCombine;
    pInitCfg->pfcbGetPacket = &Ethernet_getPacketBuffer;    //custom
    pInitCfg->pfcbFreePacket = &Ethernet_ReleaseTxPacketBufferCombine;

    //
    //Assign the Buffer to be used by the Low level driver for receiving
    //Packets. This should be accessible by the Ethernet DMA
    //
    pInitCfg->rxBuffer = Ethernet_rxBuffer;

    //
    // The Application handle is not used by this application
    // Hence using a dummy value of 1
    //
    Ethernet_getHandle((Ethernet_Handle)1, pInitCfg , &emac_handle);

    //
    // Disable transmit buffer unavailable and normal interrupt which
    // are enabled by default in Ethernet_getHandle.
    //
    Ethernet_disableDmaInterrupt(Ethernet_device_struct.baseAddresses.enet_base,
                                 0, (ETHERNET_DMA_CH0_INTERRUPT_ENABLE_TBUE |
                                     ETHERNET_DMA_CH0_INTERRUPT_ENABLE_NIE));

    //
    // Enable the MTL interrupt to service the receive FIFO overflow
    // condition in the Ethernet module.
    //
    Ethernet_enableMTLInterrupt(Ethernet_device_struct.baseAddresses.enet_base,0,
                                ETHERNET_MTL_Q0_INTERRUPT_CONTROL_STATUS_RXOIE);

    //
    // Disable the MAC Management counter interrupts as they are not used
    // in this application.
    //
    HWREG(Ethernet_device_struct.baseAddresses.enet_base + ETHERNET_O_MMC_RX_INTERRUPT_MASK) = 0xFFFFFFFF;
    HWREG(Ethernet_device_struct.baseAddresses.enet_base + ETHERNET_O_MMC_IPC_RX_INTERRUPT_MASK) = 0xFFFFFFFF;
    HWREG(Ethernet_device_struct.baseAddresses.enet_base + ETHERNET_O_MMC_TX_INTERRUPT_MASK) = 0xFFFFFFFF;
    //
    //Do global Interrupt Enable
    //
    (void)Interrupt_enableInProcessor();

    //
    //Assign default ISRs
    //
    Interrupt_registerHandler(INT_EMAC_TX0, Ethernet_transmitISR);
    Interrupt_registerHandler(INT_EMAC_RX0, Ethernet_receiveISR);
    Interrupt_registerHandler(INT_EMAC, Ethernet_genericISRCustom);

    //
    // Convert the mac address string into the 32/16 split variables format
    // that is required by the driver to program into hardware registers.
    // Note: This step is done after the Ethernet_getHandle function because
    //       a dummy MAC address is programmed in that function.
    //
    temp = (uint8_t *)&macLower;
    temp[0] = mac[0];
    temp[1] = mac[1];
    temp[2] = mac[2];
    temp[3] = mac[3];

    temp = (uint8_t *)&macHigher;
    temp[0] = mac[4];
    temp[1] = mac[5];

    //
    // Program the unicast mac address.
    //
    Ethernet_setMACAddr(EMAC_BASE,
                        0,
                        macHigher,
                        macLower,
                        ETHERNET_CHANNEL_0);

    Ethernet_clearMACConfigurationCustom(Ethernet_device_struct.baseAddresses.enet_base,ETHERNET_MAC_CONFIGURATION_RE);
    Ethernet_setMACConfigurationCustom(Ethernet_device_struct.baseAddresses.enet_base,ETHERNET_MAC_CONFIGURATION_RE);
}

void  Lwip_ParamInit(void)
{
    uint8_t pucMACArray[8];
    uint32_t IPAddr; // 0xC0A80004; //192.168.0.4
    uint32_t NetMask;
    uint32_t GWAddr;

    IPAddr  = MAKE_IP_ADDRESS(CmLocalParam.pIpAddr1[3],CmLocalParam.pIpAddr1[2],CmLocalParam.pIpAddr1[1],CmLocalParam.pIpAddr1[0]);
    NetMask = MAKE_IP_ADDRESS(255,255,255,0);
    GWAddr  = MAKE_IP_ADDRESS(CmLocalParam.pIpAddr1[3],CmLocalParam.pIpAddr1[2],CmLocalParam.pIpAddr1[1],1);

    pucMACArray[0] = ((CmIpc_cpu2cm.uniqueID_L >> 0) & 0xff);
    pucMACArray[1] = ((CmIpc_cpu2cm.uniqueID_L >> 8) & 0xff);
    pucMACArray[2] = ((CmIpc_cpu2cm.uniqueID_M >> 0) & 0xff);
    pucMACArray[3] = ((CmIpc_cpu2cm.uniqueID_M >> 8) & 0xff);
    pucMACArray[4] = 0x08;
    pucMACArray[5] = 0x01;

    // Initialize ethernet module.
    Ethernet_init(pucMACArray);

    // Initialze the lwIP library, using DHCP.
    lwIPInit(0, pucMACArray, IPAddr, NetMask, GWAddr, IPADDR_USE_STATIC);

    // Loop forever. All the work is done in interrupt handlers.

    Interrupt_setPriority(INT_EMAC_TX0, 2);
    Interrupt_setPriority(INT_EMAC_RX0, 1);
    Interrupt_enable(INT_EMAC_TX0);
    Interrupt_enable(INT_EMAC_RX0);
    Interrupt_enable(INT_EMAC);
}

//*****************************************************************************
//
// Called by lwIP Library. Could be used for periodic custom tasks.
//
//*****************************************************************************

uint32_t cnt_ms_lwip_Htimer=0;
uint32_t cnt_ms_TX_Htimer=0;
void lwIPHostTimerHandler(void)
{
//  msTime++;

    cnt_ms_lwip_Htimer++;
}

//
// Pack header message into OUT buffer of ptpClock
//
static void msgPackHeader(Octet * buf, void *ptpState)
{
    Nibble transport = 0x80;
    PTPMasterState *ptpMasterState = (PTPMasterState*)ptpState;

    *(UInteger8 *) (buf + 0) = transport;
    *(UInteger4 *) (buf + 1) = 0x2;
    *(UInteger8 *) (buf + 4) = 0;

    if (PTP_TWO_STEP)
        *(UInteger8 *) (buf + 6) = PTP_TWO_STEP;

    memset((buf + 8), 0, 8);
    memcpy((buf + 20), ptpMasterState->portIdentity.clockIdentity,
           CLOCK_IDENTITY_LENGTH);

    *(UInteger16 *) (buf + 28) =
            flip16(ptpMasterState->portIdentity.portNumber);

    *(UInteger8 *) (buf + 33) = 0x7F;
}

//
// Pack SYNC message into OUT buffer of ptpClock
//
static void msgPackSync(Octet * buf, void *ptpState)
{
    PTPMasterState *ptpMasterState = (PTPMasterState*)ptpState;
    msgPackHeader(buf, ptpState);

    *(char *)(buf + 0) = *(char *)(buf + 0) & 0xF0;
    *(char *)(buf + 0) = *(char *)(buf + 0) | 0x00; /* Table 19 */

    *(UInteger16 *) (buf + 2) = flip16(SYNC_LENGTH);
    *(UInteger16 *) (buf + 30) =
            flip16(ptpMasterState->syncSeqId);

    *(UInteger8 *) (buf + 32) = 0x00;   /* Table 23 */
    *(Integer8 *) (buf + 33) = 0; // We'll send sync every second

    if(!PTP_TWO_STEP)
    {
        *(UInteger16 *) (buf + 34) =
                flip16(ptpMasterState->syncTimestamp.secondsField.msb);
        *(UInteger32 *) (buf + 36) =
                flip32(ptpMasterState->syncTimestamp.secondsField.lsb);
        *(UInteger32 *) (buf + 40) =
                flip32(ptpMasterState->syncTimestamp.nanosecondsField);
    }
}

//
// pack Follow_up message into OUT buffer of ptpClock
//
static void msgPackFollowUp(Octet * buf, void *ptpState)
{
    PTPMasterState *ptpMasterState = (PTPMasterState*)ptpState;
    msgPackHeader(buf, ptpState);

    *(char *)(buf + 0) = *(char *)(buf + 0) & 0xF0;
    *(char *)(buf + 0) = *(char *)(buf + 0) | 0x08; /* Table 19 */

    *(UInteger16 *) (buf + 2) = flip16(FOLLOW_UP_LENGTH);
    *(UInteger16 *) (buf + 30) =
            flip16(ptpMasterState->syncSeqId);

    *(UInteger8 *) (buf + 32) = 0x02;   /* Table 23 */
    *(Integer8 *) (buf + 33) = 0;   // we're sending sync every one second.

    *(UInteger16 *) (buf + 34) =
            flip16(ptpMasterState->syncTimestamp.secondsField.msb);
    *(UInteger32 *) (buf + 36) =
            flip32(ptpMasterState->syncTimestamp.secondsField.lsb);
    *(UInteger32 *) (buf + 40) =
            flip32(ptpMasterState->syncTimestamp.nanosecondsField);
}

//
// pack delayResp message into OUT buffer of ptpClock
//
static void msgPackDelayResp(Octet * buf, void *ptpState)
{
    PTPMasterState *ptpMasterState = (PTPMasterState*)ptpState;
    msgPackHeader(buf, ptpMasterState);

    *(char *)(buf + 0) = *(char *)(buf + 0) & 0xF0;
    *(char *)(buf + 0) = *(char *)(buf + 0) | 0x09; /* Table 19 */

    *(UInteger16 *) (buf + 2) = flip16(DELAY_RESP_LENGTH);
    *(UInteger8 *) (buf + 4) = ptpMasterState->delayReqHeader.domainNumber;

    *(UInteger16 *) (buf + 30) =
            flip16(ptpMasterState->delayReqHeader.sequenceId);

    *(UInteger8 *) (buf + 32) = 0x03; /* Table 23 */
    *(Integer8 *) (buf + 33) = 0; /* Table 24 */

    *(UInteger16 *) (buf + 34) =
        flip16(ptpMasterState->delayReqRecvTimestamp.secondsField.msb);
    *(UInteger32 *) (buf + 36) =
        flip32(ptpMasterState->delayReqRecvTimestamp.secondsField.lsb);
    *(UInteger32 *) (buf + 40) =
       flip32(ptpMasterState->delayReqRecvTimestamp.nanosecondsField);
    memcpy((buf + 44),
           ptpMasterState->delayReqHeader.sourcePortIdentity.clockIdentity,
           CLOCK_IDENTITY_LENGTH);
    *(UInteger16 *) (buf + 52) =
        flip16(ptpMasterState->delayReqHeader.sourcePortIdentity.portNumber);
}

//
// Unpack Header from IN buffer to msgTmpHeader field
//
static void msgUnpackHeader(Octet * buf, MsgHeader * header)
{
    header->transportSpecific = (*(Nibble *) (buf + 0)) >> 4;
    header->messageType = (*(Enumeration4 *) (buf + 0)) & 0x0F;
    header->versionPTP = (*(UInteger4 *) (buf + 1)) & 0x0F;

    //
    // force reserved bit to zero if not
    //
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

static void msgPackDelayReq(Octet * buf, void *ptpState)
{
    PTPSlaveState *ptpSlaveState = (PTPSlaveState*)ptpState;
    msgPackHeader(buf, ptpSlaveState);

    *(char *)(buf + 0) = *(char *)(buf + 0) & 0xF0;
    *(char *)(buf + 0) = *(char *)(buf + 0) | 0x01; /* Table 19 */
    *(UInteger16 *) (buf + 2) = flip16(DELAY_REQ_LENGTH);

    *(UInteger16 *) (buf + 30) = flip16(ptpSlaveState->delayReqSeqId);
    *(UInteger8 *) (buf + 32) = 0x01; /* Table 23 */
    *(Integer8 *) (buf + 33) = 0x7F; /* Table 24 */
}

static void sendMessage(Octet *msg, uint32_t messageType, void *ptpState, Ethernet_Pkt_Desc *pktDesc)
{
    uint32_t pktLen;
    PTPMasterState *ptpMasterState = (PTPMasterState*)ptpState;

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

    //
    // Reset the buffer
    //
    memset(msg + 8, 0, PACKET_LENGTH - 8);

    switch(messageType)
    {
    case SYNC:
        pktDesc->flags |= ETHERNET_PKT_FLAG_TTSE;
        msgPackSync(msg + 8, ptpMasterState);
        pktLen = SYNC_LENGTH;
        break;
    case FOLLOW_UP:
        msgPackFollowUp(msg + 8, ptpMasterState);
        pktLen = FOLLOW_UP_LENGTH;
        gPtpMasterState.syncSeqId++;
        break;
    case DELAY_RESP:
        msgPackDelayResp(msg + 8, ptpMasterState);
        pktLen = DELAY_RESP_LENGTH;
        break;
    default:
        break;
    }

    pktDesc->pktLength = pktLen + 6 + 2;
    pktDesc->validLength = pktDesc->pktLength;

    Ethernet_sendPacket(emac_handle, pktDesc);
}


static void fromInternalTime(TimeInternal * internal, Timestamp * external)
{
    external->secondsField.lsb = internal->seconds;
    external->nanosecondsField = internal->nanoseconds;
    external->secondsField.msb = 0;
}

static void normalizeTime(TimeInternal * r)
{
    r->seconds += r->nanoseconds / ONE_BILLION;
    r->nanoseconds -= r->nanoseconds / ONE_BILLION * ONE_BILLION;

    if (r->seconds > 0 && r->nanoseconds < 0) {
        r->seconds -= 1;
        r->nanoseconds += ONE_BILLION;
    }
    else if (r->seconds < 0 && r->nanoseconds > 0) {
        r->seconds += 1;
        r->nanoseconds -= ONE_BILLION;
    }
}

static void subTime(TimeInternal * r, const TimeInternal * x, const TimeInternal * y)
{
    r->seconds = x->seconds - y->seconds;
    r->nanoseconds = x->nanoseconds - y->nanoseconds;

    normalizeTime(r);
}

static void addTime(TimeInternal * r, const TimeInternal * x, const TimeInternal * y)
{
    r->seconds = x->seconds + y->seconds;
    r->nanoseconds = x->nanoseconds + y->nanoseconds;

    normalizeTime(r);
}

static void div2Time(TimeInternal *r)
{
    r->nanoseconds += r->seconds % 2 * ONE_BILLION;
    r->seconds /= 2;
    r->nanoseconds /= 2;

    normalizeTime(r);
}

static void toInternalTime(TimeInternal * internal, Timestamp * external)
{
    //
    // Program will not run after 2038...
    //
    if (external->secondsField.lsb < INT_MAX) {
        internal->seconds = external->secondsField.lsb;
        internal->nanoseconds = external->nanosecondsField;
    }
    else
    {
        return;
    }
}

static void getTime(TimeInternal *time)
{
    //
    // Fetch the current system time.
    //
    Ethernet_getSysTimePTP(EMAC_BASE, (uint32_t *)&time->seconds,
                          (uint32_t *)&time->nanoseconds);
}

static void setTime(TimeInternal *time)
{
    //
    // Set the new system time.
    //
    Ethernet_setSysTimePTP(EMAC_BASE, (uint32_t)time->seconds,
                            (uint32_t)time->nanoseconds);
}

static void updateClock(void)
{
    TimeInternal timeTmp;

    if(gPtpSlaveState.meanPathDelayValid == FALSE)
    {
        return;
    }

    if((gPtpSlaveState.offsetFromMaster.seconds != 0) ||
       (llabs((long long)gPtpSlaveState.offsetFromMaster.nanoseconds) >
        PTP_OFM_NANOSECONDS_CUTOFF))
    {
        getTime(&timeTmp);
        subTime(&timeTmp, &timeTmp, &gPtpSlaveState.offsetFromMaster);
        setTime(&timeTmp);
        gPtpSlaveState.clockUpdateCount++;
    }
}

