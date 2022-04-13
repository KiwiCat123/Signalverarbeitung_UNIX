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

void statistic(unsigned long long time_diff);

/*int writeCSV(SIGNAL_OUT SignalGenerator[], SIGNAL_OUT SignalFiltered[], unsigned long count, char path[]) {
	FILE* outFile;
	unsigned int i = 0;
	errno_t err;

	err = fopen_s(&outFile, path, "w");
	if (err) return err;

	fprintf(outFile, "Time;Value(generator);Value(filterd)\n");
	for (i = 0; i < count; i++) {
		fprintf(outFile, "%f;%i;%i\n",SignalGenerator[i].time,SignalGenerator[i].point,SignalFiltered[i].point);
		//printf_s("%f;%i;%i\n", SignalGenerator[i].time, SignalGenerator[i].point, SignalFiltered[i].point);
	}

	fclose(outFile);
	return 0;
}*/

void* consoleOut() {
	SignalPoint sampleOut = 0;
	unsigned long long _time_diff = 0; //time difference between samples
	struct timespec RawTime;
    long TimeInLong;
	double TimeInDouble; //time difference in double in ms
	unsigned long long oldTime = 0;
    time_t TimeInSec;
    time_t oldTimeSec = 0;
    int ret = 0;

    _signal_generate = true; //reset timer flag
    _signal_out = true; //reset filter flag

	while (!abortSig) {
        if(abortSig) return NULL;

		//calculate time between new samples
		gpioSetMode(OUT_PIN, PI_OUTPUT);
		gpioWrite(OUT_PIN,PI_SET);
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

		printf("%i, %.4f ms\n", sampleOut, TimeInDouble);

        while (_signal_out) { //wait for generator flag to get set
            if (abortSig) return NULL;
        }
        sampleOut = filterOutBuf; //read new sample from filter
        _signal_out = true; //reset filter flag, sample read from buffer

		gpioWrite(OUT_PIN,PI_CLEAR);

        while(_signal_generate) { //wait for timer flag
            if(abortSig) return NULL;
        }
        _signal_generate = true; //reset timer flag

	}

    return NULL;
}

void* DAC_out(void* p) {
    SignalPoint newSample = 0; //raw sample from filter
    SignalPoint generatorSample = 0; //unfiltered sample
    unsigned short sampleOut; //sample prepared for DAC output
    unsigned short message; //DAC message
    unsigned char msgBuf[2];
    int ret = 0;

    while(!abortSig) {
        //prepare DAC message (filtered sample)
        sampleOut = newSample - SHRT_MIN; //prepare sample for DAC, only positive voltage
        sampleOut = sampleOut >> 6; //16bit sample to 10bit
        message = sampleOut << 2; //make space for 2 reserved bits at beginning
        message |= 0x5000; //add flags SPD=1, PWR=0, R1=0, R0=1 0101
        msgBuf[1] = (unsigned char)(message & 0x00FC);
        msgBuf[0] = (unsigned char)(message >> 8); //gets sent first
        ret = spiWrite(spiHandle, msgBuf, 2); //send message to DAC via SPI (OUTB buffer)
        /*printf("%u   %X\n",sampleOut, message);
        printf("%X %X\n", msgBuf[1], msgBuf[0]);
        printf("return: %i\n\n", ret);*/

        //DAC message (unfiltered sample)
        sampleOut = generatorSample - SHRT_MIN;
        sampleOut = sampleOut >> 6;
        message = sampleOut << 2;
        message |= 0xC000; //SPD=1, PWR=0, R1=1, R0=0 1100
        msgBuf[1] = (unsigned char)(message & 0x00FC);
        msgBuf[0] = (unsigned char)(message >> 8); //gets sent first
        ret = spiWrite(spiHandle, msgBuf, 2); //send message to DAC via SPI (OUTA and buffer updated)

        while (_signal_out) { //wait for generator flag to get set
            if (abortSig) return NULL;
        }
        newSample = filterOutBuf; //read new sample from filter
        generatorSample = genSample;
        _signal_out = true; //reset filter flag, sample read from buffer

        while(_signal_generate) { //wait for timer flag
            if(abortSig) return NULL;
        }
        _signal_generate = true; //reset timer flag
    }

    return NULL;
}