#pragma once
#include <stdbool.h>

#define PERIOD 300 //timer period in ms

void timer_fnc();
extern volatile bool _signal_generate; //timer flag, ready for new sample, false = timer ended