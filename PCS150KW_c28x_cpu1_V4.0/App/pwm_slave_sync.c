/*
 * pwm_slave_sync.c
 *
 * Slave-side PPS to PWM phase sync.
 */

#include "pwm_slave_sync.h"
#include "bsp.h"

#define PPS_INPUTXBAR_CHANNEL       XBAR_INPUT7
#define PPS_ECAP_INPUT_SEL          ECAP_INPUT_INPUTXBAR7
#define PPS_ECAP_BASE               ECAP1_BASE
#define PPS_ECAP_INT                INT_ECAP1
#define PPS_GPIO                    47U

#define SYNC_EPWM_BASE              EPWM1_BASE

#define EPWM_TBCLK_HZ               100000000UL
#define ECAP_TSCTR_HZ               200000000UL

#define PWM_MAX_STEP_NS             100L
#define PWM_LOCK_THRESHOLD_NS       1000L
#define PPS_ISR_MAX_LATENCY_NS      5000UL
#define PPS_PHASE_OFFSET_NS         0L

#define PPS_ECAP_ALL_INT_FLAGS      (ECAP_ISR_SOURCE_CAPTURE_EVENT_1 | \
                                     ECAP_ISR_SOURCE_CAPTURE_EVENT_2 | \
                                     ECAP_ISR_SOURCE_CAPTURE_EVENT_3 | \
                                     ECAP_ISR_SOURCE_CAPTURE_EVENT_4 | \
                                     ECAP_ISR_SOURCE_COUNTER_OVERFLOW | \
                                     ECAP_ISR_SOURCE_COUNTER_PERIOD | \
                                     ECAP_ISR_SOURCE_COUNTER_COMPARE)

#define ETHERNET_DEBUG

extern IPC_DATA_CM2CPU Cpu1Ipc_cm2cpu;

volatile uint32_t g_slavePpsCapCount = 0U;
volatile int32_t  g_slavePwmPhaseErrTicks = 0;
volatile int32_t  g_slavePwmPhaseErrNs = 0;
volatile uint32_t g_slavePwmLockCount = 0U;
volatile uint32_t g_slavePwmSkipCount = 0U;
volatile bool     g_slavePwmLocked = false;
volatile uint16_t g_slavePwmApplySync = 1U;

static void Slave_InitEPwmSync(void);
static void Slave_InitPpsEcap(void);
static void Slave_ServicePpsCapture(bool allowSync);

static int32_t  ns_to_tbclk_ticks(int32_t ns);
static uint32_t ns_to_ecap_ticks(uint32_t ns);
static uint32_t ecap_ticks_to_tbclk(uint32_t ecapTicks);
static uint32_t get_epwm_up_down_phase(uint32_t base, uint32_t tbprd);
static void     set_epwm_up_down_phase(uint32_t base, uint32_t phase, uint32_t tbprd);
static uint32_t wrap_u32(int32_t value, uint32_t modulo);
static int32_t  signed_phase_error(uint32_t actual, uint32_t target, uint32_t modulo);
static int32_t  clamp_i32(int32_t value, int32_t minValue, int32_t maxValue);

void PWM_SlaveSync_Init(void)
{
    Slave_InitEPwmSync();
    Slave_InitPpsEcap();
}

static void Slave_InitEPwmSync(void)
{
    EALLOW;
    SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);
    EPWM_setSyncInPulseSource(SYNC_EPWM_BASE, EPWM_SYNC_IN_PULSE_SRC_DISABLE);
    EPWM_setPhaseShift(SYNC_EPWM_BASE, 0U);
    EPWM_disablePhaseShiftLoad(SYNC_EPWM_BASE);
    EPWM_setCountModeAfterSync(SYNC_EPWM_BASE, EPWM_COUNT_MODE_UP_AFTER_SYNC);
    EPWM_enableSyncOutPulseSource(SYNC_EPWM_BASE, EPWM_SYNC_OUT_PULSE_ON_CNTR_ZERO);
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);
    EDIS;
}

