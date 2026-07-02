#ifndef RC_H
#define RC_H

#include "sbus.h"
#include "arduino.h"
#include "config.h"
// number of bits used to store steer and trhottle values (same as pwm resolution)
#define VALUE_BIT_COUNT 12

/** SBUS proportional channels (1..16) for RC path. */
#define SBUS_CH_THROTTLE     1u
#define SBUS_CH_STEER        2u
#define SBUS_CH_START        5u
#define SBUS_CH_AMS          6u
/**
 * Momentary switch on the TX (logical HIGH while held): steer PWM OUT forced to 0 µs
 * in all operating states (RC, Active, Ready). Works whenever SBUS is valid and not failsafe.
 */
#define SBUS_CH_STEER_ZERO   9u
/** Both HIGH together = emergency off (AMS/Start forced off until released and re-armed). */
#define SBUS_CH_ESTOP_A      11u
#define SBUS_CH_ESTOP_B      12u
/** Mission id = 3-bit value: bit0..bit2 from these SBUS channels (logical HIGH). */
#define SBUS_CH_MISSION_BIT0  7u
#define SBUS_CH_MISSION_BIT1  8u
#define SBUS_CH_MISSION_BIT2  10u
#define SBUS_MISSION_BIT_COUNT  3u

/**
 * RC channel data structure
 * Stores all RC channel values from SBUS receiver
 */
struct RcData {
    uint16_t throttle = MOTOR_PWM_PULSE_NEUTRAL;      // Channel 1: throttle control
    uint16_t steer = STEER_PWM_PULSE_NEUTRAL;         // Channel 2: steering control
    bool start;         // Channel 5: start signal
    bool ams;           // Channel 6: AMS signal
    bool steer_zero;    // Channel 9: steer zero/disable
    bool estop;       // Channel 11 || 12: emergency stop A or B
    uint8_t mission;        // Channel 7, 8, 10: mission bits (3-bit value)
    bool failsafe;          // SBUS failsafe status
    bool lost_frame;        // SBUS lost frame flag
};

// Public functions
void InitRc(void);
bool ReadRc(void);
void CalibrateRc(void);

// Getter functions for RC data
uint16_t RcGetThrottle(void);
uint16_t RcGetSteer(void);
bool RcGetStart(void);
bool RcGetAms(void);
bool RcGetSteerZero(void);
bool RcGetEstop(void);
uint8_t RcGetMission(void);
bool RcIsFailsafe(void);
bool RcIsLostFrame(void);
RcData RcGetAllData(void);

#endif