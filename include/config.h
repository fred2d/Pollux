#ifndef CONFIG_H
#define CONFIG_H

#define ANALOG_WRITE_RESOLUTION 12

#define SERIAL_BAUD_RATE 460800


//pwm constants for the PWM Motor output pin. 
// if values lay not within this range, the value will be regarded as invalid and the last valid value will be used instead
#define MOTOR_PWM_PULSE_MIN 800       // microseconds
#define MOTOR_PWM_PULSE_MAX 2350       // microseconds
#define MOTOR_PWM_PULSE_NEUTRAL 1575  // microseconds

//pwm constants for the PWM Steer output pin
#define STEER_PWM_PULSE_MIN 800       // microseconds
#define STEER_PWM_PULSE_MAX 2200       // microseconds
#define STEER_PWM_PULSE_NEUTRAL 1500  // microseconds

//pwm constants for the PWM input pin
//only used to limit the valid range, directly passed through to motor output pwm
#define PWM_PULSE_MIN 1100       // microseconds
#define PWM_PULSE_MAX 2050       // microseconds

// timeout for the failsafe in milliseconds, if no valid input is received within this time, the failsafe will be triggered
#define RC_FAILSAFE_TIMEOUT 1000  // milliseconds
#define SERIAL_FAILSAFE_TIMEOUT 1000  // milliseconds
#define PWM_IN_FAILSAFE_TIMEOUT 1000  // milliseconds

#endif // CONFIG_H