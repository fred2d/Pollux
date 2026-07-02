#include "switches.h"


// Debounce state tracking structure
struct DebounceState {
    bool confirmed_state;  // Last confirmed state
    bool raw_state;        // Current raw reading
    uint8_t counter;       // Debounce counter (incremented when state matches, reset on change)
};

// Static storage for each switch's debounce state
static DebounceState error_debounce = {false, false, 0};
static DebounceState status_debounce = {false, false, 0};
static DebounceState start_debounce = {false, false, 0};
static DebounceState ams_debounce = {false, false, 0};
static DebounceState no_rc_debounce = {false, false, 0};

static SwitchData switch_data = {false, false, false, false, false};

/**
 * Helper function to apply debouncing logic to a switch
 * Returns true if state has changed and is confirmed
 */
static bool DebounceSwitch(DebounceState& debounce, bool raw_reading) {
    if (raw_reading == debounce.raw_state) {
        // Same raw state as before, increment counter
        if (debounce.counter < DEBOUNCE_COUNT) {
            debounce.counter++;
            // When counter reaches threshold, confirm the state
            if (debounce.counter == DEBOUNCE_COUNT && raw_reading != debounce.confirmed_state) {
                debounce.confirmed_state = raw_reading;
                return true;  // State changed and confirmed
            }
        }
    } else {
        // Raw state changed, reset counter
        debounce.raw_state = raw_reading;
        debounce.counter = 1;  // Start counting from 1 (first read of new state)
    }
    return false;  // No confirmed change
}

void InitSwitches(void) {
    // Initialize switch pins as inputs
    pinMode(ERROR_PIN, INPUT);
    pinMode(STATUS_PIN, INPUT);
    pinMode(START_PIN, INPUT);
    pinMode(AMS_PIN, INPUT_PULLDOWN);
    pinMode(NO_RC_PIN, INPUT_PULLDOWN);
}

void ReadSwitches(void) {
    // Read raw switch states (inverted because INPUT_PULLUP means LOW = pressed)
    bool error_raw = digitalRead(ERROR_PIN);
    bool status_raw = digitalRead(STATUS_PIN);
    bool start_raw = digitalRead(START_PIN);
    bool ams_raw = digitalRead(AMS_PIN);
    bool no_rc_raw = digitalRead(NO_RC_PIN);

    // Apply debouncing to each switch
    DebounceSwitch(error_debounce, error_raw);
    DebounceSwitch(status_debounce, status_raw);
    DebounceSwitch(start_debounce, start_raw);
    DebounceSwitch(ams_debounce, ams_raw);
    DebounceSwitch(no_rc_debounce, no_rc_raw);

    // Update switch data structure with confirmed states
    switch_data.error = error_debounce.confirmed_state;
    switch_data.status = status_debounce.confirmed_state;
    switch_data.start = start_debounce.confirmed_state;
    switch_data.ams = ams_debounce.confirmed_state;
    switch_data.no_rc = no_rc_debounce.confirmed_state;
}

// Getter function implementations
bool SwitchGetError(void) {
    return switch_data.error;
}

bool SwitchGetStatus(void) {
    return switch_data.status;
}

bool SwitchGetStart(void) {
    return switch_data.start;
}

bool SwitchGetAms(void) {
    return switch_data.ams;
}

bool SwitchGetNoRc(void) {
    return switch_data.no_rc;
}

SwitchData SwitchGetAllData(void) {
    return switch_data;
}