#include <Arduino.h>
#include "rc.h"
#include "pwm_in.h"
#include "pwm_out.h"
#include "switches.h"
#include "serial_com.h"
#include "config.h"

// put function declarations here:
// void DisplayRc(void);
// void DisplayPwmIn(void);
// void DebugSerial(void);
// void DisplaySwitches(void);
void Run(void);



enum OpMode {
  FAILSAFE,
  MANUAL,
  RC,
  AUTO
};

OpMode mode;// = FAILSAFE;
ReceivedData targetPulseWidth;
TransmitData stateData;
TransmitData prevStateData = {false, false, false, NO_FAILSAFE, 0};  // Track previous state for change detection

// Failsafe timeout tracking
static uint32_t rc_last_valid_time = 0;
static uint32_t serial_last_valid_time = 0;
static uint32_t pwm_in_last_valid_time = 0;

static bool rc_failsafe = false;
static bool serial_failsafe = false;
static bool pwm_in_failsafe = false;

static bool estop_triggered = false;  // Track if emergency stop has been triggered
/**
 * Helper function to compare TransmitData structs
 */
static bool StateDataChanged(const TransmitData& current, const TransmitData& previous) {
    return (current.ams != previous.ams) ||
           (current.start != previous.start) ||
           (current.no_rc != previous.no_rc) ||
           (current.failsafe != previous.failsafe) ||
           (current.mission != previous.mission);
}

/**
 * Check failsafe status for each data source based on timeout
 */
static void UpdateFailsafeStatus(bool rc_valid, bool serial_valid, bool pwm_in_valid) {
    uint32_t current_time = millis();
    
    // Check RC timeout
    if (rc_valid) {
        rc_last_valid_time = current_time;
        rc_failsafe = false;
    } else if ((current_time - rc_last_valid_time) > RC_FAILSAFE_TIMEOUT) {
        rc_failsafe = true;
    }
    
    // Check Serial timeout
    if (serial_valid) {
        serial_last_valid_time = current_time;
        serial_failsafe = false;
    } else if ((current_time - serial_last_valid_time) > SERIAL_FAILSAFE_TIMEOUT) {
        serial_failsafe = true;
    }
    
    // Check PWM input timeout
    if (pwm_in_valid) {
        pwm_in_last_valid_time = current_time;
        pwm_in_failsafe = false;
    } else if ((current_time - pwm_in_last_valid_time) > PWM_IN_FAILSAFE_TIMEOUT) {
        pwm_in_failsafe = true;
    }
}

void setup() {
  // put your setup code here, to run once:
  //Serial.begin(250000);
  InitSerial(SERIAL_BAUD_RATE);
  InitRc();
  InitPwmIn();
  InitPwmOut();
  InitSwitches();
  
  // Initialize failsafe timestamps to current time to prevent immediate timeout
  uint32_t current_time = millis();
  rc_last_valid_time = current_time;
  serial_last_valid_time = current_time;
  pwm_in_last_valid_time = current_time;
}

//to do: implement rc connection lost failsafe, and implement serial connection lost failsafe
//and pwm_in loss failsafe

void loop() {
  // put your main code here, to run repeatedly:
  bool rc_valid = ReadRc();
  bool pwm_in_valid = ReadPwmIn();
  ReadSwitches();
  bool serial_valid = ReadSerial(targetPulseWidth);
  
  // Update failsafe status based on valid data and timeouts
  UpdateFailsafeStatus(rc_valid, serial_valid, pwm_in_valid);
  
  Run();

  // Send state data only if it has changed
  if (StateDataChanged(stateData, prevStateData)) {
    SendData(stateData);
    prevStateData = stateData;  // Update previous state
  }

  //DebugSerial();  
}

