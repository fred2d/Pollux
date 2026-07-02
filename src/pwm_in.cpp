#include "pwm_in.h"

// Pin configuration


// Timing variables (volatile for ISR access)
static volatile uint32_t rise_time = 0;
static volatile uint32_t pulse_width = 0;

// Data storage
static uint32_t pwm_value = MOTOR_PWM_PULSE_NEUTRAL;  // Default to mid-point (1.575 ms) for neutral position
static volatile bool new_data_available = false;

// Validity thresholds (tolerance for detecting valid signals)
static const uint32_t PULSE_MIN_EXTENDED = PWM_PULSE_MIN - 10;
static const uint32_t PULSE_MAX_EXTENDED = PWM_PULSE_MAX + 10; 

// ISR: Interrupt service routine for PWM signal changes
void PwmInISR() {
    uint32_t current_time = micros();
    
    if (digitalRead(PWM_IN_PIN) == HIGH) {
        // Rising edge detected
        rise_time = current_time;
    
    } else {
        // Falling edge detected
        if (rise_time != 0) {
            pulse_width = current_time - rise_time;
            new_data_available = true;
        }
    }
}

// Init function: Initialize PWM input on specified pin
void InitPwmIn() {
    pinMode(PWM_IN_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PWM_IN_PIN), PwmInISR, CHANGE);
}

// Read and process PWM data
bool ReadPwmIn() {
    if (!new_data_available) {
        return false;
    }
    
    // Disable interrupts to read volatile variables safely
    noInterrupts();
    uint32_t temp_pulse_width = pulse_width;
    new_data_available = false;
    interrupts();
    
    // Validate and clamp pulse width
    
    if (temp_pulse_width >= PULSE_MIN_EXTENDED && temp_pulse_width <= PULSE_MAX_EXTENDED) {
        // Clamp to valid range
        if (temp_pulse_width < PWM_PULSE_MIN) {
            temp_pulse_width = PWM_PULSE_MIN;
        } else {
            temp_pulse_width = PWM_PULSE_MAX;
        }
    } else {
        // Too far out of range
        // Ignore this reading and keep the last valid value
        return false;
    }
    pwm_value = temp_pulse_width;
    return true;
}


// Get last measured pulse width
uint16_t GetPulseWidthPwmIn() {
    return pwm_value;
}