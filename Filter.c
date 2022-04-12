#include "Filter_1.h"
#include "Samples.h"
#include <stdlib.h>
#include <limits.h>

volatile SignalPoint filterOutBuf; //output buffer for single sample from filter
volatile bool _signal_out; //sample ready for output flag, false = ready (new sample ready)

const double dCoeff[9] = {
   0.005069883484836,  0.02935816274752,   0.1107437912658,   0.2193406809055,
	 0.2709749631927,   0.2193406809055,   0.1107437912658,  0.02935816274752,
   0.005069883484836
};

SIGNAL_OUT* filter(SIGNAL_OUT SignalInput[], unsigned long amount) {
	SIGNAL_OUT* FilterOutput = NULL;
	SignalPoint sampleBuffer[FILTER_LENGTH] = {0,0,0,0,0,0,0,0,0};
	double dResult = 0.0;
	unsigned long count = 0;
	int i;

	FilterOutput = malloc(amount * sizeof(SIGNAL_OUT));
	if (!FilterOutput) return NULL;

	for (count = 0; count < amount; count++) {
		for (i = FILTER_LENGTH - 1; i > 0; i--) { //right shift
			sampleBuffer[i] = sampleBuffer[i - 1];
		}
		sampleBuffer[0] = SignalInput[count].point; //new sample to beginning of buffer

		for (i = 0; i < FILTER_LENGTH; i++) { //Filter MAC (next sample point for filtered signal)
			dResult += dCoeff[i] * (double)sampleBuffer[i];
		}

		FilterOutput[count].time = SignalInput[count].time;

		if (dResult > 0) dResult += 0.5; //round result for integer conversion
		else dResult -= 0.5;
		if (dResult > MAX_SIG_VALUE) dResult = MAX_SIG_VALUE; //prevent overflow
		else if (dResult < MIN_SIG_VALUE) dResult = MIN_SIG_VALUE;
		FilterOutput[count].point = (short)dResult;
		dResult = 0.0;
	}

	return FilterOutput;
}