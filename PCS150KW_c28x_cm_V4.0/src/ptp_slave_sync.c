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
#include "eth_common.h"

extern PTPSlaveState gPtpSlaveState;


static void InitConstants(PTPSlaveState *ptpSlaveState);



/*============================ PTP Slave API ============================*/
void ptp_slave_init()
{
    uint32_t i;
    uint32_t varPtpConfig = 0;
    uint32_t timeSec;
    uint32_t timeNanosec;
    float subSecondInc;

    // ptp configuration time control register
    varPtpConfig = 0x0 |
                    (0 << ETHERNET_MAC_TIMESTAMP_CONTROL_SNAPTYPSEL_S) |
                    ETHERNET_MAC_TIMESTAMP_CONTROL_TSCTRLSSR |
                    ETHERNET_MAC_TIMESTAMP_CONTROL_TSMSTRENA |
                    ETHERNET_MAC_TIMESTAMP_CONTROL_TSEVNTENA |
                    ETHERNET_MAC_TIMESTAMP_CONTROL_TSVER2ENA |
                    ETHERNET_MAC_TIMESTAMP_CONTROL_TSIPENA;

    //
    // Subsecond incrSement is added to the systime counter every ptp clock tick
    // hence for Digital rollover, it is simply the time period of the clock tick.
    //
    subSecondInc = PTP_REF_CLOCK_PERIOD;

    Ethernet_setConfigTimestampPTP(EMAC_BASE, varPtpConfig, subSecondInc);
    Ethernet_enableSysTimePTP(EMAC_BASE);

    // update sysClock time
    int32_t offsetSec = 0;
    int32_t offsetNanoSec = 0;
    Ethernet_updateSysTimePTP(EMAC_BASE, offsetSec, offsetNanoSec, true);

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
                        0x00191B01U,
                        ETHERNET_CHANNEL_0);

    //
    // Configure PPS0 as fixed 1Hz waveform output.
    // The slave does not use target-time pulse scheduling.
    //
    HWREG(EMAC_BASE + ETHERNET_O_MAC_PPS_CONTROL) = 0x00U;

    Ethernet_selectTargetInterruptOrPulsePPS(EMAC_BASE,
                                             ETHERNET_MAC_PPS_OUT_INSTANCE_0,
                                             ETHERNET_MAC_PPS_CONTROL_TRGTMODSEL_PULSE);

    Ethernet_setPeriodPPS(EMAC_BASE,
                          ETHERNET_MAC_PPS_OUT_INSTANCE_0,
                          PTP_REF_CLOCK_FREQ / 100U,
                          PTP_REF_CLOCK_FREQ - 1U);

    Ethernet_setFixedModePPS(EMAC_BASE,
                             ETHERNET_MAC_PPS_CONTROL_PPSCTRL_PPS_OUTPUT_1HZ);

    InitConstants(&gPtpSlaveState);

}

void ptp_slave_run(void)
{
    // 直接进入到主循环，硬件中断调用回调函数处理
}

static void InitConstants(PTPSlaveState *ptpSlaveState)
{
    uint32_t mac_low,mac_high, i, j;
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

    //
    // Init global constants.
    //
    for (i = 0, j = 0; i < CLOCK_IDENTITY_LENGTH; i++)
    {
       if (i == 3) ptpSlaveState->portIdentity.clockIdentity[i]=0xFF;
       else if (i==4) ptpSlaveState->portIdentity.clockIdentity[i]=0xFE;
       else
       {
           ptpSlaveState->portIdentity.clockIdentity[i] =
                   ptpSlaveState->port_uuid_field[j];
         j++;
       }
    }

    ptpSlaveState->portIdentity.portNumber = 1U;
}


