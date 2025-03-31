#include <Joystick.h>
#include "interim_sketch.h"

// Pin definitions
PinConfig pin_configs[2] = {
  { // left side pin configuration
    .IDPin = {4, 5, 6},   // IDPin0, IDPin1, IDPin2
    .DataPin = {2, 3, A0} // DataPin0, DataPin1, DataPin2
  },
  { // right side pin configuration
  .IDPin = {9, 10, 11},   // IDPin0, IDPin1, IDPin2
  .DataPin = {8, 7, A1}   // DataPin0, DataPin1, DataPin2
  }
};

// Joystick setup
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,
                  JOYSTICK_TYPE_GAMEPAD,
                  2, 0,                        // 2 button, no hats      UP TO 2 BUTTONS AT ONCE
                  true, true, false,           // X, Y, Z                UP TO 2 ENCODERS AT ONCE
                  false, false, false,         // Rx, Ry, Rz
                  true,                        // Throttle (or rotation) POTENTIOMETER
                  false, false, false, false); // Rudder, Accelerator, Brake, Steering

// global variables to hold state
volatile int encoderPosition = 0;       // the count up/down
volatile unsigned long lastClickTime = 0; // track time of last full "click"
volatile unsigned long clickInterval = 0; // time between clicks (for speed)

// storing last known 2-bit state 
byte lastState = 0;

void setup() {
  // Set mode selection pins as inputs
  for (Side side = 0; side < 2; side = side + 1)
  {
    for (int idpin = 0; idpin < 3; idpin++)
    {
        pinMode(pin_configs[side].IDPin[idpin], INPUT_PULLUP);
    }
  }

  // analog pin config
  pinMode(pin_configs[0].DataPin[2], INPUT);
  // pinMode(pin_configs[1].DataPin[2], INPUT);

  // digital pin config
  pinMode(pin_configs[0].DataPin[0], INPUT_PULLUP);
  pinMode(pin_configs[0].DataPin[1], INPUT_PULLUP);
  // pinMode(pin_configs[1].DataPin[0], INPUT_PULLUP);
  // pinMode(pin_configs[1].DataPin[1], INPUT_PULLUP);

  // initialize the lastState based on the current reading
  lastState = (digitalRead(pin_configs[0].DataPin[0])) | digitalRead(pin_configs[0].DataPin[1]);

  Joystick.begin();
}

