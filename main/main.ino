#include <Joystick.h>
#include <Encoder.h>
#include "main.h"

#define SPEED_CONSTANT 5000000

// Pin definitions
PinConfig pin_configs[2] = {
  { // left side pin configuration
    .IDPin = {4, 5, 6},   // IDPin0, IDPin1, IDPin2
    .DataPin = {2, 3, A0} // DataPin2, DataPin1, DataPin0
  },
  { // right side pin configuration
  .IDPin = {9, 10, 11},   // IDPin0, IDPin1, IDPin2
  .DataPin = {0, 1, A1}   // DataPin2, DataPin1, DataPin0
  }
};

// Joystick setup
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,
                  JOYSTICK_TYPE_GAMEPAD,
                  6, 0,                        // 6 button, no hats
                  true, true, false,           // X, Y, Z               FOR THE STEER + SPEED
                  true, true, false,           // Rx, Ry, Rz            FOR THE 2 AIMING
                  false,                       // Throttle (or rotation)
                  false, false, false, false); // Rudder, Accelerator, Brake, Steering

// ENCODER
Encoder leftEnc(pin_configs[LEFT].DataPin[0], pin_configs[LEFT].DataPin[1]);
Encoder rightEnc(pin_configs[RIGHT].DataPin[0], pin_configs[RIGHT].DataPin[1]);

long oldLeftEncPosition = 0;
long oldRightEncPosition = 0;
long newLeftEncPosition = 0;
long newRightEncPosition = 0;

long steerPosition = 0;
long aimPosition = 0;
long shieldPosition = 0;

long steerDirection = 0;
long aimDirection = 0;
long shieldDirection = 0;

long prevSteerTime = 0;
long prevAimTime = 0;
long prevShieldTime = 0;

long steerInterval = 0;
long aimInterval = 0;
long shieldInterval = 0;

long steerSpeed = 0;
long aimSpeed = 0;
long shieldSpeed = 0;

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
  
  // Send initial values
  Joystick.begin(false);
  Joystick.setXAxis(512);
  Joystick.setRxAxis(512);
  Joystick.setRyAxis(512);
  Joystick.setYAxis(511);
  Joystick.setButton(0, 0);
  Joystick.setButton(1, 0);
}

