#ifndef SWITCHES_H
#define SWITCHES_H

#include "Arduino.h"
#include "pins.h"
/**
 * Debounced switch data structure
 * Uses counter-based debouncing: a change is only confirmed after 
 * the same raw state is read DEBOUNCE_COUNT times consecutively
 */
struct SwitchData {
    bool error;      // ERROR switch state (debounced)
    bool status;     // STATUS switch state (debounced)
    bool start;      // START switch state (debounced)
    bool ams;        // AMS switch state (debounced)
    bool no_rc;      // NO_RC switch state (debounced)
};

#define DEBOUNCE_COUNT 3  // Number of consistent reads required to confirm state change

// Public functions
void InitSwitches(void);
void ReadSwitches(void);  // Call frequently (e.g., in main loop at 10+ Hz)

// Getter functions for debounced switch states
bool SwitchGetError(void);
bool SwitchGetStatus(void);
bool SwitchGetStart(void);
bool SwitchGetAms(void);
bool SwitchGetNoRc(void);

SwitchData SwitchGetAllData(void);

#endif