//------------------------------------------
// TIMER API
// Also known as TIMER HAL (Hardware Abstraction Layer)
// HAL is a specific form of API that designs the interface with a certain hardware

#ifndef TIMERS_H_
#define TIMERS_H_

#include <ti/devices/msp432p4xx/driverlib/driverlib.h>


typedef struct {
    uint32_t    hwtimer;        // hardware timer used as basis for this software timer
    uint32_t    waitCycles;     // wait "cycles" for the software timer
    uint32_t    startCounter;   // last counter value when the SW timer started (C1)
} OneShotSWTimer_t;

/*
 * This function ties the software period pointed by OST to hwtimer and sets its wait cycles
 */
void InitOneShotSWTimer(OneShotSWTimer_t* OST,
                        uint32_t  hwtimer,
                        uint32_t  waitCycles);

/*
 * This function simply records the start time which is C1 from notes
 */
void StartOneShotSWTimer(OneShotSWTimer_t* OST);

/*
 * This function checks to see if the sw timer has expired, i.e. wait cycles has passed
 */
bool OneShotSWTimerExpired(OneShotSWTimer_t* OST);

/*
 * This function initializes both hw timers
 */
void InitHWTimers();



#endif // TIMERS_H_