static void Slave_InitPpsEcap(void)
{
    EALLOW;
    XBAR_setInputPin(INPUTXBAR_BASE, PPS_INPUTXBAR_CHANNEL, PPS_GPIO);
    ECAP_selectECAPInput(PPS_ECAP_BASE, PPS_ECAP_INPUT_SEL);

    Interrupt_register(PPS_ECAP_INT, &PPS_Slave_ECAP_ISR);

    ECAP_disableInterrupt(PPS_ECAP_BASE, PPS_ECAP_ALL_INT_FLAGS);
    ECAP_stopCounter(PPS_ECAP_BASE);
    ECAP_enableCaptureMode(PPS_ECAP_BASE);
    ECAP_setCaptureMode(PPS_ECAP_BASE, ECAP_CONTINUOUS_CAPTURE_MODE, ECAP_EVENT_1);
    ECAP_setEventPolarity(PPS_ECAP_BASE, ECAP_EVENT_1, ECAP_EVNT_RISING_EDGE);
    ECAP_disableCounterResetOnEvent(PPS_ECAP_BASE, ECAP_EVENT_1);
    ECAP_enableTimeStampCapture(PPS_ECAP_BASE);

    ECAP_clearInterrupt(PPS_ECAP_BASE,
                        ECAP_ISR_SOURCE_CAPTURE_EVENT_1 |
                        ECAP_ISR_SOURCE_COUNTER_OVERFLOW);
    ECAP_clearGlobalInterrupt(PPS_ECAP_BASE);
    ECAP_enableInterrupt(PPS_ECAP_BASE, ECAP_ISR_SOURCE_CAPTURE_EVENT_1);

    ECAP_startCounter(PPS_ECAP_BASE);
    ECAP_reArm(PPS_ECAP_BASE);
    Interrupt_enable(PPS_ECAP_INT);
    EDIS;
}

__interrupt void PPS_Slave_ECAP_ISR(void)
{
    Slave_ServicePpsCapture(g_slavePwmApplySync != 0U);

#ifdef ETHERNET_DEBUG
    Drv_Led_toggle(PWM_SYNC_PPS_CAPTURE_LED_CH);
#endif

    ECAP_clearInterrupt(PPS_ECAP_BASE, ECAP_ISR_SOURCE_CAPTURE_EVENT_1);
    ECAP_clearGlobalInterrupt(PPS_ECAP_BASE);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP4);
}

static void Slave_ServicePpsCapture(bool allowSync)
{
    uint32_t capTs;
    uint32_t nowTs;
    uint32_t dtEcap;
    uint32_t dtTbclk;
    uint32_t tbprd;
    uint32_t modulo;
    uint32_t targetAtPps;
    uint32_t targetNow;
    uint32_t actualNow;
    int32_t  err;
    int32_t  step;
    int32_t  lockThresholdTicks;
    uint32_t newPhase;

    g_slavePpsCapCount++;

    capTs = ECAP_getEventTimeStamp(PPS_ECAP_BASE, ECAP_EVENT_1);
    nowTs = ECAP_getTimeBaseCounter(PPS_ECAP_BASE);
    dtEcap = nowTs - capTs;
    dtTbclk = ecap_ticks_to_tbclk(dtEcap);

    if (Cpu1Ipc_cm2cpu.PtpSynced == 0)
    {
        g_slavePwmLocked = false;
        g_slavePwmLockCount = 0U;
        return;
    }

    if (dtEcap > ns_to_ecap_ticks(PPS_ISR_MAX_LATENCY_NS))
    {
        g_slavePwmSkipCount++;
        return;
    }

    tbprd = EPWM_getTimeBasePeriod(SYNC_EPWM_BASE);
    if (tbprd == 0U)
    {
        g_slavePwmSkipCount++;
        return;
    }

    modulo = 2U * tbprd;
    targetAtPps = wrap_u32(ns_to_tbclk_ticks(PPS_PHASE_OFFSET_NS), modulo);
    targetNow = (targetAtPps + (dtTbclk % modulo)) % modulo;
    actualNow = get_epwm_up_down_phase(SYNC_EPWM_BASE, tbprd);
    err = signed_phase_error(actualNow, targetNow, modulo);

    g_slavePwmPhaseErrTicks = err;
    g_slavePwmPhaseErrNs = (int32_t)(((int64_t)err * 1000000000LL) / (int64_t)EPWM_TBCLK_HZ);

    lockThresholdTicks = ns_to_tbclk_ticks(PWM_LOCK_THRESHOLD_NS);
    if (allowSync == false)
    {
        g_slavePwmLocked = false;
        g_slavePwmLockCount = 0U;
        return;
    }

    if ((err > lockThresholdTicks) || (err < -lockThresholdTicks))
    {
        if (g_slavePwmLocked == false)
        {
            newPhase = targetNow;
        }
        else
        {
            step = clamp_i32(err,
                             -ns_to_tbclk_ticks(PWM_MAX_STEP_NS),
                              ns_to_tbclk_ticks(PWM_MAX_STEP_NS));
            newPhase = wrap_u32((int32_t)actualNow - step, modulo);
        }

        set_epwm_up_down_phase(SYNC_EPWM_BASE, newPhase, tbprd);
        g_slavePwmLocked = false;
        g_slavePwmLockCount = 0U;
    }
    else
    {
        if (g_slavePwmLockCount < 0xFFFFFFFFU)
        {
            g_slavePwmLockCount++;
        }
        if (g_slavePwmLockCount >= 3U)
        {
            g_slavePwmLocked = true;
        }
    }
}

