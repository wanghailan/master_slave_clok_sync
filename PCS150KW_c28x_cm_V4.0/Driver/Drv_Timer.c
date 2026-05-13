//#############################################################################
//
// FILE:   Drv_Timer.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

#include <Driver/device/cm.h>
#include <Driver/device/driverlib_cm.h>
#include "bsp.h"

#define FcycleTime          50                             //50us中断1次

#define Fint                0x100000000                     //10000  int frequency is 10K,the times of enter int in 1s
#define K100msCnst          (100000/FcycleTime)             //0.1*20000 = 2000 20khz中断频率 50us中断一次
#define K50msCnst           (50000/FcycleTime)              //0.04*20000 = 800
#define K10msCnst           (10000/FcycleTime)              //0.01*10000 = 200
#define K5msCnst            (5000/FcycleTime)               //0.005*10000 = 100
#define K1msCnst            (1000/FcycleTime)               //0.001*10000 = 20

static uint32_t m_u32_TimerCnt;
static uint32_t m_u32_Clk1msCnt;
static uint32_t m_u32_Clk5msCnt;
static uint32_t m_u32_Clk10msCnt;
static uint32_t m_u32_Clk50msCnt;
static uint32_t m_u32_Clk100msCnt;
static uint32_t m_u32_Clk500msCnt;
static uint32_t m_u32_Clk1sCnt;
Struct_TimerFlag m_st_TimerFlag;

//uint32_t cpuTimer1IntCount;
//extern int16_t     SCIC_RxTimeOut;

static void initCPUTimer1(void);
static void configCPUTimer(uint32_t cpuTimer, uint32_t period);
__interrupt void cpuTimer1ISR(void);

void Drv_ClockCntInit(void)
{
    m_u32_TimerCnt      = 0;    //中断中计数器，每次中断+1

    m_u32_Clk1msCnt     = 0;    //1ms定时用
    m_u32_Clk5msCnt     = 0;    //5ms定时用
    m_u32_Clk10msCnt    = 0;    //10ms定时用
    m_u32_Clk50msCnt    = 0;    //50ms定时用
    m_u32_Clk100msCnt   = 0;    //100ms定时用
    m_u32_Clk500msCnt   = 0;    //500ms定时用
    m_u32_Clk1sCnt      = 0;    //1s定时用

    m_st_TimerFlag.u16_b100us   = 0;    //请0.1ms定时标志
    m_st_TimerFlag.u16_b1ms     = 0;    //请1ms定时标志
    m_st_TimerFlag.u16_b5ms     = 0;    //清5ms定时标志
    m_st_TimerFlag.u16_b10ms    = 0;    //清10ms定时标志
    m_st_TimerFlag.u16_b50ms    = 0;    //清40ms定时标志
    m_st_TimerFlag.u16_b100ms   = 0;    //清100ms定时标志
    m_st_TimerFlag.u16_b500ms   = 0;    //清500ms定时标志
    m_st_TimerFlag.u16_b1s      = 0;
}

//中断时钟维护,在Timer中断中执行,100us运行一次
inline void Drv_Timer_IntMaintain(void)
{
    m_u32_TimerCnt++;

    if(m_u32_TimerCnt >= Fint) //2^32 4.95 days
    {
        m_u32_TimerCnt = 0;
    }
}


//时钟维护
void  Drv_Timer_ClockMaintain(void)
{
    /*******************************0.1ms*********************************/
//        if(m_u32_Clk100usCnt > m_u32_TimerCnt) //预防m_u32_TimerCnt到最大反转
//            m_u32_Clk100usCnt = m_u32_TimerCnt;
//        if ((m_u32_TimerCnt - m_u32_Clk100usCnt) >= K100usCnst )
//        {
//            m_u32_Clk100usCnt = m_u32_TimerCnt;
//            m_st_TimerFlag.u16_b100us = 1;                           //置0.1ms定时标志
//        }
//        else
//           m_st_TimerFlag.u16_b100us = 0;                           //清0.1ms定时标志
/*******************************1ms*********************************/
    if(m_u32_Clk1msCnt > m_u32_TimerCnt) //预防m_u32_TimerCnt到最大反转
       m_u32_Clk1msCnt = m_u32_TimerCnt;
    if ((m_u32_TimerCnt - m_u32_Clk1msCnt) >= K1msCnst )
    {
       m_u32_Clk1msCnt = m_u32_TimerCnt;
       m_st_TimerFlag.u16_b1ms = 1;                           //置1ms定时标志
    }
    else
       m_st_TimerFlag.u16_b1ms = 0;                           //清1ms定时标志
/*******************************5ms*********************************/
    if(m_u32_Clk5msCnt > m_u32_TimerCnt) //预防m_u32_TimerCnt到最大反转
            m_u32_Clk5msCnt = m_u32_TimerCnt;
    if ((m_u32_TimerCnt - m_u32_Clk5msCnt) >= K5msCnst )
    {
        m_u32_Clk5msCnt = m_u32_TimerCnt;
        m_st_TimerFlag.u16_b5ms = 1;                           //置5ms定时标志
    }
    else
        m_st_TimerFlag.u16_b5ms = 0;                           //清5ms定时标志
/*******************************10ms*********************************/
    if(m_u32_Clk10msCnt > m_u32_TimerCnt) //预防m_u32_TimerCnt到最大反转
       m_u32_Clk10msCnt = m_u32_TimerCnt;
    if ((m_u32_TimerCnt - m_u32_Clk10msCnt) >= K10msCnst )
    {
       m_u32_Clk10msCnt = m_u32_TimerCnt;
       m_st_TimerFlag.u16_b10ms = 1;                           //置10ms定时标志
    }
    else
       m_st_TimerFlag.u16_b10ms = 0;                           //清10ms定时标志
/*******************************50ms*********************************/
    if(m_u32_Clk50msCnt > m_u32_TimerCnt) //预防m_u32_TimerCnt到最大反转
            m_u32_Clk50msCnt = m_u32_TimerCnt;
    if ((m_u32_TimerCnt - m_u32_Clk50msCnt) >= K50msCnst )
    {
        m_u32_Clk50msCnt = m_u32_TimerCnt;
        m_st_TimerFlag.u16_b50ms = 1;                           //置50ms定时标志
    }
    else
        m_st_TimerFlag.u16_b50ms = 0;                           //清50ms定时标志
/*******************************100ms*********************************/
    if(m_u32_Clk100msCnt > m_u32_TimerCnt) //预防m_u32_TimerCnt到最大反转
        m_u32_Clk100msCnt = m_u32_TimerCnt;
    if ((m_u32_TimerCnt - m_u32_Clk100msCnt) >= K100msCnst )
    {
        m_u32_Clk100msCnt = m_u32_TimerCnt;
        m_st_TimerFlag.u16_b100ms = 1;                          //置100ms定时标志
    }
    else
        m_st_TimerFlag.u16_b100ms = 0;                          //清100ms定时标志

    if(m_st_TimerFlag.u16_b100ms==1)
    {
        m_u32_Clk500msCnt++;
        m_u32_Clk1sCnt++;
    }

    if(m_u32_Clk500msCnt>=5)
    {
        m_st_TimerFlag.u16_b500ms= 1;                          //置0.5秒钟定时标志
        m_u32_Clk500msCnt=0;
    }
    else
        m_st_TimerFlag.u16_b500ms= 0;                          //清0.5秒钟定时标志

    if(m_u32_Clk1sCnt>=10)
    {
        m_st_TimerFlag.u16_b1s= 1;                          //置1秒钟定时标志
        m_u32_Clk1sCnt=0;
    }
    else
        m_st_TimerFlag.u16_b1s= 0;                          //清1秒钟定时标志
}

