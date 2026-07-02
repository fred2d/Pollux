# Pollux
Firmware for controlling the low-level hardware of the UAntwerp Solar Boat.

## Overview
Pollux manages motor and steering PWM output, RC receiver input (SBUS), physical switch inputs, and serial communication with the main control system. The firmware supports multiple operation modes (Manual, RC, Autonomous) with automatic failsafe protection.

## Pin Mapping

| Component | Pin | Direction | Notes |
|-----------|-----|-----------|-------|
| **SBUS Receiver** | 7 | Input | UART RX (cannot be changed) |
| **PWM Input** | 2 | Input | From manual controller or PWM source |
| **PWM Motor Output** | 3 | Output | 400 Hz, 800-2350 µs |
| **PWM Steering Output** | 5 | Output | 50 Hz, 800-2200 µs, could be 400 Hz in reality |
| **Physical Switch: ERROR** | 4 | Input  | Active HIGH |
| **Physical Switch: STATUS** | 8 | Input  | Active HIGH |
| **Physical Switch: START** | 9 | Input  | Active HIGH |
| **Physical Switch: NO_RC** | 22 | Input (Pull-down) | Active HIGH (Manual mode enable) |
| **Physical Switch: AMS** | 23 | Input (Pull-down) | Active HIGH (Autonomous mode enable) |

## Hardware Requirements & Warnings

### Voltage Compatibility ⚠️
**The Teensy 4.1 is NOT 5V tolerant.** All digital inputs must be at 3.3V logic levels. If connecting external devices with 5V outputs:
- **Use a logic level converter** on PWM input (pin 2) when sourcing from 5V devices
- **Use a logic level converter** on SBUS input (pin 7) if the receiver outputs 5V
- Do not connect 5V signals directly or risk permanent damage

### Motor/Steering Controller
- PWM output frequency: 400 Hz
- Pulse width range: 800-2350 µs (motor), 800-2200 µs (steering)
- Neutral position: 1575 µs (motor), 1500 µs (steering)

## Status & Testing ⚠️

### Firmware State
**This firmware has NOT been tested on actual hardware.** Before deployment:

1. **Verify RC Channel Mapping**: Confirm that each SBUS channel controls the intended function
   - Channel 1: Throttle
   - Channel 2: Steering
   - Channel 5: Start button
   - Channel 6: AMS (Autonomous mode)
   - Channels 7-10, 11-12: Mission bits and E-stop

2. **Verify Physical Button Actions**: The behavior of the physical switches (ERROR, STATUS, START, AMS, NO_RC) needs to be verified and documented

3. **Verify E-Stop Behavior**: The emergency stop logic (SBUS channels 11 & 12) requires verification

4. **Motor Controller Compatibility**: Ensure the motor and steering PWM signal ranges are compatible with your controllers

## PWM Configuration

All PWM thresholds are defined in `config.h` and should be verified against your specific hardware:

| Parameter | Motor PWM | Steering PWM | PWM Input |
|-----------|-----------|--------------|-----------|
| Min Pulse (µs) | 800 | 800 | 1100 |
| Max Pulse (µs) | 2350 | 2200 | 2050 |
| Neutral (µs) | 1575 | 1500 | N/A |

### ⚠️ Important: Range Mismatch
The **PWM input range (1100-2050 µs)** is narrower than the **motor output range (800-2350 µs)**. When using PWM input in Manual mode:
Verify this !!!

## Failsafe System

### Failsafe Types
The firmware monitors three independent input sources for data loss:

| Failsafe | Trigger Condition | Timeout | Recovery |
|----------|------------------|---------|----------|
| **RC Failsafe** | No SBUS data from RC receiver | 1000 ms | Automatic when RC signals resume |
| **Serial Failsafe** | No valid serial commands | 1000 ms | Automatic when serial resumes |
| **PWM Input Failsafe** | No valid PWM edge detection | 1000 ms | Automatic when PWM signals resumes |

### Failsafe Behavior
When any failsafe is triggered:
- Operating mode switches to **FAILSAFE**
- Motor PWM output → neutral (1575 µs)
- Steering PWM output → neutral (1500 µs)
- Vehicle stops and becomes unresponsive to control inputs

### Recovery
Failsafes are automatically lifted once valid data resumes from the failing source. No manual reset is required.
RC E-stop failsafe is resetted by turning on NO-RC switch. 

### Timeout Configuration
Failsafe timeouts (in milliseconds) can be adjusted in `config.h`:
- `RC_FAILSAFE_TIMEOUT`
- `SERIAL_FAILSAFE_TIMEOUT`
- `PWM_IN_FAILSAFE_TIMEOUT`

## Known Limitations

### No PWM Ramping
The firmware applies PWM commands **directly without ramping or slewing**. Motor output changes instantly when control inputs change. This may cause jerky acceleration; consider adding ramping in future releases if smoother motion is desired.

### Unclear Button Actions
The following button behaviors are not yet fully defined and require clarification:
- **ERROR button**: Current action unclear; behavior needs to be specified
- **STATUS button**: Current action unclear; behavior needs to be specified
- **Unused physical buttons**: Not all physical buttons may be utilized; integration with failsafe system needs clarification

### Input Validation
Input values outside the configured thresholds are either clamped to valid ranges or rejected as invalid, depending on the source. Review the thresholds in `config.h` carefully for your use case.

## Operation Modes

The firmware supports four operation modes:

1. **FAILSAFE**: Triggered by data loss; outputs neutral PWM values
2. **MANUAL**: Manual controller input via PWM (pin 2) when NO_RC switch is active
3. **RC**: SBUS receiver control; default mode when RC signal is present
4. **AUTO**: Autonomous mode via serial commands when AMS switch is active

Mode selection is automatic based on switch states and failsafe conditions.

## Serial Communication

- **Baud Rate**: 460800
- **Protocol**: Custom binary format (see `serial_com.h`)
- **Timeout**: 1000 ms for failsafe trigger

## Future Work

- [ ] Hardware testing and validation
- [ ] RC channel and button action verification
- [ ] PWM ramping/slewing implementation
- [ ] Clarify emergency stop and button behavior
- [ ] Add telemetry/logging capability
