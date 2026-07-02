#include "pwm_out.h"



void InitPwmOut() {
    pinMode(PWM_MOTOR_PIN, OUTPUT);
    pinMode(PWM_STEER_PIN, OUTPUT);

    analogWriteFrequency(PWM_MOTOR_PIN, 400);
    analogWriteFrequency(PWM_STEER_PIN, 50);
    analogWriteResolution(ANALOG_WRITE_RESOLUTION);
}

void SetMotorPwm(uint16_t pulse_width) {
    // Ensure pulse width is within valid range
    if (pulse_width < PWM_PULSE_MIN) {
        pulse_width = PWM_PULSE_MIN;
    } else if (pulse_width > PWM_PULSE_MAX) {
        pulse_width = PWM_PULSE_MAX;
    }
    // Convert pulse width to duty cycle for analogWrite
    uint8_t duty_cycle = map(pulse_width, 0, 2500, 0, (1 << ANALOG_WRITE_RESOLUTION) - 1);
    analogWrite(PWM_MOTOR_PIN, duty_cycle);
}

void SetSteerPwm(uint16_t pulse_width) {
    // Ensure pulse width is within valid range
    if (pulse_width < PWM_PULSE_MIN) {
        pulse_width = PWM_PULSE_MIN;
    } else if (pulse_width > PWM_PULSE_MAX) {
        pulse_width = PWM_PULSE_MAX;
    }
    // Convert pulse width to duty cycle for analogWrite
    uint8_t duty_cycle = map(pulse_width, 0, 2500, 0, (1 << ANALOG_WRITE_RESOLUTION) - 1);
    analogWrite(PWM_STEER_PIN, duty_cycle);
}