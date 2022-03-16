#include <stdio.h>
#include <stdlib.h>
#include "Generator.h"
#include "Samples.h"
#include "FileOut.h"
#include "Filter_1.h"
#include <signal.h>
#include <stdbool.h>
#include "Timer.h"
#include <limits.h>
#include <pthread.h>
#include "compile_helper.h"
#ifdef COMPILE_AMD64_UNIX
#include "pigpio.h"
#else
#include <pigpio.h>
#endif

#define TEST_SAMPLES 50

volatile bool abortSig; //Signal to end in RT-mode, true = end
unsigned long long* collectedTimes = NULL; //collected time differences by consoleOut()
int amount = 0; //amount of collectedTimes
sem_t OutputSem;
void result_statistics(); //evaluate collected statistic values

void SigHandler(int a) {
	printf("ctrl-c!\n");;
	abortSig = true; //signals threads to abort in RT-mode
}

int main() {
	SIGNAL_OUT* aSignalOutput = NULL; //collected samples from generator
	SIGNAL_OUT* aFilteredOutput = NULL; //collected samples from filter
	unsigned long ulCountPoints = 0; //number of overall points of output signal
	unsigned long long ullSampleFrq = SAMPLE_FRQ;
	char path[] = "out.csv"; //generated signal file
	int ret = -5;
	pthread_t ThreadsHandle;
    int arg = 1;

	signal(SIGINT, SigHandler); //ctrl+c

    ret = sem_init(&OutputSem,0,0);

	gpioInitialise(); //init gpio lib

	ret = pthread_create(&ThreadsHandle,NULL,consoleOut,&arg); //output Thread
	if (ret != 0) return -2;
	timer_fnc(); //start Timer
	ret = generate_RT(RECTANGLE, MAX_SIG_VALUE, 8, 2); //start generator
    pthread_join(ThreadsHandle,NULL);
    sem_destroy(&OutputSem);

	result_statistics();

    //cleanup
	gpioWrite(OUT_PIN,PI_CLEAR);
	gpioTerminate();
	free(collectedTimes);

	return ret;
}

void statistic(unsigned long long time_diff) {
	static bool init = false;

	if (!init) {
		collectedTimes = malloc(sizeof(unsigned long long));
		if (!collectedTimes) {
			while (1);
		}
		init = true;
		return;
	}

	amount++;
	collectedTimes = realloc(collectedTimes, sizeof(unsigned long long)*amount);
	if (!collectedTimes) return;
	collectedTimes[amount - 1] = time_diff;

	if (amount == TEST_SAMPLES)
		abortSig = true;
}

void result_statistics() {
	int i;
	unsigned long long average = 0;
	unsigned long long max = 0;
	unsigned long long min = ULLONG_MAX;
	double temp1;
	double temp2_convert;

	for (i = 0; i < amount; i++) {
		average += collectedTimes[i];
		if (average > (ULLONG_MAX - collectedTimes[i]))
			return;
	}
	average = average / amount;

	for (i = 0; i < amount; i++) {
		if (max < collectedTimes[i])
			max = collectedTimes[i];
	}

	for (i = 0; i < amount; i++) {
		if (min > collectedTimes[i])
			min = collectedTimes[i];
	}

	printf("\n\n\n");
	temp2_convert = (double)average / 10000.0;
	temp1 = temp2_convert - PERIOD;
	printf("average: %.4f ms  %+.4f ms\n", temp2_convert, temp1);

	temp2_convert = (double)min / 10000.0;
	temp1 = temp2_convert - PERIOD;
	printf("min: %.4f ms  %+.4f ms\n", temp2_convert, temp1);

	temp2_convert = (double)max / 10000.0;
	temp1 = temp2_convert - PERIOD;
	printf("max: %.4f ms  %+.4f ms\n", temp2_convert, temp1);

	temp1 = (double)(max - min) / 10000.0;
	printf("diff: %+.4f ms\n", temp1);
}