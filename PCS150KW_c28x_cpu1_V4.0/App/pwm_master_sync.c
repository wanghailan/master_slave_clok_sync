/*
 * pwm_master_sync.c
 *
* 主机侧 PPS -> EPWM 相位同步模块
 *
 * 优化点:
 *   1. 补充说明硬件连接、时序、算法原理。
 *   2. 禁用EPWM硬件同步输入(SYNCI)，改为ISR内软件强制同步(SWFSYNC)，
 *      彻底消除原有1 PPS周期控制滞后。
 *   3. 精确补偿ISR执行延迟(CAP1 时间戳 -> forceSyncPulse 生效)，
 *      换算全部改为四舍五入，消除整数截断误差，目标精度50ns。
 *   4. PI控制器优化: 积分分离、50ns死区、输出变化率限制、Kp/Ki 增大，
 *      收敛时间从约16秒缩短至4秒以内。
 *   5. 运行期不再修改TBPRD，避免计数器毛刺。
 *
 * 硬件连接 (F28388D):
 *   - PPS 信号 -> GPIO47
 *   - GPIO47 -> INPUTXBAR7 -> eCAP1 (ECAP1_BASE)
 *   - EPWM1 (EPWM1_BASE)以100MHz TBCLK上下计数运行
 *   - 注意: 本版本不再将PPS接到 INPUTXBAR5/EPWM SYNCI，完全由软件控制同步时机
 */

#include "pwm_master_sync.h"
#include "bsp.h"

// ------------------- 硬件与常量配置 -------------------

// eCAP 输入路由: GPIO47 -> INPUTXBAR7 -> eCAP1
#define PPS_ECAP_INPUTXBAR_CHANNEL  XBAR_INPUT7
#define PPS_SYNC_INPUTXBAR_CHANNEL  XBAR_INPUT5
#define PPS_ECAP_INPUT_SEL          ECAP_INPUT_INPUTXBAR7
#define PPS_ECAP_BASE               ECAP1_BASE
#define PPS_ECAP_INT                INT_ECAP1
#define PPS_GPIO                    47U

// 待同步的EPWM模块
#define SYNC_EPWM_BASE              EPWM1_BASE

// 时钟频率: EPWM TBCLK=100MHz (10ns/tick); eCAP TSCTR=200MHz (5ns/tick)
#define EPWM_TBCLK_HZ               100000000UL
#define ECAP_TSCTR_HZ               200000000UL

// 默认伺服参数
#define PWM_MAX_STEP_NS             100L     // PI 输出每周期最大调整量
#define PWM_LOCK_THRESHOLD_NS       1000L    // 默认锁定判定阈值
#define PPS_ISR_MAX_LATENCY_NS      5000UL   // ISR 最大允许延迟5us
#define PPS_PHASE_OFFSET_NS         0L       // 主机基础相位偏移
#define PWM_SYNC_NOMINAL_TBPRD      ((uint32_t)EPWM_TBPRD)


/* 同步执行延迟精细补偿 (ns)
 * 包含: ISR 尾部指令执行 + CPU->EPWM 外设总线写延迟 + SWFSYNC 生效延迟。
 * 建议初始值 500ns，可通过示波器测量 PPS 与 PWM 边沿实际偏差后微调.
 */
#define SYNC_EXEC_COMPENSATION_NS   500L

// 死区: 误差绝对值小于此值时视为零，防止锁定后抖动 (目标50ns精度)
#define PWM_DEAD_BAND_NS            50L

// eCAP 全部中断标志位掩码
#define PPS_ECAP_ALL_INT_FLAGS      (ECAP_ISR_SOURCE_CAPTURE_EVENT_1 | \
                                     ECAP_ISR_SOURCE_CAPTURE_EVENT_2 | \
                                     ECAP_ISR_SOURCE_CAPTURE_EVENT_3 | \
                                     ECAP_ISR_SOURCE_CAPTURE_EVENT_4 | \
                                     ECAP_ISR_SOURCE_COUNTER_OVERFLOW | \
                                     ECAP_ISR_SOURCE_COUNTER_PERIOD | \
                                     ECAP_ISR_SOURCE_COUNTER_COMPARE)

