/*
ESP8266 A4988 Stepper Motor Control with button
*/

#include <AccelStepper.h>

#define DIR 14
#define STEP 12
#define BUTTON 4

// initialize the stepper library
AccelStepper stepper(AccelStepper::DRIVER, DIR, STEP);

// State machine: 0=STOP, 1=FORWARD, 2=STOP, 3=REVERSE
int motorState = 0;

// Button state variables
bool lastButtonState = HIGH;
bool currentButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;  // 50ms debounce delay

void setup() {
  // initialize the serial port
  Serial.begin(115200);

  // Set up button pin with internal pull-up
  pinMode(BUTTON, INPUT_PULLUP);
  
  stepper.setMaxSpeed(1000);
  stepper.setSpeed(1000);
  stepper.setAcceleration(100);
  Serial.println("Stepper motor control ready. Press button to cycle: Forward -> Stop -> Reverse -> Stop");
}

void loop() {
  int reading = digitalRead(BUTTON);

  // Check if button state changed (for debouncing)
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  // If enough time has passed since last state change, consider it stable
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // If button state has changed
    if (reading != currentButtonState) {
      currentButtonState = reading;
      
      // Button pressed (LOW because of pull-up)
      if (currentButtonState == LOW) {
        // Advance to next state (0->1->2->3->0)
        motorState = (motorState + 1) % 4;
        
        switch(motorState) {
          case 0:  // STOP
            Serial.println("State: STOP");
            stepper.setSpeed(0);
            stepper.moveTo(stepper.currentPosition());
            break;
            
          case 1:  // FORWARD
            Serial.println("State: FORWARD - rotating forward");
            stepper.setSpeed(1000);  // Positive speed for forward
            stepper.moveTo(1000000);  // Large target for continuous rotation
            break;
            
          case 2:  // STOP
            Serial.println("State: STOP");
            stepper.setSpeed(0);
            stepper.moveTo(stepper.currentPosition());
            break;
            
          case 3:  // REVERSE
            Serial.println("State: REVERSE - rotating backward");
            stepper.setSpeed(-1000);  // Negative speed for reverse
            stepper.moveTo(-1000000);  // Large negative target for continuous reverse rotation
            break;
        }
      }
    }
  }

  // Update last button state
  lastButtonState = reading;

  // Run the stepper motor (only moves if target is different from current position)
  stepper.run();
}