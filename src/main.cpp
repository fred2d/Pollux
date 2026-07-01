#include <Arduino.h>

// Board is a Teensy 4.1-NE without Ethernet feature

const int pwmSteerPin = 3;
const int pwmMotorPin = 4;
const int pwmInPin    = 5;

const int errorIn = 7;
const int statusIn = 8;
const int startIn = 9;
const int amsIn = 23;

const int ARRAY_SIZE = 31;
int readingsSF[ARRAY_SIZE];
int readingsSD[ARRAY_SIZE];
int readingsAMS[ARRAY_SIZE];
int bufIndex = 0;

uint32_t motor_us = 1575;
uint32_t motor_neutral = 1575;
uint32_t steer_us = 1500;
uint32_t steer_neutral = 1500;
volatile uint32_t pwmIn_us = 1575;
volatile uint32_t last_pwm = 1575;

// === Configuration parameters ===
const unsigned int LOOP_INTERVAL_MS        = 10;    // main loop period
const unsigned int PRINT_INTERVAL_MS       = 50;  // print status every X ms
const unsigned int MOVAVG_WINDOW           = 50;   // number of samples in moving average
const unsigned int RELAY_SWITCH_DELAY_MS   = 50;  // break-before-make delay
const unsigned int DEAD_BAND               = 50;   // ±50 µs deadband half-width
const unsigned int CENTER_PULSE            = 1575; // center of deadband window (µs)

const float alpha               = 0.005f;
const float SLEW_RATE_US_PER_S  = 200.0f;
unsigned int EMAvgPWM           = 1575;
float slewedPWM                 = EMAvgPWM;

const unsigned int MIN_PULSE = 1100;  // µs
const unsigned int MAX_PULSE = 2050;  // µs

static uint32_t lastPrint = 0;
static uint32_t last_us = 0;

bool error = false;
bool status = false;
bool start = false;
bool ams = false;

void pwmInISR() {
  static uint32_t riseTime = 0;
  if (digitalReadFast(pwmInPin)) {
    riseTime = micros();
  } else {
    pwmIn_us = micros() - riseTime;
  }
}

volatile uint32_t riseTime = 0;
//volatile uint32_t pwmIn_us = 1500;

volatile uint32_t riseCount = 0;
volatile uint32_t fallCount = 0;
char debug[20];

void pwmRiseISR() {
  riseCount++;
  riseTime = micros();
}

void pwmFallISR() {
  fallCount++;
  pwmIn_us = micros() - riseTime;
}

int getMajorityValue(int arr[]) {
  int count = 0;
  for (int i = 0; i < ARRAY_SIZE; i++) {
    count += arr[i];
  }
  // If more than half are 1s, majority is 1; otherwise 0
  return (count > ARRAY_SIZE / 2) ? 1 : 0;
}

void setup() {
  Serial.begin(460800);
  pinMode(pwmMotorPin, OUTPUT);
  pinMode(pwmSteerPin, OUTPUT);
  //pinMode(pwmInPin, INPUT);
  pinMode(pwmInPin, INPUT_PULLUP);
  pinMode(errorIn, INPUT);
  pinMode(statusIn, INPUT);
  pinMode(startIn, INPUT);
  pinMode(amsIn, INPUT);

  //attachInterrupt(digitalPinToInterrupt(pwmInPin), pwmInISR, CHANGE);
  //attachInterrupt(digitalPinToInterrupt(pwmInPin), pwmRiseISR, RISING);
  //attachInterrupt(digitalPinToInterrupt(pwmInPin), pwmFallISR, FALLING);

  attachInterrupt(digitalPinToInterrupt(pwmInPin), pwmFallISR, RISING);
  attachInterrupt(digitalPinToInterrupt(pwmInPin), pwmRiseISR, FALLING);
  
  analogWriteFrequency(pwmMotorPin, 400);
  analogWriteFrequency(pwmSteerPin, 50);
  analogWriteResolution(12);

  for (int i = 0; i < ARRAY_SIZE; i++) {
    readingsSF[i] = 0;
    readingsSD[i] = 0;
    readingsAMS[i] = 0;
  }

}