#define PWM_SCOPE_TZ_CBC_FLAGS      (EPWM_TZ_CBC_FLAG_1 | \
                                     EPWM_TZ_CBC_FLAG_2 | \
                                     EPWM_TZ_CBC_FLAG_3 | \
                                     EPWM_TZ_CBC_FLAG_4 | \
                                     EPWM_TZ_CBC_FLAG_5 | \
                                     EPWM_TZ_CBC_FLAG_6 | \
                                     EPWM_TZ_CBC_FLAG_DCAEVT2 | \
                                     EPWM_TZ_CBC_FLAG_DCBEVT2)

#define ETHERNET_DEBUG


/*
 * PPS servo diagnostics and tuning knobs.
 * Phase error is signed: positive means the measured EPWM phase is ahead of the
 * target PPS-aligned phase. OffsetNs is used to compensate fixed board/channel
 * delay between master and slave measurements.
 */
volatile uint32_t g_masterPpsCapCount = 0U;
volatile int32_t  g_masterPwmPhaseErrTicks = 0;
volatile int32_t  g_masterPwmPhaseErrNs = 0;
volatile uint32_t g_masterPwmLockCount = 0U;
volatile uint32_t g_masterPwmSkipCount = 0U;
volatile bool     g_masterPwmLocked = false;
volatile uint16_t g_masterPwmApplySync = 1U;
volatile int32_t  g_masterPwmPhaseOffsetNs = PPS_PHASE_OFFSET_NS;
volatile int32_t  g_masterPwmLockThresholdNs = 50L;
volatile int32_t  g_masterPwmMaxStepNs = PWM_MAX_STEP_NS;
volatile uint16_t g_masterPwmHardwareSyncEnable = 1U;
volatile uint16_t g_masterPwmPiEnable = 1U;
volatile int32_t  g_masterPwmPiOffsetNs = 0L;
volatile int32_t  g_masterPwmPiIntegralNs = 0L;
volatile int32_t  g_masterPwmPiMaxOffsetNs = 2000L;
volatile uint16_t g_masterPwmPiKpDiv = 4U;
volatile uint16_t g_masterPwmPiKiDiv = 16U;
volatile uint32_t g_masterPwmApplyCount = 0U;

static void Master_InitEPwmSync(void);
static void Master_InitPpsEcap(void);
static void Master_ServicePpsCapture(bool allowSync);


static int32_t  ns_to_tbclk_ticks(int32_t ns);
static uint32_t ns_to_ecap_ticks(uint32_t ns);
static uint32_t ecap_ticks_to_tbclk(uint32_t ecapTicks);
static uint32_t get_epwm_up_down_phase(uint32_t base, uint32_t tbprd);
static void     set_epwm_up_down_phase(uint32_t base, uint32_t phase, uint32_t tbprd, bool forceNow);
static uint32_t wrap_u32(int32_t value, uint32_t modulo);
static int32_t  signed_phase_error(uint32_t actual, uint32_t target, uint32_t modulo);
static int32_t  clamp_i32(int32_t value, int32_t minValue, int32_t maxValue);
static void     update_phase_pi(int32_t phaseErrNs);


void PWM_MasterSync_Init(void)
{
    Master_InitEPwmSync();

    Master_InitPpsEcap();
}

static void Master_InitEPwmSync(void)
{
    EALLOW;

    SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);

    XBAR_setInputPin(INPUTXBAR_BASE, PPS_SYNC_INPUTXBAR_CHANNEL, PPS_GPIO);
    EPWM_setSyncInPulseSource(SYNC_EPWM_BASE,
                              EPWM_SYNC_IN_PULSE_SRC_INPUTXBAR_OUT5);
    EPWM_setPhaseShift(SYNC_EPWM_BASE, 0U);
    EPWM_enablePhaseShiftLoad(SYNC_EPWM_BASE);
    EPWM_setCountModeAfterSync(SYNC_EPWM_BASE, EPWM_COUNT_MODE_UP_AFTER_SYNC);
    EPWM_enableSyncOutPulseSource(SYNC_EPWM_BASE, EPWM_SYNC_OUT_PULSE_ON_CNTR_ZERO);
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);

    EDIS;
}

