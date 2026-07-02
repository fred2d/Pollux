#ifndef PWM_OUT_H
#define PWM_OUT_H

#include "Arduino.h"

#include "config.h"
#include "pins.h"

void InitPwmOut();
void SetMotorPwm(uint16_t pulse_width);
void SetSteerPwm(uint16_t pulse_width);

#endif