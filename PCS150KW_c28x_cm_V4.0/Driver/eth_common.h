/*
 * eth_common.h
 *
 *  Created on: 2026年3月31日
 *      Author: whl
 */

#ifndef DRIVER_ETH_COMMON_H_
#define DRIVER_ETH_COMMON_H_
#include "driverlib_cm.h"
#include "string.h"
#include "cm.h"

#define PTP_REF_CLOCK_FREQ   25000000UL   // 25MHz for MII 100Mbps
#define PTP_REF_CLOCK_PERIOD 1000000000UL / PTP_REF_CLOCK_FREQ  // 40ns
#define NUM_PACKET_DESC_RX_APPLICATION  8U


// Network related definitions as follows
// Taking care of network byte order conversions using these macros.
#define PP_HTONS(x) ((uint16_t)((((x) & (uint16_t)0x00ffU) << 8) |             \
                     (((x) & (uint16_t)0xff00U) >> 8)))
#define PP_NTOHS(x) PP_HTONS(x)
#define PP_HTONL(x) ((((x) & (uint32_t)0x000000ffUL) << 24) |                  \
                     (((x) & (uint32_t)0x0000ff00UL) <<  8) |                  \
                     (((x) & (uint32_t)0x00ff0000UL) >>  8) |                  \
                     (((x) & (uint32_t)0xff000000UL) >> 24))
#define PP_NTOHL(x) PP_HTONL(x)
#define flip16(x) PP_HTONS(x)
#define flip32(x) PP_HTONL(x)


// Following definitions and prototypes are specific to the PTP state machine
// and have been adapted from the IEEE 1588 standard spec.
#define PTP_TWO_STEP                            0x02
#define PTP_UUID_LENGTH                         6
#define CLOCK_IDENTITY_LENGTH                   8
#define FLAG_FIELD_LENGTH                       2

#define HEADER_LENGTH                           34
#define ANNOUNCE_LENGTH                         64
#define SYNC_LENGTH                             44
#define FOLLOW_UP_LENGTH                        44
#define PDELAY_REQ_LENGTH                       54
#define DELAY_REQ_LENGTH                        44
#define DELAY_RESP_LENGTH                       54
#define PDELAY_RESP_LENGTH                      54
#define PDELAY_RESP_FOLLOW_UP_LENGTH            54
#define MANAGEMENT_LENGTH                       48

#define PACKET_LENGTH       1538U
#define PTP_HEADER_OFFSET   14U
#define ETHERNET_NO_OF_RX_PACKETS   8U
#define ETHERNET_MAX_PACKET_LENGTH  PACKET_LENGTH
#define NUM_PACKET_DESC_RX_APPLICATION 8U

// 强制设置以太网MAC为100Mbps模式
#define ETHERNET_MAC_CONFIGURATION_100MBIT      0x4000U

typedef enum {FALSE=0, TRUE} Boolean;
typedef char Octet;
typedef signed char Integer8;
typedef signed short Integer16;
typedef signed int Integer32;
typedef unsigned char UInteger8;
typedef unsigned short UInteger16;
typedef unsigned int UInteger32;
typedef unsigned short Enumeration16;
typedef unsigned char Enumeration8;
typedef unsigned char Enumeration4;
typedef unsigned char UInteger4;
typedef unsigned char Nibble;

// brief Implementation specific of Integer64 type
typedef struct {
    unsigned int lsb;
    int msb;
} Integer64;

typedef struct {
    unsigned int lsb;
    unsigned short msb;
} UInteger48;

typedef struct  {
    UInteger48 secondsField;
    UInteger32 nanosecondsField;
} Timestamp;

// brief Time structure to handle Linux time information
typedef struct {
    Integer32 seconds;
    Integer32 nanoseconds;
} TimeInternal;

// brief The ClockIdentity type identifies a clock
typedef Octet ClockIdentity[CLOCK_IDENTITY_LENGTH];

// brief The PortIdentity identifies a PTP port.
typedef struct {
    ClockIdentity clockIdentity;
    UInteger16 portNumber;
} PortIdentity;

// brief The common header for all PTP messages (Table 18 of the spec)
// Message header
typedef struct {
    Nibble transportSpecific;
    Enumeration4 messageType;
    UInteger4 versionPTP;
    UInteger16 messageLength;
    UInteger8 domainNumber;
    Octet flagField[2];
    Integer64 correctionfield;
    PortIdentity sourcePortIdentity;
    UInteger16 sequenceId;
    UInteger8 controlField;
    Integer8 logMessageInterval;
} MsgHeader;

// 主机收发报文结构体
typedef struct
{
    PortIdentity portIdentity;
    Octet port_uuid_field[PTP_UUID_LENGTH];
    Timestamp syncTimestamp;
    Timestamp delayReqRecvTimestamp;
    uint16_t syncSeqId;
    uint16_t portNumber;
    Boolean syncTimestampAvailable;
    Boolean sendingDelayResp;
    MsgHeader delayReqHeader;
} PTPMasterState;

// 从机收发报文结构体
typedef struct {
    PortIdentity portIdentity;
    Octet port_uuid_field[PTP_UUID_LENGTH];

    Timestamp syncRecvTimestamp;     // Sync报文接收时间t2
    Timestamp syncOriginTimestamp;   // T1
    Timestamp delayReqSentTimestamp; // Delay_Req发送时间t3
    Timestamp delayReqRecvTimestamp; // Master的Delay_Req接收时间t4（来自Delay_Resp）

    TimeInternal delayMS;
    TimeInternal delaySM;
    TimeInternal offsetFromMaster;
    TimeInternal meanPathDelay;

    uint16_t lastSyncSeqId;
    uint16_t followUpSeqId;
    uint16_t delayReqSeqId;
    uint16_t portNumber;
    uint32_t clockUpdateCount;

    Boolean syncReceived;
    Boolean followUpReceived;
    Boolean delayRespReceived;

    uint32_t  lockCount;
    bool      isLocked;
} PTPSlaveState;


// brief PTP Messages
enum {
    SYNC=0x0,
    DELAY_REQ=0x1,
    FOLLOW_UP = 0x8,
    DELAY_RESP = 0x9,
};

Ethernet_Pkt_Desc gPktDesc;

extern uint8_t g_ptpMode;   // 0=Master, 1=Slave

#endif /* DRIVER_ETH_COMMON_H_ */