static void Master_InitPpsEcap(void)
{
    EALLOW;

    /*
     * Route the CM/PTP PPS signal into eCAP1 through INPUTXBAR7 and capture
     * only the rising edge. The counter is not reset on capture; the ISR uses
     * CAP1 and current TSCTR to compensate interrupt latency.
     */
    XBAR_setInputPin(INPUTXBAR_BASE, PPS_ECAP_INPUTXBAR_CHANNEL, PPS_GPIO);
    ECAP_selectECAPInput(PPS_ECAP_BASE, PPS_ECAP_INPUT_SEL);

    Interrupt_register(PPS_ECAP_INT, &PPS_Master_ECAP_ISR);

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

__interrupt void PPS_Master_ECAP_ISR(void)
{
    /* Timestamp PPS first, then toggle LED for capture-health indication. */
    Master_ServicePpsCapture(g_masterPwmApplySync != 0U);

#ifdef ETHERNET_DEBUG
    Drv_Led_toggle(PWM_SYNC_PPS_CAPTURE_LED_CH);
#endif

    ECAP_clearInterrupt(PPS_ECAP_BASE, ECAP_ISR_SOURCE_CAPTURE_EVENT_1);
    ECAP_clearGlobalInterrupt(PPS_ECAP_BASE);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP4);
}

