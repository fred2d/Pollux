#include "rc.h"

static bfs::SbusRx sbus_rx(&Serial2);
static bfs::SbusData data;
static RcData rc_data = {0};
// static uint16_t rc_data_min[data.NUM_CH] = {0};
// static uint16_t rc_data_max[data.NUM_CH] = {0};

uint16_t SbusToPulseWidth(uint8_t);
uint8_t ParseRcSwitch(uint8_t);

void InitRc() {
  sbus_rx.Begin();
  // CalibrateRc();
}

bool ReadRc() {
  if (sbus_rx.Read()) {
    /* Grab the received data */
    data = sbus_rx.data();
    
    /* Update RC data structure */
    
    // check if values lay within valid range.
    uint16_t throttle_pulse = SbusToPulseWidth(SBUS_CH_THROTTLE);
    uint16_t steer_pulse = SbusToPulseWidth(SBUS_CH_STEER);
    if (throttle_pulse < MOTOR_PWM_PULSE_MIN || throttle_pulse > MOTOR_PWM_PULSE_MAX) return false;
    if (steer_pulse < STEER_PWM_PULSE_MIN || steer_pulse > STEER_PWM_PULSE_MAX) return false;
    rc_data.throttle = throttle_pulse; //ParseRcLinear(SBUS_CH_THROTTLE, MOTOR_PWM_PULSE_MIN, MOTOR_PWM_PULSE_MAX);

    rc_data.steer = steer_pulse; //ParseRcLinear(SBUS_CH_STEER, STEER_PWM_PULSE_MIN, STEER_PWM_PULSE_MAX);
    
    // rc switch 1 or 2 -> true, else false
    rc_data.start = ParseRcSwitch(SBUS_CH_START);
    rc_data.ams = ParseRcSwitch(SBUS_CH_AMS);
    rc_data.steer_zero = ParseRcSwitch(SBUS_CH_STEER_ZERO);
    rc_data.estop = ParseRcSwitch(SBUS_CH_ESTOP_A) || ParseRcSwitch(SBUS_CH_ESTOP_B);
    rc_data.mission = (ParseRcSwitch(SBUS_CH_MISSION_BIT2)<<2) | (ParseRcSwitch(SBUS_CH_MISSION_BIT1)<<1) | ParseRcSwitch(SBUS_CH_MISSION_BIT0);
    rc_data.failsafe = data.failsafe;
    rc_data.lost_frame = data.lost_frame;
    return true;
  }
  return false;
}

uint16_t SbusToPulseWidth(uint8_t channel) {
  // Map SBUS value (172..1811) to pulse width (min_value..max_value)
  uint16_t sbus_value = data.ch[channel - 1];
  return map(sbus_value, 172, 1811, 1000, 2000);
}

//returns 0, 1 or 2 with 1 being the middle position
uint8_t ParseRcSwitch(uint8_t channel) {
  //convert sbus to pulse width
  uint16_t pulse_width = SbusToPulseWidth(channel);
  
  if (pulse_width < 1400) {
    return 0;
  } else if (pulse_width < 1600) {
    return 1;
  } else {
    return 2;
  }
}

// void CalibrateRc() {
//   for (int8_t i = 0; i < data.NUM_CH; i++) {
//     rc_data_min[i] = 172; //-1;
//     rc_data_max[i] = 1811; //0;  
//   }
// }

// Getter function implementations
uint16_t RcGetThrottle(void) {
    return rc_data.throttle;
}

uint16_t RcGetSteer(void) {
    return rc_data.steer;
}

bool RcGetStart(void) {
    return rc_data.start;
}

bool RcGetAms(void) {
    return rc_data.ams;
}

bool RcGetSteerZero(void) {
    return rc_data.steer_zero;
}

bool RcGetEstop(void) {
    return rc_data.estop;
}

uint8_t RcGetMission(void) {
    return rc_data.mission;
}


bool RcIsFailsafe(void) {
    return rc_data.failsafe;
}

bool RcIsLostFrame(void) {
    return rc_data.lost_frame;
}

RcData RcGetAllData(void) {
    return rc_data;
}
