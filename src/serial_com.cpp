#include "Serial_com.h"
const unsigned int MAX_MESSAGE_LENGTH = 64; // Increased slightly to accommodate extra variables

void InitSerial(unsigned long baudRate) {
    Serial.begin(baudRate);
}

bool ReadSerial(ReceivedData &data) {
    static char message[MAX_MESSAGE_LENGTH];
    static unsigned int messagePos = 0;

    while (Serial.available() > 0) {
        char inByte = Serial.read();

        if (inByte == '\n') {
            message[messagePos] = '\0';
            messagePos = 0; // Reset for next time

            bool foundM = false;
            bool foundS = false;
            int tempM = 0;
            int tempS = 0;

            // strtok modifies the string, so we tokenize by spaces
            char* token = strtok(message, " ");
            
            while (token != NULL) {
                // Check if the current token is "M"
                if (strcmp(token, "M") == 0) {
                    token = strtok(NULL, " "); // Move to the next token (the value)
                    if (token != NULL) {
                        tempM = atoi(token);
                        foundM = true;
                    }
                } 
                // Check if the current token is "S"
                else if (strcmp(token, "S") == 0) {
                    token = strtok(NULL, " "); // Move to the next token (the value)
                    if (token != NULL) {
                        tempS = atoi(token);
                        foundS = true;
                    }
                }
                
                // Move to the next token in the loop
                token = strtok(NULL, " ");
            }

            // Only return true if we successfully found both required metrics
            if (foundM && foundS) {
                // check if values lay within valid range.
                if (tempM < MOTOR_PWM_PULSE_MIN || tempM > MOTOR_PWM_PULSE_MAX) return false;
                if (tempS < STEER_PWM_PULSE_MIN || tempS > STEER_PWM_PULSE_MAX) return false;
                data.motor = tempM;
                data.steer = tempS;
                return true;
            }
            return false; 
        }
        else if (inByte != '\r') {
            if (messagePos < MAX_MESSAGE_LENGTH - 1) {
                message[messagePos] = inByte;
                messagePos++;
            } else {
                messagePos = 0; // Overflow protection
            }
        }
    }
    return false;
}

void SendData(TransmitData &data) {
    Serial.print("A ");
    Serial.print(data.ams ? 1 : 0);
    Serial.print(" S ");
    Serial.print(data.start ? 1 : 0);
    Serial.print(" R ");
    Serial.print(data.no_rc ? 1 : 0);
    Serial.print(" F ");
    Serial.print(data.failsafe);
    Serial.print(" M ");
    Serial.println(data.mission);
}