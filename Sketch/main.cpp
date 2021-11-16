// For Arduino comment out the next line (it's auto-included)
#include <Arduino.h>

// Rotary encoder pins
// For Arduino use pins 2 & 3 for Pin A and Pin B, any other GPIO for Push Button
#define PIN_A 32
#define PIN_B 4
#define PUSH_BTN 27

// A detent counter for the rotary encoder (negative = anti-clockwise)
volatile int rotationCounter = 0;

// Flag from interrupt routine (moved=true)
volatile bool rotaryEncoder = false;

// Store latest PIN_B value
volatile uint8_t PIN_B_VALUE = 0;

// Interrupt for PIN A
// For Arduino remove the IRAM_ATTR bit
void IRAM_ATTR rotaryA() {
    // Only allow interrupts as per datasheet: 60rpm = 1rps = 12pps = 1 pulse every 40mS
    static unsigned long isrMillis = 0;

    // We can adjust the Real World delay-between-pulses to suit
    // I've found it actually works with a pulse every 20mS
    if (millis() - isrMillis >= 20) {
        // Set flag
        rotaryEncoder = true;

        // Clockwise
        if (digitalRead(PIN_A)) {
            if (digitalRead(PIN_B) == LOW) {
                // if (PIN_B_VALUE == 0) {
                rotationCounter++;
            } else {
                rotationCounter--;
            }
        }

        // Anti-clockwise
        if (!digitalRead(PIN_A)) {
            if (digitalRead(PIN_B) == HIGH) {
                // if (PIN_B_VALUE == HIGH) {
                rotationCounter++;
            } else {
                rotationCounter--;
            }
        }
    } else {
        Serial.println("Too fast (ignored)");
    }

    // Update when we were here (same value as on entry)
    isrMillis = millis();
}

void setup() {
    Serial.begin(115200);

    // Outputs are controlled by 74HAC14
    pinMode(PIN_A, INPUT);
    pinMode(PIN_B, INPUT);

    // We need to monitor a change on PIN A (rising and falling)
    attachInterrupt(digitalPinToInterrupt(PIN_A), rotaryA, CHANGE);
    Serial.println("Setup completed - turn the rotary encoder!");
}

void loop() {
    // Track the rotation value
    static int prevRotationCounter = 0;

    // Has rotary encoder moved?
    if (rotaryEncoder) {
        rotaryEncoder = false;

        // On an ESP32 printf is built-in so we will use it.
        // On an Arduino load the printf library or use Serial.print
        // See https://youtu.be/lhwk5vJ1iMA for details

        // For Arduino uncomment the two lines below
        // Serial.print(rotationCounter < prevRotationCounter ? "L" : "R");
        // Serial.print(rotationCounter);

        // For Arduino comment out the next line
        printf(rotationCounter < prevRotationCounter ? "L%d\n" : "R%d\n", rotationCounter);

        // Track the rotation value
        prevRotationCounter = rotationCounter;
    }

    // Push button?
    if (digitalRead(PUSH_BTN) == HIGH) {
        rotationCounter     = 0;
        prevRotationCounter = 0;
        Serial.println("Reset to ZERO");

        // Wait until button released before continuing (blocking!)
        while (digitalRead(PUSH_BTN))
            ;
    }
}