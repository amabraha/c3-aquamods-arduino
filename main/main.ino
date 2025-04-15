#include <Joystick.h>
#include "main.h"

// Pin definitions
PinConfig pin_configs[2] = {
  { // left side pin configuration
    .IDPin = {4, 5, 6},   // IDPin0, IDPin1, IDPin2
    .DataPin = {2, 3, A0} // DataPin2, DataPin1, DataPin0
  },
  { // right side pin configuration
  .IDPin = {9, 10, 11},   // IDPin0, IDPin1, IDPin2
  .DataPin = {8, 7, A1}   // DataPin2, DataPin1, DataPin0
  }
};

// Joystick setup
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,
                  JOYSTICK_TYPE_JOYSTICK,
                  6, 0,                        // 6 button, no hats      UP TO 2 BUTTONS AT ONCE
                  true, true, false,            // X, Y, Z                UP TO 3 ENCODERS AT ONCE
                  false, false, false,         // Rx, Ry, Rz
                  false,                        // Throttle (or rotation) POTENTIOMETER
                  false, false, false, false); // Rudder, Accelerator, Brake, Steering

// 0, 1, 2: aim gun, aim shield, steer
volatile int encoderPosition[3] = {0, 0, 0}; // the count up/down
unsigned long lastClickTime[3] = {0, 0, 0}; // track time of last full "click"
unsigned long clickInterval[3] = {0, 0, 0}; // time between clicks (for speed)

int steerDirection;

// global variables to hold state
// volatile int encoderPosition = 0;         // the count up/down
// volatile unsigned long lastClickTime = 0; // track time of last full "click"
// volatile unsigned long clickInterval = 0; // time between clicks (for speed)

// storing last known 2-bit state 
byte lastState[3] = {0, 0, 0};

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
  pinMode(pin_configs[1].DataPin[2], INPUT);

  // digital pin config
  pinMode(pin_configs[0].DataPin[0], INPUT_PULLUP);
  pinMode(pin_configs[0].DataPin[1], INPUT_PULLUP);
  pinMode(pin_configs[1].DataPin[0], INPUT_PULLUP);
  pinMode(pin_configs[1].DataPin[1], INPUT_PULLUP);

  // initialize the lastState based on the current reading (for encoder)
  for (int i = 0; i < 3; i++) {
    lastState[i] = (digitalRead(pin_configs[0].DataPin[0])) | digitalRead(pin_configs[0].DataPin[1]);
  }
  
  Joystick.begin();
}