void Run(void) {
  // stage state information for serial transmission
  stateData.no_rc = SwitchGetNoRc();
  if (stateData.no_rc) {
    stateData.ams = SwitchGetAms();
    stateData.start = SwitchGetStart();
    stateData.mission = 0;
  } else {
    stateData.ams = RcGetAms();
    stateData.start = RcGetStart();
    stateData.mission = RcGetMission();
  }

  if (stateData.ams) mode = AUTO;
  else if (stateData.no_rc) mode = MANUAL;
  else mode = RC;

  if (RcIsFailsafe()) rc_failsafe = true;

  if (!stateData.no_rc && (RcGetEstop() || estop_triggered)) {
    mode = FAILSAFE;
    stateData.failsafe = E_STOP_FAILSAFE;
    estop_triggered = true;
  } else if (!stateData.no_rc && rc_failsafe) {
    mode = FAILSAFE;
    stateData.failsafe = RC_FAILSAFE;
  } else if (stateData.ams && serial_failsafe) {
    mode = FAILSAFE;
    stateData.failsafe = SERIAL_FAILSAFE;
  } else if (stateData.no_rc && pwm_in_failsafe && !stateData.ams) {
    mode = FAILSAFE;
    stateData.failsafe = PWM_IN_FAILSAFE;
  } else {
    stateData.failsafe = NO_FAILSAFE;
  }

  if (estop_triggered && stateData.no_rc) {
    estop_triggered = false;  // Reset emergency stop when in manual mode
  }
  // Execute actions based on the current mode
  switch (mode) {
    
    case FAILSAFE:
      SetMotorPwm(MOTOR_PWM_PULSE_NEUTRAL);
      SetSteerPwm(STEER_PWM_PULSE_NEUTRAL);
      break;

    case MANUAL:
      SetMotorPwm(GetPulseWidthPwmIn());
      SetSteerPwm(0);
      break;

    case RC:
      SetMotorPwm(RcGetThrottle());
      SetSteerPwm(RcGetSteer());
      break;

    case AUTO:
      if (stateData.start) {
        SetMotorPwm(targetPulseWidth.motor);
        SetSteerPwm(targetPulseWidth.steer);
      } else {
        SetMotorPwm(MOTOR_PWM_PULSE_NEUTRAL);
        SetSteerPwm(STEER_PWM_PULSE_NEUTRAL);
      }
      break;

    default:
      // Handle unexpected mode
      SetMotorPwm(MOTOR_PWM_PULSE_NEUTRAL);
      SetSteerPwm(STEER_PWM_PULSE_NEUTRAL);
      break;
  }
}

// void DebugSerial(void) {
//   static time_t last_display_time = 0;
//   if (millis() - last_display_time > 500) {
//     last_display_time = millis();
//     DisplayRc();
//     DisplayPwmIn();
//     DisplaySwitches();
//   }
// }

// // put function definitions here:
// void DisplayRc(void) {
//   // Display RC data
//   Serial.print("Throttle: ");
//   Serial.print(RcGetThrottle());
//   Serial.print("\tSteer: ");
//   Serial.print(RcGetSteer());
//   Serial.print("\tMission: ");
//   Serial.print(RcGetMission());
//   Serial.print("\tStart: ");
//   Serial.print(RcGetStart());
//   Serial.print("\tAMS: ");
//   Serial.print(RcGetAms());
//   Serial.print("\tSteer-Zero: ");
//   Serial.print(RcGetSteerZero());
//   Serial.print("\tEstop A: ");
//   Serial.print(RcGetEstop());
//   Serial.println();
// }
// void DisplayPwmIn(void) {
//   // Display PWM input data
//   Serial.print("PWM Pulse Width: ");
//   Serial.print(GetPulseWidthPwmIn());
//   Serial.println();
// }

// void DisplaySwitches(void) {
//   // Display switch states
//   SwitchData switches = SwitchGetAllData();
//   Serial.print("Error: ");
//   Serial.print(switches.error);
//   Serial.print("\tStatus: ");
//   Serial.print(switches.status);
//   Serial.print("\tStart: ");
//   Serial.print(switches.start);
//   Serial.print("\tAMS: ");
//   Serial.print(switches.ams);
//   Serial.println();
// }