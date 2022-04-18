#include "FileOut.h"
#include "Samples.h"
#include <stdio.h>
#include <stdbool.h>
#include "Generator.h"
#include <time.h>
#include "pigpio.h"
#include <unistd.h>
#include <limits.h>
#include "Timer.h"
#include "Filter_1.h"
#include <stdio.h>

void statistic(unsigned long long time_diff);
SignalPoint newSample = 0; //raw sample from filter
SignalPoint generatorSample = 0; //unfiltered sample
double TimeInDouble; //time difference between samples in double in ms
sem_t OutputSem;

int CSV_out() {
    FILE* outFile;
    int err = 0;
    char path[] = "out.csv";

    outFile = fopen(path,"w");
    if(!outFile) return -1;

    fprintf(outFile,"Time;Value(generator);Value(filtered)\n");
    fprintf(outFile,"%f;%i;%i\n",TimeInDouble,generatorSample,newSample);

    return 0;
}

int consoleOut() {
    int ret = 0;

    printf("%i, %.4f ms\n", newSample, TimeInDouble);

    return ret;
}

int DAC_out() {
    unsigned short sampleOut; //sample prepared for DAC output
    unsigned short message; //DAC message
    unsigned char msgBuf[2];
    int ret = 0;

    //prepare DAC message (filtered sample)
    sampleOut = newSample - SHRT_MIN; //prepare sample for DAC, only positive voltage
    sampleOut = sampleOut >> 6; //16bit sample to 10bit
    message = sampleOut << 2; //make space for 2 reserved bits at beginning
    message |= 0x5000; //add flags SPD=1, PWR=0, R1=0, R0=1 0101
    msgBuf[1] = (unsigned char)(message & 0x00FC);
    msgBuf[0] = (unsigned char)(message >> 8); //gets sent first
    ret = spiWrite(spiHandle, msgBuf, 2); //send message to DAC via SPI (OUTB buffer)
    printf("%u   %X\n",sampleOut, message);
    printf("%X %X\n", msgBuf[1], msgBuf[0]);
    printf("return: %i\n\n", ret);

    //DAC message (unfiltered sample)
    sampleOut = generatorSample - SHRT_MIN;
    sampleOut = sampleOut >> 6;
    message = sampleOut << 2;
    message |= 0xC000; //SPD=1, PWR=0, R1=1, R0=0 1100
    msgBuf[1] = (unsigned char)(message & 0x00FC);
    msgBuf[0] = (unsigned char)(message >> 8); //gets sent first
    ret = spiWrite(spiHandle, msgBuf, 2); //send message to DAC via SPI (OUTA and buffer updated)

    return ret;
}

int OutputFnc(const struct OUTARGS* pArgs) {
    unsigned long long _time_diff = 0; //time difference between samples
    struct timespec RawTime;
    long TimeInLong;
    unsigned long long oldTime = 0;
    time_t TimeInSec;
    time_t oldTimeSec = 0;
    int i;
    int ret = 0;

    _signal_generate = true; //reset timer flag
    _signal_out = true; //reset filter flag
    gpioSetMode(OUT_PIN, PI_OUTPUT);

    while(!abortSig) {
        gpioWrite(OUT_PIN,PI_SET);

        /*while(_signal_generate) { //wait for timer flag
            if(abortSig) return ret;
        }
        _signal_generate = true; //reset timer flag*/
        sem_wait(&OutputSem);
        if(abortSig) return ret;

        //calculate time between new samples
        ret = clock_gettime(CLOCK_MONOTONIC_RAW,&RawTime);
        TimeInLong = RawTime.tv_nsec;
        TimeInSec = RawTime.tv_sec;
        if(TimeInSec > oldTimeSec) {
            _time_diff = (unsigned long long)1000000000 - oldTime;
            _time_diff += TimeInLong;
        }
        else {
            _time_diff = TimeInLong - oldTime;
        }
        oldTime = TimeInLong;
        oldTimeSec = TimeInSec;

        //convert time to ms (double)
        TimeInDouble = (double)_time_diff / 1000000.0;
        statistic((_time_diff / 100));

        for(i=0;i<pArgs->cnt;i++) {
            ret = ((int(*)())pArgs->fnc[i])();
        }

        while (_signal_out) { //wait for filter flag to get set
            if (abortSig) return ret;
        }
        newSample = filterOutBuf; //read new sample from filter
        generatorSample = genSample; //read unfiltered sample
        _signal_out = true; //reset filter flag, sample read from buffer

        gpioWrite(OUT_PIN,PI_CLEAR);
    }

    return ret;
}