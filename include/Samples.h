#pragma once
#include <limits.h>

typedef short SignalPoint;
typedef double SignalTime;
#define SAMPLE_FRQ 100000
typedef struct {
	SignalPoint point;
	float time;
} SIGNAL_OUT;
#define MAX_SIG_VALUE SHRT_MAX
#define MIN_SIG_VALUE SHRT_MIN