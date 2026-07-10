/*
 * initial_CLA.c
 *
 *  Created on: 2024年9月16日
 *      Author: guowei680
 */


#include "initial_CLA.h"

#pragma DATA_SECTION(tClaPrivate,"CLADataLS1");
volatile T_CLA_PRIVATE_VARIABLE tClaPrivate;

#pragma DATA_SECTION(tCpu2Cla,"CpuToCla1MsgRAM");
volatile T_CPU2CLA_VARIABLE tCpu2Cla;

#pragma DATA_SECTION(tCla2Cpu,"Cla1ToCpuMsgRAM");
volatile T_CLA2CPU_VARIABLE tCla2Cpu;


__interrupt void cla1Isr1();
__interrupt void cla1Isr2();
__interrupt void cla1Isr3();
__interrupt void cla1Isr4();
__interrupt void cla1Isr5();
__interrupt void cla1Isr6();
__interrupt void cla1Isr7();
__interrupt void cla1Isr8();


//
// configClaMemory - Configure CLA memory
//
void configClaMemory(void)
{
    //
    // Sync with CPU1 using IPC flag 31
    //
//    IPC_sync(IPC_CPU2_L_CPU1_R, IPC_FLAG31);

#ifdef _FLASH
    extern uint32_t Cla1funcsRunStart, Cla1funcsLoadStart, Cla1funcsLoadSize;
    //
    // Copy over code from FLASH to RAM
    //
    memcpy((uint32_t *)&Cla1funcsRunStart, (uint32_t *)&Cla1funcsLoadStart,
           (uint32_t)&Cla1funcsLoadSize);
#endif //_FLASH

#ifdef CPU2
    //
    // Enable CPU2 clocking at the sys clock level
    //
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_CLA1);
#endif //CPU2

    //
    // Initialize and wait for CLA1ToCPUMsgRAM
    //
    MemCfg_initSections(MEMCFG_SECT_MSGCLA1TOCPU);
    while(!MemCfg_getInitStatus(MEMCFG_SECT_MSGCLA1TOCPU))
    {
    }

    //
    // Initialize and wait for CPUToCLA1MsgRAM
    //
    MemCfg_initSections(MEMCFG_SECT_MSGCPUTOCLA1);
    while(!MemCfg_getInitStatus(MEMCFG_SECT_MSGCPUTOCLA1))
    {
    }

    //
    // Select LS5RAM to be the programming space for the CLA
    // First configure the CLA to be the master for LS5 and then
    // set the space to be a program block
    //
    MemCfg_setLSRAMMasterSel(MEMCFG_SECT_LS5, MEMCFG_LSRAMMASTER_CPU_CLA1);
    MemCfg_setCLAMemType(MEMCFG_SECT_LS5, MEMCFG_CLA_MEM_PROGRAM);

    MemCfg_setLSRAMMasterSel(MEMCFG_SECT_LS6, MEMCFG_LSRAMMASTER_CPU_CLA1);
    MemCfg_setCLAMemType(MEMCFG_SECT_LS6, MEMCFG_CLA_MEM_PROGRAM);

    MemCfg_setLSRAMMasterSel(MEMCFG_SECT_LS7, MEMCFG_LSRAMMASTER_CPU_CLA1);
    MemCfg_setCLAMemType(MEMCFG_SECT_LS7, MEMCFG_CLA_MEM_PROGRAM);

    //
    // Next configure LS0RAM and LS1RAM as data spaces for the CLA
    // First configure the CLA to be the master for LS0(1) and then
    // set the spaces to be code blocks
    //
    MemCfg_setLSRAMMasterSel(MEMCFG_SECT_LS0, MEMCFG_LSRAMMASTER_CPU_CLA1);
    MemCfg_setCLAMemType(MEMCFG_SECT_LS0, MEMCFG_CLA_MEM_DATA);

    MemCfg_setLSRAMMasterSel(MEMCFG_SECT_LS1, MEMCFG_LSRAMMASTER_CPU_CLA1);
    MemCfg_setCLAMemType(MEMCFG_SECT_LS1, MEMCFG_CLA_MEM_DATA);
}

