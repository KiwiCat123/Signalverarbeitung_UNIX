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
volatile bool _generator_ready; //flag generator finished new sample, false = ready (new sample ready)


int generate_RT(enum eSIGNAL eSignal, SignalPoint sAmplitude, unsigned long ulPeriod, const unsigned long ulSampleRate) {
	SignalTime dTimePerPoint; //time between 2 points of output signal
	SignalTime dCurTime = 0; //current time for next point
    int ret;

	//first point
	switch (eSignal) {
	case SINUS: {
		generateOutBuf = 0;

	} break;
	case RECTANGLE: {
		generateOutBuf = _Rectangle(0.0, 0, ulPeriod);
	} break;
	default: {
		return -1;
	}
	};

    _generator_ready = false; //new sample ready
	while (_signal_generate); //wait until set to 0 again from other thread
	_signal_generate = true; //reset timer flag

	while (!abortSig) {
		switch (eSignal) {
		case SINUS: {
			generateOutBuf = 0;
		} break;
		case RECTANGLE: {
			generateOutBuf = _Rectangle(dCurTime, sAmplitude, 0.0);
		} break;
		};

		//increase time, check if period is over
		dCurTime += dTimePerPoint;
		if (dCurTime > ulPeriod) {
			dCurTime -= ulPeriod;
		}

        _generator_ready = false; //new sample ready
		while (_signal_generate); //wait until set to 0 again from other thread
		_signal_generate = true; //reset timer flag
		
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

	dTimePerPoint = 1.0 / ullSampleRate;
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
