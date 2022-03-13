#include "FileOut.h"
#include "Samples.h"
#include <stdio.h>
#include <stdbool.h>
#include "Generator.h"
#include <time.h>
#include "pigpio.h"


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

void* consoleOut(void* p) {
	SignalPoint sampleOut;
	unsigned long long _time_diff = 0; //time difference between samples
	struct timespec RawTime;
    long TimeInLong;
	double TimeInDouble; //time difference in double in ms
	unsigned long long oldTime = 0;
    time_t TimeInSec;
    time_t oldTimeSec = 0;
    int ret = 0;

	_generator_ready = true; //reset generator flag

	while (!abortSig) {
		while (_generator_ready) { //wait for generator flag to get set
            if (abortSig) return 0;
        }
        _generator_ready = true; //reset generator flag
		//read new sample
		sampleOut = generateOutBuf;

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
		gpioWrite(OUT_PIN,PI_CLEAR);
	}
}