void loop() {
  //TODO: try messing with resetting if button is held when module removed or potentiometer stuff
  //TODO: send negative number for unplugged potentiometer

  for (Side side = 0; side < 2; side = side + 1) {
    Module modInserted = readModule(side);
    // Module_Type modType = get_type(modInserted);
    // display_inserted_module(side, modInserted);

    switch(modInserted) {
      case MOD_STEER: {
        read_encoder(side, modInserted);
        // Serial.print("STEER POSITION: "); Serial.println(steerPosition);
        // if (steerSpeed > 0) {Serial.print("STEER SPEED: "); Serial.println(steerSpeed);}
        
        // Range we should send is 0 - 1023
        // 0 is full left, 1023 is full right
        // Serial.println(512 + steerSpeed * steerDirection);
        Joystick.setXAxis(512 + steerSpeed * steerDirection);
        Joystick.sendState();
      } break;
      case MOD_AIM: {
        read_encoder(side, modInserted);
        // Serial.print("AIM POSITION: "); Serial.println(aimPosition);
        // if (aimSpeed > 0) {Serial.print("AIM SPEED: "); Serial.println(aimSpeed);}
        Joystick.setRxAxis(512 + aimSpeed * aimDirection);
        Joystick.sendState();
      } break;
      case MOD_SHIELD: {
        read_encoder(side, modInserted);
        // Serial.print("SHIELD POSITION: "); Serial.println(shieldPosition);
        // if (shieldSpeed > 0) {Serial.print("SHIELD SPEED: "); Serial.println(shieldSpeed);}
        Joystick.setRyAxis(512 + shieldSpeed * shieldDirection);
        Joystick.sendState();
      } break;
      case MOD_SPEED: {
        int potStatus = analogRead(pin_configs[side].DataPin[2]);
        int mappedPotStatus = map(potStatus, 0, 1023, 511, 0); 
        // Serial.print("Mapped Speed Potentiometer status: ");
        // Serial.println(mappedPotStatus);
        Joystick.setYAxis(mappedPotStatus);
        Joystick.sendState();
      } break;
      case MOD_SHOOT: {
        int shootButtonStatus = !digitalRead(pin_configs[side].DataPin[0]);
        // Serial.print("shoot status: "); Serial.println(shootButtonStatus);
        Joystick.setButton(0, shootButtonStatus);
        Joystick.sendState();
      } break;
      case MOD_CHARGE: {
        int chargeButtonStatus = !digitalRead(pin_configs[side].DataPin[0]);
        // Serial.print("charge status: "); Serial.println(chargeButtonStatus);
        Joystick.setButton(1, chargeButtonStatus);
        Joystick.sendState();
      } break;
      case MOD_NONE: {
        // stuff
      } break;
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
      return MOD_STEER;  // Steering (Encoder)
    case 0b010:
      return MOD_AIM;    // Aim (Gun) (Encoder)
    case 0b011:
      return MOD_SHIELD; // Aim (Shield) (Encoder)
    case 0b100:
      return MOD_SPEED;  // Adjust Speed (Potentiometer)
    case 0b101:
      return MOD_SHOOT;  // Fire Gun (Button)
    case 0b110:
      return MOD_CHARGE; // Charge Battery (Button)
    default:
      return MOD_NONE;   // Default to no module
  }
}

void display_inserted_module(enum Side side, enum Module module) {
  switch(module) {
    case MOD_STEER:
      Serial.print("Side "); Serial.print(side); Serial.print(" STEER          ");
      break;
    case MOD_AIM:
      Serial.print("Side "); Serial.print(side); Serial.print(" AIM GUN        ");
      break;
    case MOD_SHIELD:
      Serial.print("Side "); Serial.print(side); Serial.print(" AIM SHIELD     ");
      break;
    case MOD_SPEED:
      Serial.print("Side "); Serial.print(side); Serial.print(" SPEED          ");
      break;
    case MOD_SHOOT:
      Serial.print("Side "); Serial.print(side); Serial.print(" FIRE GUN       ");
      break;
    case MOD_CHARGE:
      Serial.print("Side "); Serial.print(side); Serial.print(" BATTERY        ");
      break;
    case MOD_NONE:
      Serial.print("Side "); Serial.print(side); Serial.print(" NONE           ");
      break;
  }
  if (side == RIGHT) Serial.print("\n");
}

// Given a side and a module (one of the 3 encoder modules),
// read the encoder and update the stored position of that
// encoder module
void read_encoder(enum Side side, enum Module module) {
  // absolute position calculation
  if (side == LEFT) {
    newLeftEncPosition = leftEnc.read();

    // time interval calculation
    if (newLeftEncPosition != oldLeftEncPosition) { 
      unsigned long now = micros();
      switch (module) {
        case MOD_STEER: {
          steerInterval = now - prevSteerTime;
          steerSpeed = SPEED_CONSTANT / steerInterval;
          prevSteerTime = now;
        } break;
        case MOD_AIM: {
          aimInterval = now - prevAimTime;
          aimSpeed = SPEED_CONSTANT / aimInterval;
          prevAimTime = now;
        } break;
        case MOD_SHIELD: {
          shieldInterval = now - prevShieldTime;
          shieldSpeed = SPEED_CONSTANT / shieldInterval;
          prevShieldTime = now;
        } break;
      }
    } else {
      switch (module) {
        case MOD_STEER: {
          steerSpeed = 0;
          steerDirection = 0;
        } break;
        case MOD_AIM: {
          aimSpeed = 0;
          aimDirection = 0;
        } break;
        case MOD_SHIELD: {
          shieldSpeed = 0;
          shieldDirection = 0;
        } break;
      }
    }

    // absolute position calculation
    if (newLeftEncPosition < oldLeftEncPosition) { 
      // CLOCKWISE
      switch (module) {
        case MOD_STEER: {
          steerPosition++;
          steerDirection = 1;
        } break;
        case MOD_AIM: {
          aimPosition++;
          aimDirection = 1;
        } break;
        case MOD_SHIELD: {
          shieldPosition++;
          shieldDirection = 1;
        } break;
      }
    } else if (newLeftEncPosition > oldLeftEncPosition) { 
      // COUNTER-CLOCKWISE
      switch (module) {
        case MOD_STEER: {
          steerPosition--;
          steerDirection = -1;
        } break;
        case MOD_AIM: {
          aimPosition--;
          aimDirection = -1;
        } break;
        case MOD_SHIELD: {
          shieldPosition--;
          shieldDirection = -1;
        } break;
      }
    }
    oldLeftEncPosition = newLeftEncPosition;
  } else {
    newRightEncPosition = rightEnc.read();
    
    // time interval calculation
    if (newRightEncPosition != oldRightEncPosition) { 
      unsigned long now = micros();
      switch (module) {
        case MOD_STEER: {
          steerInterval = now - prevSteerTime;
          steerSpeed = SPEED_CONSTANT / steerInterval;
          prevSteerTime = now;
        } break;
        case MOD_AIM: {
          aimInterval = now - prevAimTime;
          aimSpeed = SPEED_CONSTANT / aimInterval;
          prevAimTime = now;
        } break;
        case MOD_SHIELD: {
          shieldInterval = now - prevShieldTime;
          shieldSpeed = SPEED_CONSTANT / shieldInterval;
          prevShieldTime = now;
        } break;
      }
    } else {
      switch (module) {
        case MOD_STEER: {
          steerSpeed = 0;
          steerDirection = 0;
        } break;
        case MOD_AIM: {
          aimSpeed = 0;
          aimDirection = 0;
        } break;
        case MOD_SHIELD: {
          shieldSpeed = 0;
          shieldDirection = 0;
        } break;
      }
    }

    // absolute position calculation
    if (newRightEncPosition < oldRightEncPosition) { 
      // CLOCKWISE
      switch (module) {
        case MOD_STEER: {
          steerPosition++;
          steerDirection = 1;
        } break;
        case MOD_AIM: {
          aimPosition++;
          aimDirection = 1;
        } break;
        case MOD_SHIELD: {
          shieldPosition++;
          shieldDirection = 1;
        } break;
      }
    } else if (newRightEncPosition > oldRightEncPosition) { 
      // COUNTER-CLOCKWISE
      switch (module) {
        case MOD_STEER: {
          steerPosition--;
          steerDirection = -1;
        } break;
        case MOD_AIM: {
          aimPosition--;
          aimDirection = -1;
        } break;
        case MOD_SHIELD: {
          shieldPosition--;
          shieldDirection = -1;
        } break;
      }
    }
    oldRightEncPosition = newRightEncPosition;
  }
}