void loop() {
  // for (Side side = 0; side < 2; side = side + 1) {
  //   Module modInserted = readModule(side);
  //   Module_Type modType = get_type(modInserted);

  //   if (modType == TYPE_BUTTON) {
  //     int buttonStatus = !digitalRead(pin_configs[side].DataPin[0]);
  //     Serial.print("Fire Gun Button status: ");
  //     Serial.println(buttonStatus);
  //     Joystick.setButton(0, buttonStatus);
  //   } else if (modType == TYPE_POTENTIOMETER){
  //     int potStatus = analogRead(pin_configs[side].DataPin[2]);
  //     int mappedPotStatus = map(potStatus, 0, 1023, 0, 255); 
  //     Serial.print("Mapped Speed Potentiometer status: ");
  //     Serial.println(mappedPotStatus);
  //     Joystick.setYAxis(10*mappedPotStatus);
  //   } else if (modType == TYPE_ENCODER) {
  //     int statusA = digitalRead(pin_configs[side].DataPin[0]);
  //     int statusB = digitalRead(pin_configs[side].DataPin[1]);

  //     byte newState = statusA << 1 | statusB;

  //     if (newState != lastState) {
  //       byte combined = (lastState << 2) | newState;
  //       if (newState == 0b00) {
  //         byte oldA = (lastState >> 1) & 1;
  //         byte oldB = lastState & 1;
  //         byte currentA = (newState >> 1) & 1;
  //         byte currentB = newState & 1;
  //         if (lastState == 0b10) {
  //           // Clockwise
  //           encoderPosition++;
  //           unsigned long now = micros();
  //           clickInterval = now - lastClickTime;
  //           lastClickTime = now;
  //           Serial.print("Clockwise, Position = ");
  //           Serial.print(encoderPosition);
  //           Serial.print(", time interval (us) = ");
  //           Serial.println(clickInterval);
  //         }
  //         else if (lastState == 0b01) {
  //           // Counterclockwise
  //           encoderPosition--;
  //           unsigned long now = micros();
  //           clickInterval = now - lastClickTime;
  //           lastClickTime = now;
  //           Serial.print("Counterclockwise, Position = ");
  //           Serial.print(encoderPosition);
  //           Serial.print(", time interval (us) = ");
  //           Serial.println(clickInterval);
  //         }
  //       }
  //     }
  //     lastState = newState;
  //     Joystick.setXAxis(encoderPosition);

  //   } else {
  //     Serial.println("No module attached, no data!");
  //   }
  // }
  Side side = 0;
  Module leftModule = readModule(side);
  Module_Type leftModuleType = get_type(leftModule);

  if (leftModuleType == TYPE_BUTTON) {
    int buttonStatus = !digitalRead(pin_configs[side].DataPin[0]);
    Serial.print("Fire Gun Button status: ");
    Serial.println(buttonStatus);
    Joystick.setButton(0, buttonStatus);
  } else if (leftModuleType == TYPE_POTENTIOMETER){
    int potStatus = analogRead(pin_configs[side].DataPin[2]);
    int mappedPotStatus = map(potStatus, 0, 1023, 0, 255); 
    Serial.print("Mapped Speed Potentiometer status: ");
    Serial.println(mappedPotStatus);
    Joystick.setYAxis(10*mappedPotStatus);
  } else if (leftModuleType == TYPE_ENCODER) {
    int statusA = digitalRead(pin_configs[side].DataPin[0]);
    int statusB = digitalRead(pin_configs[side].DataPin[1]);

    byte newState = statusA << 1 | statusB;

    if (newState != lastState) {
      byte combined = (lastState << 2) | newState;
      if (newState == 0b00) {
        byte oldA = (lastState >> 1) & 1;
        byte oldB = lastState & 1;
        byte currentA = (newState >> 1) & 1;
        byte currentB = newState & 1;
        if (lastState == 0b10) {
          // Clockwise
          encoderPosition++;
          unsigned long now = micros();
          clickInterval = now - lastClickTime;
          lastClickTime = now;
          Serial.print("Clockwise, Position = ");
          Serial.print(encoderPosition);
          Serial.print(", time interval (us) = ");
          Serial.println(clickInterval);
        }
        else if (lastState == 0b01) {
          // Counterclockwise
          encoderPosition--;
          unsigned long now = micros();
          clickInterval = now - lastClickTime;
          lastClickTime = now;
          Serial.print("Counterclockwise, Position = ");
          Serial.print(encoderPosition);
          Serial.print(", time interval (us) = ");
          Serial.println(clickInterval);
        }
      }
    }
    lastState = newState;
    Joystick.setXAxis(encoderPosition);

  } else {
    Serial.println("No module attached, no data!");
  }
}

/**************** HELPER FUNCTIONS ****************/

// Given a module, return its type (button, potentiometer, or encoder)
Module_Type get_type(Module mod)
{
  switch (mod)
  {
    case MOD_SPEED:
      return TYPE_POTENTIOMETER;
    case MOD_STEER:
    case MOD_AIM:
    case MOD_SHIELD:
      return TYPE_ENCODER;
    case MOD_SHOOT:
    case MOD_CHARGE:
      return TYPE_BUTTON;
    case MOD_NONE:
      return TYPE_NONE;
  }
}

// Dynamically reads the module from D6, D5, D4
enum Module readModule(enum Side side)
{
  int moduleSelect = (digitalRead(pin_configs[side].IDPin[0])) |
                     (digitalRead(pin_configs[side].IDPin[1]) << 1) |
                     (digitalRead(pin_configs[side].IDPin[2]) << 2);

  switch (moduleSelect)
  {
    case 0b001:
      return MOD_STEER; // Steering (Encoder)
    case 0b010:
      return MOD_AIM; // Aim (Gun) (Encoder)
    case 0b011:
      return MOD_SHIELD; // Aim (Shield) (Encoder)
    case 0b100:
      return MOD_SPEED; // Adjust Speed (Potentiometer)
    case 0b101:
      return MOD_SHOOT; // Fire Gun (Button)
    case 0b110:
      return MOD_CHARGE; // Charge Battery (Button)
    default:
      return MOD_NONE; // Default to no module
  }
}

