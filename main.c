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
#include "pigpio.h"

#define TEST_SAMPLES 50

volatile bool abortSig; //Signal to end in RT-mode, true = end
unsigned long long* collectedTimes = NULL; //collected time differences by consoleOut()
int amount = 0; //amount of collectedTimes
int spiHandle = 0;
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
	pthread_t ThreadsHandle[2];
    int arg = 1;

    ret = sem_init(&OutputSem,0,0);

	gpioInitialise(); //init gpio lib

    signal(SIGINT, SigHandler); //ctrl+c

    //prepare SPI
    spiHandle = spiOpen(0,1000000,0x802); //open SPI, chanel 0, 1MHz, Mode 2 + 2Bytes
    printf("%i\n\n",spiHandle);

    timer_fnc(); //start timer
	ret = pthread_create(&(ThreadsHandle[0]),NULL,DAC_out, NULL); //output Thread
	if (ret != 0) return -2;
    ret = pthread_create(&(ThreadsHandle[1]),NULL,filter_RT, NULL); //filter Thread
    if (ret != 0) return -2;
	ret = generate_RT(RECTANGLE, MAX_SIG_VALUE, PERIOD*8, PERIOD); //start generator
    pthread_join(ThreadsHandle[0],NULL);
    pthread_join(ThreadsHandle[1],NULL);
    sem_destroy(&OutputSem);

	result_statistics();

    //cleanup
	gpioWrite(OUT_PIN,PI_CLEAR);
    ret = spiClose(spiHandle);
    printf("\n\n%i\n", ret);
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

    if(amount <= 0) return;

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