void loop() {
  // unsigned long loopStart = millis();

  /*if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    int mIndex = line.indexOf('M'); // Index for Motor Out PWM
    int sIndex = line.indexOf('S'); // Index for Steer Out PWM

    if (mIndex >= 0) {
      motor_us = line.substring(mIndex + 1).toInt();
    }
    if (sIndex >= 0) {
      steer_us = line.substring(sIndex + 1).toInt();
    }
  }*/
  static String line = "";

  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      int mIndex = line.indexOf('M');
      int sIndex = line.indexOf('S');

      if (mIndex >= 0) motor_us = line.substring(mIndex + 1).toInt();
      if (sIndex >= 0) steer_us = line.substring(sIndex + 1).toInt();

      line = "";
    } else {
      line += c;
    }
  }

  // --- 1) Read raw pulseWidth atomically ---
  /*noInterrupts();
    unsigned int rawPW = pwmIn_us;
  interrupts();*/
  uint32_t rawPW = pulseIn(pwmInPin, HIGH, 30000);
  //Serial.println(rawPW);

  if (rawPW == 0) {
      rawPW = last_pwm;   // signal lost
  }

  // --- Clamp to allowed range: if outside, use CENTER_PULSE ---
  if (rawPW < MIN_PULSE || rawPW > MAX_PULSE) {
    rawPW = last_pwm;
  }
  last_pwm = rawPW;
  //Serial.println(rawPW);

  /*if (millis() - lastPrint > 500) {
    Serial.print("raw: ");
    Serial.println(rawPW);
  }*/

  // 2.1) Slew-rate limiting on the EMA output (µs per second)

  uint32_t now_us = micros();
  float dt_s;
  if (last_us == 0) {
    dt_s = 0.0f;        // first iteration
  } else {
    dt_s = (now_us - last_us) * 1e-6f;
  }
  last_us = now_us;
  if (dt_s > 0.1f) dt_s = 0.1f;   // cap at 100 ms

  float maxDelta = SLEW_RATE_US_PER_S * dt_s;
  float target   = (float)rawPW;
  if (slewedPWM + maxDelta < target) {
    slewedPWM += maxDelta;
  }
  else if (slewedPWM - maxDelta > target) {
    slewedPWM -= maxDelta;
  }
  else {
    slewedPWM = target;
  }
  unsigned int filteredPWM = (unsigned int)(slewedPWM + 0.5f);

/*
  if (millis() - lastPrint > 500) {
    Serial.print("filteredPWM: ");
    Serial.println(filteredPWM);
  }*/

  status  = digitalRead(statusIn);

  readingsSF[bufIndex] = digitalRead(startIn);
  readingsSD[bufIndex] = digitalRead(errorIn);
  readingsAMS[bufIndex] = digitalRead(amsIn);
  bufIndex = (bufIndex + 1) % ARRAY_SIZE;

  start = getMajorityValue(readingsSF);
  error = getMajorityValue(readingsSD);
  ams = getMajorityValue(readingsAMS);

  // analogWriteFrequency(pwmMotorPin, 400);
  // analogWriteFrequency(pwmSteerPin, 400);

  bool commandInvalid = motor_us <= 500 || steer_us <= 500;
  bool systemFault = error == HIGH;
  bool systemDisabled = ams == LOW;
  bool systemReady = ams == HIGH && start == LOW;

  if (systemReady && !systemFault) { // Ready mode
    analogWrite(pwmMotorPin, constrain(map(motor_neutral, 0, 2500, 0, 4095), 0, 4095)); 
    analogWrite(pwmSteerPin, constrain(map(steer_neutral, 0, 2500, 0, 4095), 0, 4095));
  } else if (commandInvalid || systemFault || systemDisabled) { // Manual mode
    analogWrite(pwmMotorPin, constrain(map(filteredPWM, 0, 2500, 0, 4095), 0, 4095));
    analogWrite(pwmSteerPin, 0);
  } else { // Autonomous mode
    analogWrite(pwmMotorPin, constrain(map(motor_us, 0, 2500, 0, 4095), 0, 4095)); 
    analogWrite(pwmSteerPin, constrain(map(steer_us, 0, 2500, 0, 4095), 0, 4095)); // 255 if not 12 bit
  }
  

  if (millis() - lastPrint > 1000) {
    lastPrint = millis();
    /*Serial.print("F "); // Feedback (Motor PWM from MC)
    Serial.println(filteredPWM);
    Serial.print("E "); // Error status from PLC
    Serial.println(error);
    Serial.print("P "); // PLC Status
    Serial.println(status);*/
    Serial.print("S "); // Start switch state
    if (start == LOW || systemDisabled) {
      Serial.println("0");
    } else {
      Serial.println("1");
    }
    Serial.print("A ");
    if (!systemDisabled) {
      Serial.println("1");
    } else {
      Serial.println("0");
    }
    //Serial.println(digitalRead(pwmInPin));
    /*Serial.print("rise=");
    Serial.print(riseCount);
    Serial.print(" fall=");
    Serial.println(fallCount);*/
    /*Serial.print("C "); // Current PWM output
    Serial.println(steer_us);
    Serial.print("I "); // output as int
    Serial.println(constrain(map(steer_us, 0, 2500, 0, 255), 0, 255));*/
  }

  // --- 8) Enforce loop rate ---
  /*unsigned long elapsed = millis() - loopStart;
  if (elapsed < LOOP_INTERVAL_MS) {
    unsigned int time = (LOOP_INTERVAL_MS - elapsed)*1000;
    delayMicroseconds(time);
  }*/
}