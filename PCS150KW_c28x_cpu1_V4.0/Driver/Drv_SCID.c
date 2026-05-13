//#############################################################################
//
// FILE:   Drv_SCI.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

#include <Driver/device/device.h>
#include <Driver/device/driverlib.h>
#include <Drv_SCID.h>
#include "bsp.h"


#define  SCIBase    SCID_BASE

uint16_t    SCID_RxBuf[SCID_BufSize] = {0};
int16_t     SCID_RxCnt = 0;
int32_t     SCID_RxTimeOut = 0;

uint16_t    SCID_TxBuf[SCID_BufSize] = {0};
int16_t     SCID_TxCnt = 0;
int16_t     SCID_TxFlag = 0;



static void Drv_SCID_Init(uint32_t ulBaudRate, uint16_t ucDataBits, uint16_t ucParity);
//__interrupt void SCIDTxISR(void);
__interrupt void SCIDRxISR(void);

//*****************************************************************************
//
// PINMUX Configurations
//
//*****************************************************************************

void Drv_SCIDPin_Init(void)
{
    // SCIC ->Pinmux
    GPIO_setPinConfig(SCID_SCIRX_PIN_CONFIG);
    GPIO_setDirectionMode(SCID_SCIRX_GPIO, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(SCID_SCIRX_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(SCID_SCIRX_GPIO, GPIO_QUAL_ASYNC);

    GPIO_setPinConfig(SCID_SCITX_PIN_CONFIG);
    GPIO_setDirectionMode(SCID_SCITX_GPIO, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(SCID_SCITX_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(SCID_SCITX_GPIO, GPIO_QUAL_ASYNC);
    //SCIC_OE
    GPIO_setPinConfig(SCID_OE_PIN_CONFIG);
    GPIO_setDirectionMode(SCID_GPIO_PIN_OE, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(SCID_GPIO_PIN_OE, GPIO_PIN_TYPE_STD);

    SysCtl_selectCPUForPeripheralInstance(SYSCTL_CPUSEL_SCID, SYSCTL_CPUSEL_CPU1);
    GPIO_setControllerCore(SCID_GPIO_PIN_OE, GPIO_CORE_CPU1);
}
//*****************************************************************************
//
// SCI Configurations
//
//*****************************************************************************
void Drv_SCIDInit(void)
{
    Drv_SCID_Init(38400,8,0);
    SCID_RxEn();//接收
    SCID_RxCnt = 0;
    SCID_RxTimeOut = 0;
}


/**********************************************************************************
  函数名称：Drv_SCID_WriteByte()
  功能描述：SCI发送
  修改日期：2024-11-13
***********************************************************************************/
void   Drv_SCID_WriteByte(uint16_t dat)
{
//        while(SCI_getTxFIFOStatus(SCIC_BASE) == SCI_FIFO_TX16);//fifo满时等待
        SCI_writeCharBlockingFIFO(SCIBase, dat);
//        while(SCI_isSpaceAvailableNonFIFO(SCIC_BASE) == false);//等待SCITXBUF可以接收下一个数据
}

/**********************************************************************************
  函数名称：Drv_SCI_WriteDataBuf()
  功能描述：SCI发送
  修改日期：2024-08-31
***********************************************************************************/
int16_t   Drv_SCID_WriteDataBuf(int16_t dLen,uint16_t *dBuf)
{
    SCID_TxEn();//发送
    SCI_writeCharArray(SCIBase,dBuf,dLen);
    while(SCI_getTxFIFOStatus(SCIBase) != SCI_FIFO_TX0);//等待FIFO发送完
    while(SCI_isTransmitterBusy(SCIBase) == true);//等待发送器空标志
    DEVICE_DELAY_US(400);
    SCID_RxEn();//转接收
    return 0;
}



static void Drv_SCID_Init(uint32_t ulBaudRate, uint16_t ucDataBits, uint16_t ucParity)
{
    uint32_t SCIconfig;

    SCIconfig = 0;
    switch(ucDataBits)
    {
        case 8: SCIconfig |= SCI_CONFIG_WLEN_8;break;
        case 7: SCIconfig |= SCI_CONFIG_WLEN_7;break;
        case 6: SCIconfig |= SCI_CONFIG_WLEN_6;break;
        case 5: SCIconfig |= SCI_CONFIG_WLEN_5;break;
        default:SCIconfig |= SCI_CONFIG_WLEN_8;break;
    };
    SCIconfig |= SCI_CONFIG_STOP_ONE;

    //EPARITY SETTINGS ----------------------------------------------
    switch(ucParity){
        case 2:  SCIconfig |= SCI_CONFIG_PAR_EVEN; break;
        case 1:  SCIconfig |= SCI_CONFIG_PAR_ODD; break;
        case 0:  SCIconfig |= SCI_CONFIG_PAR_NONE; break;
        default: SCIconfig |= SCI_CONFIG_PAR_NONE; break;
    }

    // Initialize SCIC and its FIFO.
    SCI_clearInterruptStatus(SCIBase, SCI_INT_RXFF | SCI_INT_TXFF | SCI_INT_FE | SCI_INT_OE | SCI_INT_PE | SCI_INT_RXERR | SCI_INT_RXRDY_BRKDT | SCI_INT_TXRDY);
    SCI_clearOverflowStatus(SCIBase);
    SCI_resetChannels(SCIBase);
    SCI_setConfig(SCIBase, DEVICE_LSPCLK_FREQ, ulBaudRate, SCIconfig);
    SCI_enableModule(SCIBase);
    SCI_resetChannels(SCIBase);
    SCI_enableFIFO(SCIBase);

    // Map the ISR to the wake interrupt.
//    Interrupt_register(INT_SCIC_TX, &SCICTxISR);
    Interrupt_register(INT_SCID_RX, &SCIDRxISR);
    // Enable the TXFF and RXFF interrupts.
    SCI_enableInterrupt(SCIBase, SCI_INT_RXFF);
    SCI_disableInterrupt(SCIBase, SCI_INT_RXERR);

    // Set the transmit FIFO level to 0 and the receive FIFO level to 2.
    SCI_setFIFOInterruptLevel(SCIBase, SCI_FIFO_TX0, SCI_FIFO_RX1);
    SCI_performSoftwareReset(SCIBase);
    SCI_resetTxFIFO(SCIBase);
    SCI_resetRxFIFO(SCIBase);
    // Enable the interrupts in the PIE: Group 8 interrupts 1 & 2.
    Interrupt_enable(INT_SCID_RX);
//    Interrupt_enable(INT_SCIC_TX);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP8);
}

//
// SCICTxISR - Disable the TXFF interrupt and print message asking
//             for two characters.
//
//__interrupt void SCICTxISR(void)
//{
//    // Disable the TXRDY interrupt.
//    SCI_disableInterrupt(SCIC_BASE, SCI_INT_TXFF);
//
////    msg = "\r\nEnter two characters: \0";
////    SCI_writeCharArray(SCIC_BASE, (uint16_t*)msg, 26);
//
//    // Acknowledge the PIE interrupt.
//    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);
//}

//
// SCICRxISR - Read two characters from the RXBUF and echo them back.
//
__interrupt void SCIDRxISR(void)
{
    // Read characters from the FIFO.
    SCID_RxTimeOut = 0;
    if(SCID_RxCnt < SCID_BufSize)
    {
        SCID_RxBuf[SCID_RxCnt++] = SCI_readCharBlockingFIFO(SCIBase);
    }
    SCI_clearOverflowStatus(SCIBase);
    // Clear the SCI RXFF interrupt and acknowledge the PIE interrupt.
    SCI_clearInterruptStatus(SCIBase, SCI_INT_RXFF);
    if(SCI_getInterruptStatus(SCIBase)&SCI_INT_RXERR != 0)
    {
        SCI_clearInterruptStatus(SCIBase, SCI_INT_RXERR);
        SCI_enableModule(SCIBase);
    }
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP8);
}

int16_t SCIDRx_Poll(void)
{
    /* 超过3.5个字符时间后 通知主程序开始解码 */
    if (SCID_RxTimeOut < SciRxTimeOut)
    {
        return -1;  // 没有超时，继续接收。不要清零
    }

    SCID_RxTimeOut = 0;     // 清超时标志

    if (SCID_RxCnt < 4)     // 接收到的数据小于4个字节就认为错误，地址（8bit）+指令（8bit）+操作寄存器（16bit）
    {
        SCID_RxCnt = 0;     // 必须清零计数器，方便下次帧同步
        return -2;
    }

    // 透传数据到CM
    ipc_TxData_Cpu1ToCM(ipcCmd_SciD,SCID_RxCnt);

    SCID_RxCnt = 0;         // 必须清零计数器，方便下次帧同步
    return 0;
}

int16_t SCIDTx_Poll(void)
{
    if(SCID_TxFlag)
    {
        if (SCID_TxCnt >= 5)     // 接收到的数据小于4个字节就认为错误，地址（8bit）+指令（8bit）+操作寄存器（16bit）
        {
            Drv_SCID_WriteDataBuf(SCID_TxCnt,SCID_TxBuf);
        }
       SCID_TxFlag = 0;
    }
    return 0;
}

void SciD_Poll(void)
{
    SCIDRx_Poll();
    SCIDTx_Poll();
}




