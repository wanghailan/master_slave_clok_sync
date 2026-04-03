/*
 * ptp_master_sync.c
 *
 *  Created on: 2026年3月31日
 *      Author: whl
 */
#include "ptp_master_sync.h"
#include "Eth_mii.h"
#include "bsp.h"

#define ETHERNET_MAC_TIMESTAMP_CONTROL_TSCFUPDT 0x00000020U

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
}

// ptpd init
void ptp_master_init()
{
    uint32_t i;
    uint32_t varPtpConfig = 0;
    float subSecondInc;
    uint32_t timeSec, timeNanosec;

    // 强制设置以太网MAC为100Mbps模式
    Ethernet_setMACConfiguration(EMAC_BASE, ETHERNET_MAC_CONFIGURATION_100MBIT);

    // ========== 新增：先禁用时间戳，避免配置PPS时状态冲突 ==========
    uint32_t tsCtrl = HWREG(EMAC_BASE + ETHERNET_O_MAC_TIMESTAMP_CONTROL);
    HWREG(EMAC_BASE + ETHERNET_O_MAC_TIMESTAMP_CONTROL) = tsCtrl & ~ETHERNET_MAC_TIMESTAMP_CONTROL_TSENA;


    // PTP相关配置 ETHERNET_MAC_TIMESTAMP_CONTROL_TSIPENA / 0x20
    varPtpConfig = (0 << ETHERNET_MAC_TIMESTAMP_CONTROL_SNAPTYPSEL_S) |
                            ETHERNET_MAC_TIMESTAMP_CONTROL_TSCTRLSSR |
                            ETHERNET_MAC_TIMESTAMP_CONTROL_TSMSTRENA |
                            ETHERNET_MAC_TIMESTAMP_CONTROL_TSEVNTENA |
                            ETHERNET_MAC_TIMESTAMP_CONTROL_TSVER2ENA |
                            0x20;

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
                        0x00191B01,
                        ETHERNET_CHANNEL_0);

    // 配置速度和双工模式
    Ethernet_setMACConfiguration(EMAC_BASE, ((uint32_t)1 << 14));
    Ethernet_setMACConfiguration(EMAC_BASE, ((uint32_t)1 << 13));

    // 重新使能TX/RX
    Ethernet_setMACConfiguration(EMAC_BASE, 0x2);  // 使能TX
    Ethernet_setMACConfiguration(EMAC_BASE, 0x1);  // 使能RX

//    // PPS输出配置为中断模式
//    Ethernet_selectTargetInterruptOrPulsePPS(
//                            EMAC_BASE,
//                            ETHERNET_MAC_PPS_OUT_INSTANCE_0,
//                            ETHERNET_MAC_PPS_CONTROL_TRGTMODSEL_INTERRUPT);

    // 配置PPS输出为脉冲模式(新增)
    Ethernet_selectTargetInterruptOrPulsePPS(
            EMAC_BASE,
            ETHERNET_MAC_PPS_OUT_INSTANCE_0,
            ETHERNET_MAC_PPS_CONTROL_TRGTMODSEL_PULSE);

