//#############################################################################
//
// FILE:   Drv_SCI.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

#include <Driver/device/device.h>
#include <Driver/device/driverlib.h>
#include <Drv_SCIB.h>
#include "bsp.h"


#define  SCIBase    SCIB_BASE


uint16_t    SCIB_RxBuf[SCIB_BufSize] = {0};
int16_t     SCIB_RxCnt = 0;
int32_t     SCIB_RxTimeOut = 0;

//static int16_t  Igbt_id;
//static int16_t  Igbt1_st,Igbt2_st;
//static int16_t  Igbt1_Temp,Igbt2_Temp,Igbt3_Temp,Igbt4_Temp;


static void Drv_SCIB_Init(uint32_t ulBaudRate, uint16_t ucDataBits, uint16_t ucParity);
//__interrupt void SCIBTxISR(void);
__interrupt void SCIBRxISR(void);

//*****************************************************************************
//
// SCI Configurations
//
//*****************************************************************************
void Drv_SCIBInit(void)
{
    Drv_SCIB_Init(9600,8,0);
    SCIB_TxEn();
    SCIB_RxEn();
    SCIB_RxCnt = 0;
    SCIB_RxTimeOut = 0;
}


/**********************************************************************************
  函数名称：Drv_SCI_WriteByte()
  功能描述：SCI发送
  修改日期：2018-11-14
***********************************************************************************/
void   Drv_SCIB_WriteByte(uint16_t dat)
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
void   Drv_SCIB_WriteDataBuf(int16_t dLen,uint16_t *dBuf)
{
//    SCIB_TxEn();//发送
    SCI_writeCharArray(SCIBase,dBuf,dLen);
    while(SCI_getTxFIFOStatus(SCIBase) != SCI_FIFO_TX0);//等待FIFO发送完
    while(SCI_isTransmitterBusy(SCIBase) == true);//等待发送器空标志
//    DEVICE_DELAY_US(300);
//    SCIB_RxEn();//转接收
}