static int32_t ns_to_tbclk_ticks(int32_t ns)
{
    int64_t ticks = ((int64_t)ns * (int64_t)EPWM_TBCLK_HZ) / 1000000000LL;
    return (int32_t)ticks;
}

static uint32_t ns_to_ecap_ticks(uint32_t ns)
{
    return (uint32_t)(((uint64_t)ns * (uint64_t)ECAP_TSCTR_HZ) / 1000000000ULL);
}

static uint32_t ecap_ticks_to_tbclk(uint32_t ecapTicks)
{
    return (uint32_t)(((uint64_t)ecapTicks * (uint64_t)EPWM_TBCLK_HZ) /
                      (uint64_t)ECAP_TSCTR_HZ);
}

static uint32_t get_epwm_up_down_phase(uint32_t base, uint32_t tbprd)
{
    uint32_t ctr = (uint32_t)EPWM_getTimeBaseCounterValue(base);
    uint32_t modulo = 2U * tbprd;
    uint32_t phase;

    if (EPWM_getTimeBaseCounterDirection(base) == EPWM_TIME_BASE_STATUS_COUNT_UP)
    {
        phase = ctr;
    }
    else
    {
        phase = modulo - ctr;
    }

    return phase % modulo;
}

static void set_epwm_up_down_phase(uint32_t base, uint32_t phase, uint32_t tbprd)
{
    uint32_t modulo = 2U * tbprd;
    uint32_t wrappedPhase = phase % modulo;
    uint32_t tbphs;

    if (wrappedPhase <= tbprd)
    {
        tbphs = wrappedPhase;
        EPWM_setCountModeAfterSync(base, EPWM_COUNT_MODE_UP_AFTER_SYNC);
    }
    else
    {
        tbphs = modulo - wrappedPhase;
        EPWM_setCountModeAfterSync(base, EPWM_COUNT_MODE_DOWN_AFTER_SYNC);
    }

    EPWM_setPhaseShift(base, (uint16_t)tbphs);
    EPWM_enablePhaseShiftLoad(base);
    EPWM_forceSyncPulse(base);
}

static uint32_t wrap_u32(int32_t value, uint32_t modulo)
{
    int32_t m = (int32_t)modulo;
    int32_t r = value % m;

    if (r < 0)
    {
        r += m;
    }

    return (uint32_t)r;
}

static int32_t signed_phase_error(uint32_t actual, uint32_t target, uint32_t modulo)
{
    int32_t err = (int32_t)actual - (int32_t)target;
    int32_t half = (int32_t)(modulo / 2U);

    if (err > half)
    {
        err -= (int32_t)modulo;
    }
    else if (err < -half)
    {
        err += (int32_t)modulo;
    }

    return err;
}

static int32_t clamp_i32(int32_t value, int32_t minValue, int32_t maxValue)
{
    if (value > maxValue)
    {
        return maxValue;
    }

    if (value < minValue)
    {
        return minValue;
    }

    return value;
}