void loop() {
  //TODO: send automatic sending to false and then manually send state
  //TODO: try messing with resetting if button is held when module removed or potentiometer stuff
  //TODO: Encoder library
  //TODO: send negative number for unplugged potentiometer

  for (Side side = 0; side < 2; side = side + 1) {
    Module modInserted = readModule(side);
    Module_Type modType = get_type(modInserted);

    if (modType == TYPE_BUTTON) {
      if (modInserted == MOD_SHOOT) {
        int buttonStatus = !digitalRead(pin_configs[side].DataPin[0]);
        Serial.print("Fire Gun Button status: ");
        Serial.println(buttonStatus);
        Joystick.setButton(0, buttonStatus);
      } else if (modInserted == MOD_CHARGE) {
        int buttonStatus = !digitalRead(pin_configs[side].DataPin[0]);
        Serial.print("Charge Battery Button status: ");
        Serial.println(buttonStatus);
        Joystick.setButton(1, buttonStatus);
      }
    } else if (modType == TYPE_POTENTIOMETER){
      int potStatus = analogRead(pin_configs[side].DataPin[2]);
      int mappedPotStatus = map(potStatus, 0, 1023, 0, 255); 
      Serial.print("Mapped Speed Potentiometer status: ");
      Serial.println(mappedPotStatus);
      Joystick.setYAxis(255 - mappedPotStatus);
    } else if (modType == TYPE_ENCODER) {
      int emt; // encoder_module_type
      if (modInserted == MOD_AIM) {
        emt = 0;
      } else if (modInserted == MOD_SHIELD) {
        emt = 1;
      } else if (modInserted == MOD_STEER) {
        emt = 2;
      }

      int statusA = digitalRead(pin_configs[side].DataPin[0]);
      int statusB = digitalRead(pin_configs[side].DataPin[1]);

      byte newState = statusA << 1 | statusB;

      if (newState != lastState[emt]) {
        byte combined = (lastState[emt] << 2) | newState;
        if (newState == 0b00) {
          byte oldA = (lastState[emt] >> 1) & 1;
          byte oldB = lastState[emt] & 1;
          byte currentA = (newState >> 1) & 1;
          byte currentB = newState & 1;
          if (lastState[emt] == 0b10) {
            // Clockwise
            steerDirection = -1;
            if (emt == 0)  { // aim gun
              Serial.println("gun clockwise");
              Joystick.setButton(3, 0);
              Joystick.setButton(4, 1);
            } else if (emt == 1) { // aim shield
              Serial.println("Shield clockwise");
              Joystick.setButton(5, 0);
              Joystick.setButton(6, 1);
            }
            encoderPosition[emt]++;
            unsigned long now = micros();
            clickInterval[emt] = now - lastClickTime[emt];
            lastClickTime[emt] = now;
            Serial.print("Clockwise, Position = ");
            Serial.print(encoderPosition[emt]);
            Serial.print(", time interval (us) = ");
            Serial.println(clickInterval[emt]);
          }
          else if (lastState[emt] == 0b01) {
            // Counterclockwise
            steerDirection = -1;
            if (emt == 0)  { // aim gun
              Serial.println("gun counterclockwise");
              Joystick.setButton(3, 1);
              Joystick.setButton(4, 0);
            } else if (emt == 1) { // aim shield
              Serial.println("Shield counterclockwise");
              Joystick.setButton(5, 1);
              Joystick.setButton(6, 0);
            }
            encoderPosition[emt]--;
            unsigned long now = micros();
            clickInterval[emt] = now - lastClickTime[emt];
            lastClickTime[emt] = now;
            Serial.print("Counterclockwise, Position = ");
            Serial.print(encoderPosition[emt]);
            Serial.print(", time interval (us) = ");
            Serial.println(clickInterval[emt]);
          }
        }
      } else {
        // Joystick.setButton(3, 0);
        // Joystick.setButton(4, 0);
        // Joystick.setButton(5, 0);
        // Joystick.setButton(6, 0);
      }
      lastState[emt] = newState;
      switch (emt)
      {
        case 0:
          // Joystick.setXAxis(1000000 / clickInterval[0]);
          // Serial.println(1000000 / clickInterval[0]);
          break;
        case 1:
          // Joystick.setYAxis(1000000 / clickInterval[1]);
          // Serial.println(1000000 / clickInterval[1]);
          break;
        case 2: // steer
          Joystick.setXAxis(512 + steerDirection * (1000000 / clickInterval[2]));
          Serial.println(512 + steerDirection * (1000000 / clickInterval[2]));
          break;
      }
      

    } else {
      //Serial.println("No module attached, no data!");
    }
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
      // Serial.println("*** STEER ENCODER ***");
      return MOD_STEER; // Steering (Encoder)
    case 0b010:
      // Serial.println("*** AIM GUN ENCODER ***");
      return MOD_AIM; // Aim (Gun) (Encoder)
    case 0b011:
      // Serial.println("*** AIM SHIELD ENCODER ***");
      return MOD_SHIELD; // Aim (Shield) (Encoder)
    case 0b100:
      // Serial.println("*** SPEED POTENTIOMETER ***");
      return MOD_SPEED; // Adjust Speed (Potentiometer)
    case 0b101:
      // Serial.println("*** FIRE GUN BUTTON ***");
      return MOD_SHOOT; // Fire Gun (Button)
    case 0b110:
      // Serial.println("*** CHARGE BATTERY BUTTON ***");
      return MOD_CHARGE; // Charge Battery (Button)
    default:
      return MOD_NONE; // Default to no module
  }
}
