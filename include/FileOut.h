#pragma once
#include "Samples.h"
#include <stdbool.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

int writeCSV(SIGNAL_OUT SignalGenerator[], SIGNAL_OUT SignalFiltered[], unsigned long count, char path[]);

void* consoleOut(void*);

#define OUT_PIN 22 //GPIO-Pin for output

extern sem_t OutputSem;