//    // ========== 新增：设置PPS脉冲宽度（单位：PTP参考时钟周期） ==========
//    // PTP参考时钟频率为25MHz（周期40ns），脉冲宽度设为1000ms
//    // 实际频率需根据PTP_REF_CLOCK_PERIOD计算：周期数 =1 / PTP_REF_CLOCK_PERIOD
//    // 脉冲宽度 = 500ms / 40ns = 25,000,000

    // 设置PPS脉冲宽度为500ms
    uint32_t PTP_CLOCK_FREQ = 25000000;
    uint32_t ppsWidth = (uint32_t)(0.5 * PTP_CLOCK_FREQ);
    HWREG(EMAC_BASE + ETHERNET_MAC_PPS_WIDTH) = ppsWidth;  // 设置宽度寄存器
    HWREG(EMAC_BASE + ETHERNET_O_MAC_PPS_CONTROL) |= ETHERNET_MAC_PPS_CONTROL_PPSEN0;  // 使能PPS0输出

    // 初始化时设置PPS目标时间
    // 获取当前系统时间
    Ethernet_getSysTimePTP(EMAC_BASE, &timeSec, &timeNanosec);

    // 设置PPS在下一秒0纳秒时刻触发，之后每秒自动重复
    Ethernet_setTargetTimePPS(EMAC_BASE,
                              ETHERNET_MAC_PPS_OUT_INSTANCE_0,
                              timeSec + 1,
                              0);

    // ========== 新增：重新使能时间戳 ==========
    HWREG(EMAC_BASE + ETHERNET_O_MAC_TIMESTAMP_CONTROL) = tsCtrl | ETHERNET_MAC_TIMESTAMP_CONTROL_TSENA;

    // We need to set a standard defined Multicast address : 01:1B:19:00:00:00
    // as the Destination address in the ethernet frame and that is how the
    // receiver will recognize it as a valid PTP over Ethernet packet.
    //
    i=0; *((uint32_t *)gMsgBuf + i) = 0x00191B01;
    i++; *((uint32_t *)gMsgBuf + i)  = 0xF7880000;

    InitConstants(&gPtpMasterState);

}

void ptp_master_run()
{
    static uint32_t lastRunSec = 0;
    uint32_t timeSec, timeNanosec;
    const uint32_t TIMEOUT_MAX = 2000000;
    uint32_t timeout = 0;

    // 1. 获取当前时间，计算下一秒目标
    Ethernet_getSysTimePTP(EMAC_BASE, &timeSec, &timeNanosec);

    // ===== 每秒只执行一次 =====
    if(timeSec == lastRunSec)
    {
        return;  // 同一秒内不重复执行
    }
    lastRunSec = timeSec;

    uint32_t targetSec = timeSec + 1;

    // 2. 设置PPS在目标时间输出脉冲
    Ethernet_setTargetTimePPS(EMAC_BASE, ETHERNET_MAC_PPS_OUT_INSTANCE_0,
                              targetSec, 0);
//    // 3. 等待目标时间到达
//    while(timeout < TIMEOUT_MAX)
//    {
//        Ethernet_getSysTimePTP(EMAC_BASE, &timeSec, &timeNanosec);
//        if(timeSec >= targetSec)
//            break;
//        timeout++;
//    }
//    if(timeout >= TIMEOUT_MAX)
//    {
//        CmIpc_cm2cpu.IpcCpu2Cm_Fault = 1;
//        return;
//    }

    // 4. 发送SYNC报文
    gPtpMasterState.syncTimestampAvailable = FALSE;
    sendMessage((Octet *)gMsgBuf, SYNC, &gPtpMasterState, &gPktDesc);

    // 5. 等待时间戳捕获（脉冲模式下可能超时）
    timeout = 0;
    while((gPtpMasterState.syncTimestampAvailable == FALSE) && (timeout < TIMEOUT_MAX))
    {
        timeout++;
    }

    // 超时后填充估算时间戳到 syncTimestamp
    if(timeout >= TIMEOUT_MAX)
    {
        CmIpc_cm2cpu.IpcCpu2Cm_Fault = 1;

        uint32_t estSec, estNs;
        Ethernet_getSysTimePTP(EMAC_BASE, &estSec, &estNs);

        gPtpMasterState.syncTimestamp.secondsField.lsb = estSec;
        gPtpMasterState.syncTimestamp.secondsField.msb = 0;
        gPtpMasterState.syncTimestamp.nanosecondsField = estNs;

        gPtpMasterState.syncTimestampAvailable = TRUE;
    }

    // 6. 发送FOLLOW_UP（使用 syncTimestamp 中的时间戳）
    sendMessage((Octet *)gMsgBuf, FOLLOW_UP, &gPtpMasterState, &gPktDesc);
}


