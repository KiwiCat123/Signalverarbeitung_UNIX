#include "Timer.h"
#include <time.h>
#include <signal.h>
#include <semaphore.h>
#include "Filter_1.h"
#include "Generator.h"
#include "FileOut.h"

volatile bool _signal_generate; //timer flag, ready for new sample, false = timer ended

void Timer_Sig_Handler(int a) {
    _signal_generate = 0;
    sem_post(&OutputSem);
    sem_post(&FilterSem);
    sem_post(&GeneratorSem);
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
