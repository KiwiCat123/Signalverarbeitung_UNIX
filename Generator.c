#include <stdio.h>
#include <stdlib.h>
#include "Samples.h"
#include "Generator.h"
#include "Timer.h"
#include <unistd.h>
#include "FileOut.h"
#include <stdbool.h>

volatile bool _signal_generate; //timer flag, ready for new sample, false = timer ended
volatile SignalPoint generateOutBuf; //output buffer for sample


int generate_RT(enum eSIGNAL eSignal, SignalPoint sAmplitude, const unsigned long ulPeriod, const unsigned long ulSamplePeriod) {
	unsigned long ulCurTime = 0; //current time for next point
    SignalPoint newSample; //generated new sample
    int ret;

	//init generator function
	switch (eSignal) {
	case SINUS: {
		newSample = 0;
	} break;
	case RECTANGLE: {
		newSample = _Rectangle(0.0, 0, (SignalTime)ulPeriod);
	} break;
	default: {
		return -1;
	}
	};

    //generating
	while (!abortSig) {
		switch (eSignal) {
		case SINUS: {
			newSample = 0;
		} break;
		case RECTANGLE: {
			newSample = _Rectangle((double)ulCurTime, sAmplitude, 0.0);
		} break;
		};

		//increase time, check if period is over
		ulCurTime += ulSamplePeriod;
		if (ulCurTime >= ulPeriod) {
			ulCurTime -= ulPeriod;
		}

        while (!_generator_ready) { //wait for generator flag to get reset, until last sample got read from buffer
            if (abortSig) return 0;
        }
        generateOutBuf = newSample; //write new sample in buffer
        _generator_ready = false; //new sample ready
		
	}

	return 0;
}

unsigned long generate(SIGNAL_OUT** SignalReturn, enum eSIGNAL eSignal, SignalPoint sAmplitude, SignalTime dPeriod, SignalTime dLength, const unsigned long long ullSampleRate) {
	SIGNAL_OUT* SignalOut = NULL; //array of points for output signal with value and time
	SIGNAL_OUT* pHelpOut = NULL; //pointer on current signal point structure
	unsigned long ulMaxPoints = 0; //number of overall points of output signal
	SignalTime dCurTime = 0; //current time for next point
	SignalTime dTimePerPoint = 0; //time between 2 points of output signal
	SignalPoint sNewPointValue; //value of next out signal point

	if (!SignalReturn) return 0; //output buffer available?

	dTimePerPoint = 1.0 / (double)ullSampleRate;
	ulMaxPoints = (unsigned long)(dLength / dTimePerPoint) + 1;
	SignalOut = malloc(ulMaxPoints * sizeof(SIGNAL_OUT));
	if (!SignalOut) return 0;
	pHelpOut = SignalOut;

	//first point
	switch (eSignal) {
	case SINUS: {
		sNewPointValue = 0;
	} break;
	case RECTANGLE: {
		sNewPointValue = _Rectangle(0.0, 0, dPeriod);
	} break;
	default: {
		free(SignalOut);
		return 0;
	}
	};

	pHelpOut->point = sNewPointValue;
	pHelpOut->time = (float)dCurTime;
	pHelpOut++;
	dCurTime += dTimePerPoint;

	do {
		switch (eSignal) {
		case SINUS: {
			sNewPointValue = 0;
		} break;
		case RECTANGLE: {
			sNewPointValue = _Rectangle(dCurTime, sAmplitude, 0.0);
		} break;
		};

		pHelpOut->point = sNewPointValue;
		pHelpOut->time = (float)dCurTime;
		pHelpOut++;
		dCurTime += dTimePerPoint;
	} while (dCurTime < dLength);

	*SignalReturn = SignalOut;
	return ulMaxPoints;
}

SignalPoint _Rectangle(SignalTime dCurTime, const SignalPoint amplitude, const SignalTime dSetPeriodTime) {
	static SignalTime dPeriod = 0.0;
	
	if (dSetPeriodTime != 0.0) { //set new Time for one period
		dPeriod = dSetPeriodTime;
		return 0;
	}

	if (dPeriod == 0.0) return 0; //generate without period set?

	//generate
	if (dCurTime > dPeriod) {
		do {
			dCurTime -= dPeriod;
		} while (dCurTime > dPeriod);
	}
	if (dCurTime < (dPeriod / 2.0)) return amplitude;
	else return (-1 * amplitude);
}