//
// initCpu2Cla1 - Initialize CLA1 task vectors and end of task ISRs
//
void initCpu2Cla1(void)
{
    //
    // Compute all CLA task vectors
    // On Type-1 CLAs the MVECT registers accept full 16-bit task addresses as
    // opposed to offsets used on older Type-0 CLAs
    //
    // Adding a pragma to suppress the warnings when casting the CLA tasks'
    // function pointers to uint16_t. Compiler believes this to be improper
    // although the CLA only has 16-bit addressing.
    //
#pragma diag_suppress=770
    CLA_mapTaskVector(CLA1_BASE, CLA_MVECT_1, (uint16_t)&Cla1Task1);
    CLA_mapTaskVector(CLA1_BASE, CLA_MVECT_2, (uint16_t)&Cla1Task2);
    CLA_mapTaskVector(CLA1_BASE, CLA_MVECT_3, (uint16_t)&Cla1Task3);
    CLA_mapTaskVector(CLA1_BASE, CLA_MVECT_4, (uint16_t)&Cla1Task4);
    CLA_mapTaskVector(CLA1_BASE, CLA_MVECT_5, (uint16_t)&Cla1Task5);
    CLA_mapTaskVector(CLA1_BASE, CLA_MVECT_6, (uint16_t)&Cla1Task6);
    CLA_mapTaskVector(CLA1_BASE, CLA_MVECT_7, (uint16_t)&Cla1Task7);
    CLA_mapTaskVector(CLA1_BASE, CLA_MVECT_8, (uint16_t)&Cla1Task8);
#pragma diag_warning=770

    //
    // Enable the IACK instruction to start a task on CLA in software
    // for all  8 CLA tasks. Also, globally enable all 8 tasks (or a
    // subset of tasks) by writing to their respective bits in the
    // MIER register
    //
    CLA_enableIACK(CLA1_BASE);
    CLA_enableTasks(CLA1_BASE, 0x00FF);

    //
    // Configure the vectors for the end-of-task interrupt for all
    // 8 tasks
    //
    Interrupt_register(INT_CLA1_1, cla1Isr1);
    Interrupt_register(INT_CLA1_2, cla1Isr2);
    Interrupt_register(INT_CLA1_3, cla1Isr3);
    Interrupt_register(INT_CLA1_4, cla1Isr4);
    Interrupt_register(INT_CLA1_5, cla1Isr5);
    Interrupt_register(INT_CLA1_6, cla1Isr6);
    Interrupt_register(INT_CLA1_7, cla1Isr7);
    Interrupt_register(INT_CLA1_8, cla1Isr8);

    //
    // Enable CLA interrupts at the group and subgroup levels
    //
    Interrupt_enable(INT_CLA1_1);
    Interrupt_enable(INT_CLA1_2);
    Interrupt_enable(INT_CLA1_3);
    Interrupt_enable(INT_CLA1_4);
    Interrupt_enable(INT_CLA1_5);
    Interrupt_enable(INT_CLA1_6);
    Interrupt_enable(INT_CLA1_7);
    Interrupt_enable(INT_CLA1_8);
    IER |= INTERRUPT_CPU_INT11;
}

//
// CLA Initialization
//
void initial_CLA(void)
{
    configClaMemory();
    initCpu2Cla1();
    CLA_setTriggerSource(CLA_TASK_1,CLA_TRIGGER_ADCA1);//ClaTask1触发源设置为ADCA1
    CLA_setTriggerSource(CLA_TASK_2,CLA_TRIGGER_SOFTWARE);
    CLA_setTriggerSource(CLA_TASK_3,CLA_TRIGGER_SOFTWARE);
    CLA_setTriggerSource(CLA_TASK_4,CLA_TRIGGER_SOFTWARE);
    CLA_setTriggerSource(CLA_TASK_5,CLA_TRIGGER_SOFTWARE);
    CLA_setTriggerSource(CLA_TASK_6,CLA_TRIGGER_SOFTWARE);
    CLA_setTriggerSource(CLA_TASK_7,CLA_TRIGGER_SOFTWARE);
    CLA_setTriggerSource(CLA_TASK_8,CLA_TRIGGER_SOFTWARE);
    CLA_forceTasks(CLA1_BASE, CLA_TASKFLAG_8);//软件触发一次ClaTask8,执行CLA数据初始化
}

//
// cla1Isr1 - CLA1 ISR 1
//
__interrupt void cla1Isr1 ()
{
    //
    // Acknowledge the end-of-task interrupt for task 1
    //
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP11);
//    asm(" ESTOP0");
}

//
// cla1Isr2 - CLA1 ISR 2
//
__interrupt void cla1Isr2 ()
{
    asm(" ESTOP0");
}

//
// cla1Isr3 - CLA1 ISR 3
//
__interrupt void cla1Isr3 ()
{
    asm(" ESTOP0");
}

//
// cla1Isr4 - CLA1 ISR 4
//
__interrupt void cla1Isr4 ()
{
    asm(" ESTOP0");
}

//
// cla1Isr5 - CLA1 ISR 5
//
__interrupt void cla1Isr5 ()
{
    asm(" ESTOP0");
}

//
// cla1Isr6 - CLA1 ISR 6
//
__interrupt void cla1Isr6 ()
{
    asm(" ESTOP0");
}

//
// cla1Isr7 - CLA1 ISR 7
//
__interrupt void cla1Isr7 ()
{
    asm(" ESTOP0");
}

//
// cla1Isr8 - CLA1 ISR 8
//
__interrupt void cla1Isr8 ()
{
    //
    // Acknowledge the end-of-task interrupt for task 8
    //
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP11);
//    asm(" ESTOP0");
}