//*****************************************************************************
//
// PINMUX Configurations
//
//*****************************************************************************
void Drv_SCIBPin_Init(void)
{
    // SCIB ->Pinmux
    GPIO_setPinConfig(SCIB_SCIRX_PIN_CONFIG);
    GPIO_setDirectionMode(SCIB_SCIRX_GPIO, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(SCIB_SCIRX_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(SCIB_SCIRX_GPIO, GPIO_QUAL_ASYNC);

    GPIO_setPinConfig(SCIB_SCITX_PIN_CONFIG);
    GPIO_setDirectionMode(SCIB_SCITX_GPIO, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(SCIB_SCITX_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(SCIB_SCITX_GPIO, GPIO_QUAL_ASYNC);
    //SCIB_DE
    GPIO_setPinConfig(SCIB_DE_PIN_CONFIG);
    GPIO_setDirectionMode(SCIB_GPIO_PIN_DE, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(SCIB_GPIO_PIN_DE, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(SCIB_GPIO_PIN_DE, GPIO_DIR_MODE_OUT);
    //SCIB_RE
    GPIO_setPinConfig(SCIB_RE_PIN_CONFIG);
    GPIO_setDirectionMode(SCIB_GPIO_PIN_RE, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(SCIB_GPIO_PIN_RE, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(SCIB_GPIO_PIN_RE, GPIO_DIR_MODE_OUT);
}

static void Drv_SCIB_Init(uint32_t ulBaudRate, uint16_t ucDataBits, uint16_t ucParity)
{
    uint32_t SCIBonfig;

    SCIBonfig = 0;
    switch(ucDataBits)
    {
        case 8: SCIBonfig |= SCI_CONFIG_WLEN_8;break;
        case 7: SCIBonfig |= SCI_CONFIG_WLEN_7;break;
        case 6: SCIBonfig |= SCI_CONFIG_WLEN_6;break;
        case 5: SCIBonfig |= SCI_CONFIG_WLEN_5;break;
        default:SCIBonfig |= SCI_CONFIG_WLEN_8;break;
    };
    SCIBonfig |= SCI_CONFIG_STOP_ONE;

    //EPARITY SETTINGS ----------------------------------------------
    switch(ucParity){
        case 2:  SCIBonfig |= SCI_CONFIG_PAR_EVEN; break;
        case 1:  SCIBonfig |= SCI_CONFIG_PAR_ODD; break;
        case 0:  SCIBonfig |= SCI_CONFIG_PAR_NONE; break;
        default: SCIBonfig |= SCI_CONFIG_PAR_NONE; break;
    }

    // Initialize SCIB and its FIFO.
    SCI_clearInterruptStatus(SCIBase, SCI_INT_RXFF | SCI_INT_TXFF | SCI_INT_FE | SCI_INT_OE | SCI_INT_PE | SCI_INT_RXERR | SCI_INT_RXRDY_BRKDT | SCI_INT_TXRDY);
    SCI_clearOverflowStatus(SCIBase);
    SCI_resetChannels(SCIBase);
    SCI_setConfig(SCIBase, DEVICE_LSPCLK_FREQ, ulBaudRate, SCIBonfig);
    SCI_enableModule(SCIBase);
    SCI_resetChannels(SCIBase);
    SCI_enableFIFO(SCIBase);

    // Map the ISR to the wake interrupt.
//    Interrupt_register(INT_SCIB_TX, &SCIBTxISR);
    Interrupt_register(INT_SCIB_RX, &SCIBRxISR);
    // Enable the TXFF and RXFF interrupts.
    SCI_enableInterrupt(SCIBase, SCI_INT_RXFF);
    SCI_disableInterrupt(SCIBase, SCI_INT_RXERR);

    // Set the transmit FIFO level to 0 and the receive FIFO level to 2.
    SCI_setFIFOInterruptLevel(SCIBase, SCI_FIFO_TX0, SCI_FIFO_RX1);
    SCI_performSoftwareReset(SCIBase);
    SCI_resetTxFIFO(SCIBase);
    SCI_resetRxFIFO(SCIBase);
    // Enable the interrupts in the PIE: Group 8 interrupts 1 & 2.
    Interrupt_enable(INT_SCIB_RX);
//    Interrupt_enable(INT_SCIB_TX);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);
}

//
// SCIBTxISR - Disable the TXFF interrupt and print message asking
//             for two characters.
//
//__interrupt void SCIBTxISR(void)
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

int16_t  check_Sum = 0;
void Fiber_DatRx(uint16_t _rx)
{
//    switch(SCIB_RxCnt)
//    {
//        case 0:
//            check_Sum = 0;
//            if(_rx == 0xAA)
//               SCIB_RxCnt = 1;
//            else
//               SCIB_RxCnt = 0;
//            break;
//        case 1:
//            SCIB_RxCnt = 2;
//            check_Sum += _rx;
//            if(_rx == 0x00)
//                Igbt_id = 0;
//            else if(_rx == 0x01)
//                Igbt_id = 1;
//            else
//                SCIB_RxCnt = 0;
//            break;
//        case 2:
//            SCIB_RxCnt = 3;
//            check_Sum += _rx;
//            if(Igbt_id == 0)
//                Igbt1_st = _rx;
//            else if(Igbt_id == 1)
//                Igbt2_st = _rx;
//            else
//                SCIB_RxCnt = 0;
//            break;
//        case 3:
//            SCIB_RxCnt = 4;
//            check_Sum += _rx;
//            if(Igbt_id == 0)
//                Igbt1_Temp = (_rx<<8);
//            else if(Igbt_id == 1)
//                Igbt3_Temp = (_rx<<8);
//            else
//                SCIB_RxCnt = 0;
//            break;
//        case 4:
//            SCIB_RxCnt = 5;
//            check_Sum += _rx;
//            if(Igbt_id == 0)
//                Igbt1_Temp += _rx;
//            else if(Igbt_id == 1)
//                Igbt3_Temp += _rx;
//            else
//                SCIB_RxCnt = 0;
//            break;
//        case 5:
//            SCIB_RxCnt = 6;
//            check_Sum += _rx;
//            if(Igbt_id == 0)
//                Igbt2_Temp = (_rx<<8);
//            else if(Igbt_id == 1)
//                Igbt4_Temp = (_rx<<8);
//            else
//                SCIB_RxCnt = 0;
//            break;
//        case 6:
//            SCIB_RxCnt = 7;
//            check_Sum += _rx;
//            if(Igbt_id == 0)
//                Igbt2_Temp += _rx;
//            else if(Igbt_id == 1)
//                Igbt4_Temp += _rx;
//            else
//                SCIB_RxCnt = 0;
//            break;
//        case 7:
//            SCIB_RxCnt = 8;
//            if((check_Sum&0xFF) != _rx)
//                SCIB_RxCnt = 0;
//            break;
//        case 8:
//            SCIB_RxCnt = 0;
//            if(_rx == 0x55)
//            {
//                DC_OutMeter.DC_IGBT_Temp1 = (float32_t)Igbt1_Temp*0.1f;
//                DC_OutMeter.DC_IGBT_Temp2 = (float32_t)Igbt2_Temp*0.1f;
//                DC_OutMeter.DC_IGBT_Temp3 = (float32_t)Igbt3_Temp*0.1f;
//                DC_OutMeter.DC_IGBT_Temp4 = (float32_t)Igbt4_Temp*0.1f;
//                if(Igbt1_st&0x20)
//                    Cpu1Ipc_cpu2cm.FaultStatus.DCDC_Fault2.tbits.bMIgbt1Fault = 1;
//                else
//                    Cpu1Ipc_cpu2cm.FaultStatus.DCDC_Fault2.tbits.bMIgbt1Fault = 0;
//                if(Igbt1_st&0x10)
//                    Cpu1Ipc_cpu2cm.FaultStatus.DCDC_Fault2.tbits.bMIgbt2Fault = 1;
//                else
//                    Cpu1Ipc_cpu2cm.FaultStatus.DCDC_Fault2.tbits.bMIgbt2Fault = 0;
//                if(Igbt2_st&0x08)
//                    Cpu1Ipc_cpu2cm.FaultStatus.DCDC_Fault2.tbits.bMIgbt3Fault = 1;
//                else
//                    Cpu1Ipc_cpu2cm.FaultStatus.DCDC_Fault2.tbits.bMIgbt3Fault = 0;
//                if(Igbt2_st&0x04)
//                    Cpu1Ipc_cpu2cm.FaultStatus.DCDC_Fault2.tbits.bMIgbt4Fault = 1;
//                else
//                    Cpu1Ipc_cpu2cm.FaultStatus.DCDC_Fault2.tbits.bMIgbt4Fault = 0;
//            }
//            break;
//        default:break;
//    }
}

//
// SCIBRxISR - Read two characters from the RXBUF and echo them back.
//

__interrupt void SCIBRxISR(void)
{
     //Read characters from the FIFO.
    SCIB_RxTimeOut = 0;
//    if(SCIB_RxCnt < SCIB_BufSize)
//    {
//        SCIB_RxBuf[SCIB_RxCnt++] = SCI_readCharBlockingFIFO(SCIBase);
//    }
    Fiber_DatRx(SCI_readCharBlockingFIFO(SCIBase));

    SCI_clearOverflowStatus(SCIBase);
    // Clear the SCI RXFF interrupt and acknowledge the PIE interrupt.
    SCI_clearInterruptStatus(SCIBase, SCI_INT_RXFF);
    if(SCI_getInterruptStatus(SCIBase)&SCI_INT_RXERR != 0)
    {
        SCI_clearInterruptStatus(SCIBase, SCI_INT_RXERR);
        SCI_enableModule(SCIBase);
    }
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);
}




