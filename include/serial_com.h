#ifndef SERIAL_COM_H
#define SERIAL_COM_H

#include <Arduino.h>
#include "config.h"

// Structure to hold the parsed incoming data
struct ReceivedData {
    int motor = MOTOR_PWM_PULSE_NEUTRAL;
    int steer = STEER_PWM_PULSE_NEUTRAL;
};

enum FailsafeType {
    NO_FAILSAFE = 0,
    RC_FAILSAFE = 1,
    SERIAL_FAILSAFE = 2,
    PWM_IN_FAILSAFE = 3,
    E_STOP_FAILSAFE = 4
};

struct TransmitData {
    bool ams;
    bool start;
    bool no_rc;
    FailsafeType failsafe;
    uint8_t mission;
};

// Initializes the serial communication
void InitSerial(unsigned long baudRate = 115200);

// Checks for incoming serial data, parses it, and returns true if a valid message was received
bool ReadSerial(ReceivedData &data);

// Transmits the fixed response: "A 0 S 1"
void SendData(TransmitData &data);

#endif // SERIAL_COM_H