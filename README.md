# Pollux
Firmware for controlling the low-level hardware of a autonomous boat

## Overview
Pollux manages motor and steering PWM output, RC receiver input (SBUS), physical switch inputs, and serial communication with the main control system. The firmware supports multiple operation modes (Manual, RC, Autonomous) with automatic failsafe protection.

## Pin Mapping

| Component | Pin | Direction | Notes |
|-----------|-----|-----------|-------|
| **SBUS Receiver** | 7 | Input | UART RX (can not be changed) |
| **PWM Input** | 2 | Input | From manual controller or PWM source |
| **PWM Motor Output** | 3 | Output | 400 Hz, 800-2350 µs |
| **PWM Steering Output** | 5 | Output | 50 Hz, 800-2200 µs, could be 400 Hz in reality |
| **Physical Switch: ERROR** | 4 | Input  | Active HIGH, debounced |
| **Physical Switch: STATUS** | 8 | Input  | Active HIGH, debounced |
| **Physical Switch: START** | 9 | Input (pull-dwon) | Active HIGH, debounced |
| **Physical Switch: NO_RC** | 22 | Input (Pull-down) | Active HIGH, debounced |
| **Physical Switch: AMS** | 23 | Input (Pull-down) | Active HIGH, debounced |

*Pin definitions can be edited in `pins.h`.*


## Serial Communication

- **Baud Rate**: 460800

### Read
The following format is expected: `M 1234 S 4567`.
   
- order can be mixed and other commands can be added, but will be ignored

### Send
The following format is sent: `A x S x R x F i M i`.

With `x` being a bool 0 or 1 and i an intiger.
|Token| Description| 
|---|-----|
| **A** | AMS |
| **S** | Start |
| **R** | NO_RC |
| **F** | Failsafe |
| **M** | Mission |
   
|Failsafe | i |
|-----|-|
|NO_FAILSAFE | 0
|RC_FAILSAFE | 1
|  SERIAL_FAILSAFE | 2
| PWM_IN_FAILSAFE | 3
|E_STOP_FAILSAFE | 4

## Normal operation
1) NO_RC switch is read
2) Based on the NO_RC state either the physical buttons or the rc inputs are used for AMS, START and MISSION.
3) The mode is the result of following truth table
   
   | NO_RC | AMS | START | mode |
   |-------|-----|-------|----------|
   | *HIGH* | *LOW* | *X* | MANUAL |
   | *LOW* | *LOW* | *X* | RC |
   | *X* | *HIGH* | *X* | AUTO |

### Mode definitions

| Mode | START | PWM Steer | PWM Motor |
|------|-------|--------|-------|
|MANUAL| X| 0 | PWM_IN |
|RC |X| RC_STEER | RC_MOTOR |
|AUTO | LOW | NEUTRAL | NEUTRAL |
|AUTO | HIGH | SERIAL_STEER | SERIAL_MOTOR|

## RC E-stop
Only active when NO_RC is low.
When momentarily high the e-stop is triggered and neutral values are applied.
RC E-stop failsafe is resetted by turning on NO-RC switch. 

## Failsafe System

### Failsafe Types
The firmware monitors three independent input sources for data loss:

| Failsafe | Trigger Condition | Timeout | Recovery |
|----------|------------------|---------|----------|
| **RC Failsafe** | No valid SBUS data from RC receiver | 1000 ms | Automatic when RC signals resume |
| **Serial Failsafe** | No valid serial commands | 1000 ms | Automatic when serial resumes |
| **PWM Input Failsafe** | No valid PWM edge detection | 1000 ms | Automatic when PWM signals resumes |

### Failsafe Behavior
When any failsafe is triggered:
- Operating mode switches to **FAILSAFE**
- Motor PWM output → neutral (1575 µs)
- Steering PWM output → neutral (1500 µs)
- Vehicle stops and becomes unresponsive to control inputs

**Failsafes will only trigger in its relevant states / mode**

### Timeout Configuration
Failsafe timeouts (in milliseconds) can be adjusted in `config.h`:
- `RC_FAILSAFE_TIMEOUT`
- `SERIAL_FAILSAFE_TIMEOUT`
- `PWM_IN_FAILSAFE_TIMEOUT`



## Hardware Requirements & Warnings

### Voltage Compatibility ⚠️
**The Teensy 4.1 is NOT 5V tolerant.** All digital inputs must be at 3.3V logic levels. If connecting external devices with 5V outputs:
- **Use a logic level converter** on PWM input (pin 2) when sourcing from 5V devices
- **Use a logic level converter** on PWM output (pin 3) if the motor controller expects 5V

### PWM Range Mismatch
The **PWM input range (1100-2050 µs)** is narrower than the **motor output range (800-2350 µs)**.

***Verify this !!!***
*Ranges can be viewed in `config.h`*

## Status & Testing ⚠️

### Firmware State
**This firmware has NOT been tested on actual hardware.** Before deployment:

1. **Verify RC Channel Mapping**: Confirm that each SBUS channel controls the intended function and direction

2. **Verify Physical Button Actions**: The behavior of the physical switches (ERROR, STATUS, START, AMS, NO_RC) needs to be verified

4. **Motor Controller Compatibility**: Ensure the motor and steering PWM signal ranges are compatible with the controller

## PWM Configuration

All PWM thresholds are defined in `config.h` and should be verified against your specific hardware:

| Parameter | Motor PWM | Steering PWM | PWM Input |
|-----------|-----------|--------------|-----------|
| Min Pulse (µs) | 800 | 800 | 1100 |
| Max Pulse (µs) | 2350 | 2200 | 2050 |
| Neutral (µs) | 1575 | 1500 | N/A |

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

## Future Work

- [ ] Hardware testing and validation
- [ ] RC channel and button action verification
- [ ] PWM ramping/slewing implementation
- [ ] Clarify emergency stop and button behavior
- [ ] Add telemetry/logging capability
