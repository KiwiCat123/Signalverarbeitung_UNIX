#pragma once
#include "Samples.h"
#include <stdbool.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

struct OUTARGS {
    unsigned int cnt; //amount of output functions
    int**fnc; //array of pointers to output fnc
};

int writeCSV(SIGNAL_OUT SignalGenerator[], SIGNAL_OUT SignalFiltered[], unsigned long count, char path[]);

int consoleOut();
int DAC_out();
int CSV_out();
int OutputFnc(struct OUTARGS*);

#define OUT_PIN 22 //GPIO-Pin for output

extern sem_t OutputSem;
extern int spiHandle;