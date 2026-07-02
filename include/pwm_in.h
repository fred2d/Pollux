#ifndef PWM_IN
#define PWM_IN

#include "Arduino.h"

// PWM signal constraints
#include "config.h"
#include "pins.h"


// Public functions
void InitPwmIn();
bool ReadPwmIn();
uint16_t GetPulseWidthPwmIn();

#endif