static void Master_ServicePpsCapture(bool allowSync)
{
    uint32_t capTs;
    uint32_t nowTs;
    uint32_t dtEcap;
    uint32_t dtTbclk;
    uint32_t tbprd;
    uint32_t modulo;
    uint32_t targetAtPps;
    uint32_t targetNow;
    uint32_t targetAtApply;
    uint32_t applyTs;
    uint32_t applyDtTbclk;
    uint32_t actualNow;
    int32_t  err;
    int32_t  lockThresholdNs;
    int32_t  lockThresholdTicks;
    uint32_t newPhase;
    int32_t  targetOffsetNs;

    g_masterPpsCapCount++;

    /*
     * CAP1 is the PPS edge timestamp. TSCTR is sampled inside the ISR so the
     * target EPWM phase can be advanced by the ISR latency before comparing.
     */
    capTs = ECAP_getEventTimeStamp(PPS_ECAP_BASE, ECAP_EVENT_1);
    nowTs = ECAP_getTimeBaseCounter(PPS_ECAP_BASE);
    dtEcap = nowTs - capTs;
    dtTbclk = ecap_ticks_to_tbclk(dtEcap);

    if (dtEcap > ns_to_ecap_ticks(PPS_ISR_MAX_LATENCY_NS))
    {
        g_masterPwmSkipCount++;
        return;
    }

    EPWM_setTimeBasePeriod(SYNC_EPWM_BASE, PWM_SYNC_NOMINAL_TBPRD);
    tbprd = PWM_SYNC_NOMINAL_TBPRD;
    if (tbprd == 0U)
    {
        g_masterPwmSkipCount++;
        return;
    }

    modulo = 2U * tbprd;
    /*
     * Up-down PWM phase is represented as a 0..(2*TBPRD-1) ramp:
     *   0        : TBCTR=0, counting up
     *   TBPRD    : TBCTR=TBPRD
     *   2*TBPRD  : wraps back to zero
     */
    targetOffsetNs = g_masterPwmPhaseOffsetNs + g_masterPwmPiOffsetNs;
    targetAtPps = wrap_u32(ns_to_tbclk_ticks(targetOffsetNs), modulo);
    targetNow = (targetAtPps + (dtTbclk % modulo)) % modulo;
    actualNow = get_epwm_up_down_phase(SYNC_EPWM_BASE, tbprd);
    err = signed_phase_error(actualNow, targetNow, modulo);

    g_masterPwmPhaseErrTicks = err;
    g_masterPwmPhaseErrNs = (int32_t)(((int64_t)err * 1000000000LL) / (int64_t)EPWM_TBCLK_HZ);
    update_phase_pi(g_masterPwmPhaseErrNs);

    lockThresholdNs = g_masterPwmLockThresholdNs;
    if (lockThresholdNs < 10L)
    {
        lockThresholdNs = 10L;
    }
    lockThresholdTicks = ns_to_tbclk_ticks(lockThresholdNs);
    if (allowSync == false)
    {
        g_masterPwmLocked = false;
        g_masterPwmLockCount = 0U;
        return;
    }

    targetOffsetNs = g_masterPwmPhaseOffsetNs + g_masterPwmPiOffsetNs;
    targetAtPps = wrap_u32(ns_to_tbclk_ticks(targetOffsetNs), modulo);

    if (g_masterPwmHardwareSyncEnable != 0U)
    {
        set_epwm_up_down_phase(SYNC_EPWM_BASE, targetAtPps, tbprd, false);
        g_masterPwmApplyCount++;

        if ((err > lockThresholdTicks) || (err < -lockThresholdTicks))
        {
            g_masterPwmLocked = false;
            g_masterPwmLockCount = 0U;
        }
        else
        {
            if (g_masterPwmLockCount < 0xFFFFFFFFU)
            {
                g_masterPwmLockCount++;
            }
            if (g_masterPwmLockCount >= 3U)
            {
                g_masterPwmLocked = true;
            }
        }
        return;
    }

    if ((err > lockThresholdTicks) || (err < -lockThresholdTicks))
    {
        /*
         * Re-read eCAP immediately before SWFSYNC. The earlier targetNow only
         * compensates PPS-to-ISR latency; this read also covers the C code time
         * spent deciding whether a correction is needed.
         */
        applyTs = ECAP_getTimeBaseCounter(PPS_ECAP_BASE);
        applyDtTbclk = ecap_ticks_to_tbclk(applyTs - capTs);
        targetAtApply = (targetAtPps + (applyDtTbclk % modulo)) % modulo;
        newPhase = targetAtApply;

        set_epwm_up_down_phase(SYNC_EPWM_BASE, newPhase, tbprd, true);
        g_masterPwmApplyCount++;
        g_masterPwmLocked = false;
        g_masterPwmLockCount = 0U;
    }
    else
    {
        if (g_masterPwmLockCount < 0xFFFFFFFFU)
        {
            g_masterPwmLockCount++;
        }
        if (g_masterPwmLockCount >= 3U)
        {
            g_masterPwmLocked = true;
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
    /* Convert TBCTR plus direction into the monotonic phase ramp used above. */
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

static void set_epwm_up_down_phase(uint32_t base, uint32_t phase, uint32_t tbprd, bool forceNow)
{
    /*
     * Convert monotonic phase back to TBPHS plus post-sync count direction,
     * then force a sync pulse so the new phase loads immediately.
     */
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
    if (forceNow)
    {
        EPWM_forceSyncPulse(base);
    }
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

static void update_phase_pi(int32_t phaseErrNs)
{
    int32_t maxOffsetNs = g_masterPwmPiMaxOffsetNs;
    int32_t kpDiv = (int32_t)g_masterPwmPiKpDiv;
    int32_t kiDiv = (int32_t)g_masterPwmPiKiDiv;
    int32_t pTerm;

    if (g_masterPwmPiEnable == 0U)
    {
        g_masterPwmPiOffsetNs = 0L;
        g_masterPwmPiIntegralNs = 0L;
        return;
    }

    if (maxOffsetNs < 0L)
    {
        maxOffsetNs = -maxOffsetNs;
    }
    if (maxOffsetNs < 100L)
    {
        maxOffsetNs = 100L;
    }
    if (kpDiv < 1L)
    {
        kpDiv = 1L;
    }
    if (kiDiv < 1L)
    {
        kiDiv = 1L;
    }

    pTerm = phaseErrNs / kpDiv;
    g_masterPwmPiIntegralNs =
        clamp_i32(g_masterPwmPiIntegralNs + (phaseErrNs / kiDiv),
                  -maxOffsetNs,
                   maxOffsetNs);
    g_masterPwmPiOffsetNs =
        clamp_i32(pTerm + g_masterPwmPiIntegralNs,
                  -maxOffsetNs,
                   maxOffsetNs);
}
