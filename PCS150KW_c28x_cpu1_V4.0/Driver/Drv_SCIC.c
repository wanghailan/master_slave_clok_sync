//#############################################################################
//
// FILE:   Drv_SCI.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

#include <Driver/device/device.h>
#include <Driver/device/driverlib.h>
#include "bsp.h"

#define  SCIBase    SCIC_BASE

uint16_t    SCIC_RxBuf[SCIC_BufSize] = {0};
int16_t     SCIC_RxCnt = 0;
int32_t     SCIC_RxTimeOut = 0;

uint16_t    SCIC_TxBuf[SCIC_BufSize] = {0};
int16_t     SCIC_TxCnt = 0;
int16_t     SCIC_TxFlag = 0;

static void Drv_SCIC_Init(uint32_t ulBaudRate, uint16_t ucDataBits, uint16_t ucParity);
//__interrupt void SCICTxISR(void);
__interrupt void SCICRxISR(void);

//*****************************************************************************
//
// PINMUX Configurations
//
//*****************************************************************************

void Drv_SCICPin_Init(void)
{
    // SCIC ->Pinmux
    GPIO_setPinConfig(SCIC_SCIRX_PIN_CONFIG);
    GPIO_setDirectionMode(SCIC_SCIRX_GPIO, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(SCIC_SCIRX_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(SCIC_SCIRX_GPIO, GPIO_QUAL_ASYNC);

    GPIO_setPinConfig(SCIC_SCITX_PIN_CONFIG);
    GPIO_setDirectionMode(SCIC_SCITX_GPIO, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(SCIC_SCITX_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(SCIC_SCITX_GPIO, GPIO_QUAL_ASYNC);
    //SCIC_OE
    GPIO_setPinConfig(SCIC_OE_PIN_CONFIG);
    GPIO_setDirectionMode(SCIC_GPIO_PIN_OE, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(SCIC_GPIO_PIN_OE, GPIO_PIN_TYPE_STD);

    SysCtl_selectCPUForPeripheralInstance(SYSCTL_CPUSEL_SCIC, SYSCTL_CPUSEL_CPU1);
    GPIO_setControllerCore(SCIC_GPIO_PIN_OE, GPIO_CORE_CPU1);
}

//*****************************************************************************
//
// SCI Configurations
//
//*****************************************************************************
void Drv_SCICInit(void)
{
    Drv_SCIC_Init(38400,8,0);
    SCIC_RxEn();//接收
    SCIC_RxCnt = 0;
    SCIC_RxTimeOut = 0;
}


/**********************************************************************************
  函数名称：Drv_SCI_WriteByte()
  功能描述：SCI发送
  修改日期：2018-11-14
***********************************************************************************/
void   Drv_SCIC_WriteByte(uint16_t dat)
{
//        while(SCI_getTxFIFOStatus(SCIBase) == SCI_FIFO_TX16);//fifo满时等待
        SCI_writeCharBlockingFIFO(SCIBase, dat);
//        while(SCI_isSpaceAvailableNonFIFO(SCIBase) == false);//等待SCITXBUF可以接收下一个数据
}

/**********************************************************************************
  函数名称：Drv_SCI_WriteDataBuf()
  功能描述：SCI发送
  修改日期：2024-08-31
***********************************************************************************/
int16_t   Drv_SCIC_WriteDataBuf(int16_t dLen,uint16_t *dBuf)
{
    SCIC_TxEn();//发送
    SCI_writeCharArray(SCIBase,dBuf,dLen);
    while(SCI_getTxFIFOStatus(SCIBase) != SCI_FIFO_TX0);//等待FIFO发送完
    while(SCI_isTransmitterBusy(SCIBase) == true);//等待发送器空标志
    DEVICE_DELAY_US(400);
    SCIC_RxEn();//转接收
    return 0;
}

static void Drv_SCIC_Init(uint32_t ulBaudRate, uint16_t ucDataBits, uint16_t ucParity)
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
    Interrupt_register(INT_SCIC_RX, &SCICRxISR);
    // Enable the TXFF and RXFF interrupts.
    SCI_enableInterrupt(SCIBase, SCI_INT_RXFF);
    SCI_disableInterrupt(SCIBase, SCI_INT_RXERR);

    // Set the transmit FIFO level to 0 and the receive FIFO level to 2.
    SCI_setFIFOInterruptLevel(SCIBase, SCI_FIFO_TX0, SCI_FIFO_RX1);
    SCI_performSoftwareReset(SCIBase);
    SCI_resetTxFIFO(SCIBase);
    SCI_resetRxFIFO(SCIBase);
    // Enable the interrupts in the PIE: Group 8 interrupts 1 & 2.
    Interrupt_enable(INT_SCIC_RX);
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
//    SCI_disableInterrupt(SCIBase, SCI_INT_TXFF);
//
////    msg = "\r\nEnter two characters: \0";
////    SCI_writeCharArray(SCIBase, (uint16_t*)msg, 26);
//
//    // Acknowledge the PIE interrupt.
//    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);
//}

//
// SCICRxISR - Read two characters from the RXBUF and echo them back.
//
__interrupt void SCICRxISR(void)
{
    // Read characters from the FIFO.
    SCIC_RxTimeOut = 0;
    if(SCIC_RxCnt < SCIC_BufSize)
    {
        SCIC_RxBuf[SCIC_RxCnt++] = SCI_readCharBlockingFIFO(SCIBase);
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

int16_t SCICRx_Poll(void)
{
    /* 超过3.5个字符时间后 通知主程序开始解码 */
    if (SCIC_RxTimeOut < SciRxTimeOut)
    {
        return -1;  // 没有超时，继续接收。不要清零
    }

    SCIC_RxTimeOut = 0;     // 清超时标志

    if (SCIC_RxCnt < 4)     // 接收到的数据小于4个字节就认为错误，地址（8bit）+指令（8bit）+操作寄存器（16bit）
    {
        SCIC_RxCnt = 0;     // 必须清零计数器，方便下次帧同步
        return -2;
    }

    // 透传数据到CM
    ipc_TxData_Cpu1ToCM(ipcCmd_SciC,SCIC_RxCnt);

    SCIC_RxCnt = 0;         // 必须清零计数器，方便下次帧同步
    return 0;
}

int16_t SCICTx_Poll(void)
{
    if(SCIC_TxFlag)
    {
        if (SCIC_TxCnt >= 5)     // 接收到的数据小于4个字节就认为错误，地址（8bit）+指令（8bit）+操作寄存器（16bit）
        {
            Drv_SCIC_WriteDataBuf(SCIC_TxCnt,SCIC_TxBuf);
        }
        SCIC_TxFlag = 0;
    }
    return 0;
}

void SciC_Poll(void)
{
    SCICRx_Poll();
    SCICTx_Poll();
}
