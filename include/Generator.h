#pragma once
#include "Samples.h"
#include <stdbool.h>

enum eSIGNAL {SINUS, RECTANGLE};

unsigned long generate(SIGNAL_OUT** SignalReturn, enum eSIGNAL eSignal, SignalPoint sAmplitude, SignalTime dPeriod, SignalTime dLength, const unsigned long long ullSampleRate);

SignalPoint _Rectangle(SignalTime dCurTime, const SignalPoint amplitude, const SignalTime dSetPeriodTime);

int generate_RT(enum eSIGNAL eSignal, SignalPoint sAmplitude, const unsigned long ulPeriod, const unsigned long ulSamplePeriod);
extern volatile bool _generator_ready; //flag generator finished new sample, false = ready (new sample ready)
extern volatile bool abortSig; //Signal to end in RT-mode, true = end
extern volatile SignalPoint generateOutBuf; //output buffer for single sample from generator