#include "Timer.h"
#include <time.h>
#include <signal.h>
#include "Generator.h"

void Timer_Sig_Handler(int a) {
    _signal_generate = 0;
}

void timer_fnc() {
    volatile int ret = 0;
    timer_t TimerId;
    struct itimerspec TimerSettings;
    TimerSettings.it_interval.tv_nsec = PERIOD * 1000000;
    TimerSettings.it_interval.tv_sec = 0;
    TimerSettings.it_value.tv_nsec = PERIOD * 1000000;
    TimerSettings.it_value.tv_sec = 0;

    //_signal_generate = 1;

    signal(SIGALRM,Timer_Sig_Handler);

    ret = timer_create(CLOCK_MONOTONIC,NULL,&TimerId);
    ret = timer_settime(TimerId,0,&TimerSettings,NULL);
}
