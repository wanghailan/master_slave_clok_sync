//#############################################################################
//
// FILE:   Drv_SCI.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

#include <Driver/device/device.h>
#include <Driver/device/driverlib.h>
#include <Drv_SCIA.h>

#define  SCIAase    SCIA_BASE

uint16_t    SCIA_RxBuf[SCIA_BufSize] = {0};
int16_t     SCIA_RxCnt = 0;
int16_t     SCIA_RxTimeOut = 0;


static void Drv_SCIA_Init(uint32_t ulBaudRate, uint16_t ucDataBits, uint16_t ucParity);
//__interrupt void SCIATxISR(void);
__interrupt void SCIARxISR(void);

//*****************************************************************************
//
// SCI Configurations
//
//*****************************************************************************
void Drv_SCIAInit(void)
{
    Drv_SCIA_Init(38400,8,0);
    SCIA_RxCnt = 0;
    SCIA_RxTimeOut = 0;
}


/**********************************************************************************
  函数名称：Drv_SCI_WriteByte()
  功能描述：SCI发送
  修改日期：2018-11-14
***********************************************************************************/
void   Drv_SCIA_WriteByte(uint16_t dat)
{
//        while(SCI_getTxFIFOStatus(SCIAase) == SCI_FIFO_TX16);//fifo满时等待
        SCI_writeCharBlockingFIFO(SCIAase, dat);
//        while(SCI_isSpaceAvailableNonFIFO(SCIAase) == false);//等待SCITXBUF可以接收下一个数据
}

/**********************************************************************************
  函数名称：Drv_SCI_WriteDataBuf()
  功能描述：SCI发送
  修改日期：2024-08-31
***********************************************************************************/
void   Drv_SCIA_WriteDataBuf(int16_t dLen,uint16_t *dBuf)
{
//    SCIA_TxEn();//发送
    SCI_writeCharArray(SCIAase,dBuf,dLen);
    while(SCI_getTxFIFOStatus(SCIAase) != SCI_FIFO_TX0);//等待FIFO发送完
    while(SCI_isTransmitterBusy(SCIAase) == true);//等待发送器空标志
//    DEVICE_DELAY_US(300);
//    SCIA_RxEn();//转接收
}

//*****************************************************************************
//
// PINMUX Configurations
//
//*****************************************************************************
void Drv_SCIAPin_Init(void)
{
    // SCIA ->Pinmux
    GPIO_setPinConfig(SCIA_SCIRX_PIN_CONFIG);
    GPIO_setDirectionMode(SCIA_SCIRX_GPIO, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(SCIA_SCIRX_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(SCIA_SCIRX_GPIO, GPIO_QUAL_ASYNC);

    GPIO_setPinConfig(SCIA_SCITX_PIN_CONFIG);
    GPIO_setDirectionMode(SCIA_SCITX_GPIO, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(SCIA_SCITX_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(SCIA_SCITX_GPIO, GPIO_QUAL_ASYNC);
}

static void Drv_SCIA_Init(uint32_t ulBaudRate, uint16_t ucDataBits, uint16_t ucParity)
{
    uint32_t SCIAonfig;

    SCIAonfig = 0;
    switch(ucDataBits)
    {
        case 8: SCIAonfig |= SCI_CONFIG_WLEN_8;break;
        case 7: SCIAonfig |= SCI_CONFIG_WLEN_7;break;
        case 6: SCIAonfig |= SCI_CONFIG_WLEN_6;break;
        case 5: SCIAonfig |= SCI_CONFIG_WLEN_5;break;
        default:SCIAonfig |= SCI_CONFIG_WLEN_8;break;
    };
    SCIAonfig |= SCI_CONFIG_STOP_ONE;

    //EPARITY SETTINGS ----------------------------------------------
    switch(ucParity){
        case 2:  SCIAonfig |= SCI_CONFIG_PAR_EVEN; break;
        case 1:  SCIAonfig |= SCI_CONFIG_PAR_ODD; break;
        case 0:  SCIAonfig |= SCI_CONFIG_PAR_NONE; break;
        default: SCIAonfig |= SCI_CONFIG_PAR_NONE; break;
    }

    // Initialize SCIA and its FIFO.
    SCI_clearInterruptStatus(SCIAase, SCI_INT_RXFF | SCI_INT_TXFF | SCI_INT_FE | SCI_INT_OE | SCI_INT_PE | SCI_INT_RXERR | SCI_INT_RXRDY_BRKDT | SCI_INT_TXRDY);
    SCI_clearOverflowStatus(SCIAase);
    SCI_resetChannels(SCIAase);
    SCI_setConfig(SCIAase, DEVICE_LSPCLK_FREQ, ulBaudRate, SCIAonfig);
    SCI_enableModule(SCIAase);
    SCI_resetChannels(SCIAase);
    SCI_enableFIFO(SCIAase);

    // Map the ISR to the wake interrupt.
//    Interrupt_register(INT_SCIA_TX, &SCIATxISR);
    Interrupt_register(INT_SCIA_RX, &SCIARxISR);
    // Enable the TXFF and RXFF interrupts.
    SCI_enableInterrupt(SCIAase, SCI_INT_RXFF);
    SCI_disableInterrupt(SCIAase, SCI_INT_RXERR);

    // Set the transmit FIFO level to 0 and the receive FIFO level to 2.
    SCI_setFIFOInterruptLevel(SCIAase, SCI_FIFO_TX0, SCI_FIFO_RX1);
    SCI_performSoftwareReset(SCIAase);
    SCI_resetTxFIFO(SCIAase);
    SCI_resetRxFIFO(SCIAase);
    // Enable the interrupts in the PIE: Group 8 interrupts 1 & 2.
    Interrupt_enable(INT_SCIA_RX);
//    Interrupt_enable(INT_SCIA_TX);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);

}

//
// SCIATxISR - Disable the TXFF interrupt and print message asking
//             for two characters.
//
//__interrupt void SCIATxISR(void)
//{
//    // Disable the TXRDY interrupt.
//    SCI_disableInterrupt(SCIAase, SCI_INT_TXFF);
//
////    msg = "\r\nEnter two characters: \0";
////    SCI_writeCharArray(SCIAase, (uint16_t*)msg, 26);
//
//    // Acknowledge the PIE interrupt.
//    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);
//}

//
// SCIARxISR - Read two characters from the RXBUF and echo them back.
//
__interrupt void SCIARxISR(void)
{
    // Read characters from the FIFO.
    SCIA_RxTimeOut = 0;
    if(SCIA_RxCnt < SCIA_BufSize)
    {
        SCIA_RxBuf[SCIA_RxCnt++] = SCI_readCharBlockingFIFO(SCIAase);
    }
    SCI_clearOverflowStatus(SCIAase);
    // Clear the SCI RXFF interrupt and acknowledge the PIE interrupt.
    SCI_clearInterruptStatus(SCIAase, SCI_INT_RXFF);
    if(SCI_getInterruptStatus(SCIAase)&SCI_INT_RXERR != 0)
    {
        SCI_clearInterruptStatus(SCIAase, SCI_INT_RXERR);
        SCI_enableModule(SCIAase);
    }
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);
}



