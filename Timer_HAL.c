//------------------------------------------
// Timer API (Application Programming Interface)
// Also known as BUTTON HAL (Hardware Abstraction Layer)
// HAL is a specific form of API that designs the interface with a certain hardware

#include <TIMER_HAL.h>

#define TIMER0_PRESCALER TIMER32_PRESCALER_1
#define TIMER1_PRESCALER TIMER32_PRESCALER_256
#define SYS_CLOCK_F 48000000

/* This function gets the hardware timer (since it needs its prescaler value) and time in milliseconds
 * and returns the number of wait cycles associated with that time.
 * For example, if the system clock is 3 MHz, the prescaler is 1 and TimeInMS is 1000, then the
 * returned value is going to be 3000,000
 * If the number of wait cycles is too big for this software timer to be used, it returns -1
 */
int64_t WaitCycles(uint32_t hwtimer, uint64_t TimeInMS)
{
     // This function returns the system clock
    //uint64_t sysClock = CS_getSMCLK();
    uint64_t sysClock = SYS_CLOCK_F;

    uint8_t  prescalerFlag;
    uint64_t prescalerValue;

    // Based on the hw timer, we have to see which prescaler to use
    if (hwtimer == TIMER32_0_BASE)
        prescalerFlag = TIMER0_PRESCALER;
    else if (hwtimer == TIMER32_1_BASE)
        prescalerFlag = TIMER1_PRESCALER;

    // The prescaler we get in the previous section is simply a flag. "Control click" on it to see what I mean.
    // We need to turn that into the actual value of the prescaler
    switch(prescalerFlag)
    {
    case TIMER32_PRESCALER_1:
        prescalerValue = 1;
        break;
    case TIMER32_PRESCALER_16:
        prescalerValue = 16;
        break;
    case TIMER32_PRESCALER_256:
        prescalerValue = 256;
        break;
    }

    int64_t waitCycles;
    waitCycles = TimeInMS * (sysClock / prescalerValue / 1000);

    if (waitCycles > UINT32_MAX)
        waitCycles = -1;

    return waitCycles;
}


void InitOneShotSWTimer(
        OneShotSWTimer_t* OST,
        uint32_t  hwtimer,
        uint32_t  TimeInMS)
{
    OST->hwtimer = hwtimer;

    OST->waitCycles  = WaitCycles(hwtimer, TimeInMS);
}

void StartOneShotSWTimer(OneShotSWTimer_t* OST)
{
    OST->startCounter = Timer32_getValue(OST->hwtimer);
}

bool OneShotSWTimerExpired(OneShotSWTimer_t* OST)
{
    bool expired = false;

    // HW timer period
    int64_t HWTimerPeriod = UINT32_MAX+ 1;

    //This is C2 from notes, while OST->startCounter is C1
    uint32_t currentCounter = Timer32_getValue(OST->hwtimer);

    int64_t ElapsedCycles =  OST->startCounter - currentCounter;
    if (ElapsedCycles < 0)
        ElapsedCycles += HWTimerPeriod ;

    if (ElapsedCycles >= OST->waitCycles)
        expired = true;
    else
        expired = false;

    return expired;
}


void InitHWTimers() {
    // The prescaler for each of the timers is defined as a macro
    Timer32_initModule(TIMER32_0_BASE, TIMER0_PRESCALER, TIMER32_32BIT, TIMER32_PERIODIC_MODE);
    Timer32_setCount(TIMER32_0_BASE, UINT32_MAX);
    Timer32_startTimer(TIMER32_0_BASE, false);

    Timer32_initModule(TIMER32_1_BASE, TIMER1_PRESCALER, TIMER32_32BIT, TIMER32_PERIODIC_MODE);
    Timer32_setCount(TIMER32_1_BASE, UINT32_MAX);
    Timer32_startTimer(TIMER32_1_BASE, false);
}




