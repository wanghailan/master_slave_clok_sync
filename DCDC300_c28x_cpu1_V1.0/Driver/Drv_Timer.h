/*
 * Drv_Timer.h
 *
 *  Created on: 2024年9月11日
 *      Author: guowei680
 */

#ifndef DRIVER_DRV_TIMER_H_
#define DRIVER_DRV_TIMER_H_

//#pragma pack(1) //设置内存对齐为：1字节对齐
typedef struct StructTimerFlag
{
    uint16_t  u16_b100us                  :1;                 //0.11ms时间到
    uint16_t  u16_b1ms                    :1;                 //1ms时间到
    uint16_t  u16_b5ms                    :1;                 //5ms时间到
    uint16_t  u16_b10ms                   :1;                 //10ms时间到

    uint16_t  u16_b50ms                   :1;                 //50ms时间到
    uint16_t  u16_b100ms                  :1;                 //100ms时间到
    uint16_t  u16_b500ms                  :1;                 //500ms时间到
    uint16_t  u16_b1s                     :1;                 //1s时间到
    uint16_t  u16_b5s                     :1;
    uint16_t  bResvd                      :7;
}Struct_TimerFlag;
//#pragma pack()

extern uint32_t m_u32_TimerCnt;
extern Struct_TimerFlag m_st_TimerFlag;
//extern uint32_t cpuTimer1IntCount;

void Drv_ClockCntInit(void);
inline void Drv_Timer_IntMaintain(void);
void  Drv_Timer_ClockMaintain(void);
void  Drv_Timer_usDelay(int32_t Count);

//void Drv_TimerInt_Init(void);

#endif /* DRIVER_DRV_TIMER_H_ */