void    Drv_Timer_usDelay(int32_t Count)
{
    while(Count>0)
    {
        Count--;
//        asm(" RPT #100 || NOP");//100MHZ
        DEVICE_DELAY_US(1);
    }
}


void Drv_TimerInt_Init(void)
{
      // ISRs for each CPU Timer interrupt
      Interrupt_registerHandler(INT_TIMER1, &cpuTimer1ISR);

      // Initializes the Device Peripheral. For this example, only initialize the Cpu Timers.
      initCPUTimer1();

      // Configure CPU-Timer 0, 1, and 2 to interrupt every 1, 2, 4 seconds:
      // 1, 2, 4 Period respectively (in uSeconds)
      configCPUTimer(CPUTIMER1_BASE, FcycleTime);

      // To ensure precise timing, use write-only instructions to write to the
      // entire register. Therefore, if any of the configuration bits are changed
      // in configCPUTimer and initCPUTimers, the below settings must also be updated.
      CPUTimer_enableInterrupt(CPUTIMER1_BASE);

      // Enables CPU int1, int13, and int14 which are connected to CPU-Timer 0,
      // CPU-Timer 1, and CPU-Timer 2 respectively.
      // Enable TINT0 in the PIE: Group 1 interrupt 7
      Interrupt_enable(INT_TIMER1);

      // Starts CPU-Timer 1
      CPUTimer_startTimer(CPUTIMER1_BASE);
}

static void initCPUTimer1(void)
{
    // Initialize timer period to maximum
    CPUTimer_setPeriod(CPUTIMER1_BASE, 0xFFFFFFFF);

    // Initialize pre-scale counter to divide by 1 (SYSCLKOUT)
    CPUTimer_setPreScaler(CPUTIMER1_BASE, 0);

    // Make sure timer is stopped
    CPUTimer_stopTimer(CPUTIMER1_BASE);

    // Reload all counter register with period value
    CPUTimer_reloadTimerCounter(CPUTIMER1_BASE);

//    // Reset interrupt counter
//    cpuTimer1IntCount = 0;
}


static void configCPUTimer(uint32_t cpuTimer, uint32_t period)
{
    uint32_t temp, freq = CM_CLK_FREQ;

    // Initialize timer period:
    temp = ((freq / 1000000) * period);
    CPUTimer_setPeriod(cpuTimer, temp);

    // Set pre-scale counter to divide by 1 (SYSCLKOUT):
    CPUTimer_setPreScaler(cpuTimer, 0);

    // Initializes timer control register. The timer is stopped, reloaded,
    // free run disabled, and interrupt enabled.
    // Additionally, the free and soft bits are set
    CPUTimer_stopTimer(cpuTimer);
    CPUTimer_reloadTimerCounter(cpuTimer);
    CPUTimer_setEmulationMode(cpuTimer, CPUTIMER_EMULATIONMODE_STOPAFTERNEXTDECREMENT);
    CPUTimer_enableInterrupt(cpuTimer);

    // Resets interrupt counters for the three cpuTimers
//    if(cpuTimer == CPUTIMER1_BASE)
//        cpuTimer1IntCount = 0;
}


//// cpuTimer0ISR - Counter for CpuTimer0
//__interrupt void cpuTimer0ISR(void)
//{
//    cpuTimer0IntCount++;
//}

// cpuTimer1ISR - Counter for CpuTimer1
__interrupt void cpuTimer1ISR(void)
{
//    cpuTimer1IntCount++;
    Drv_Timer_IntMaintain();
    Pcs_CmRunISR();
//    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}


