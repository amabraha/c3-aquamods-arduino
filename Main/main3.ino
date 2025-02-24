#include <Joystick.h>

enum Module {
    STEER=0,     //steering (potentiometer)
    AIM=1,       //gun aiming (encoder)
    SHIELD=2,    //shield aiming (encoder)
    SPEED=3,     //speed (slide potentiometer)
    SHOOT=4,     //gun shooting (button)
    CHARGE=5,    //charging battery (button)
    NO_MODULE=6  //no module
};

enum Side {
    LEFT=0,
    RIGHT=1
};

typedef struct {
    const int IDPin[3];    //IDPin[0] will be the LSB, IDPin[1] will be the middle bit, IDPin[2] will be the MSB
    const int DataPin[2];
} PinConfig;


PinConfig pin_configs[2] = {
    { //left side pin configuration
        .IDPin =   {6,5,4},
        .DataPin = {2,A0}
    },
    { //right side pin configuration
    //TODO: change these to be whatever they are supposed to be
        .IDPin =   {6,5,4},
        .DataPin = {2,A0}
    }
}


// Pin definitions for module selection (D6, D5, D4)
const int IDPin2 = 4; // Most significant bit (D4)
const int IDPin1 = 5; // Middle bit (D5)
const int IDPin0 = 6; // Least significant bit (D6)

// Pin definitions for inputs
const int potPin = A0;      // Potentiometer (or Encoder B)
const int buttonPin = 2;    // Button or Encoder A
const int encoderPinA = 2;  // Encoder Pin A (D2)
const int encoderPinB = A0; // Encoder Pin B (A0 in digital mode)

// Joystick setup (1 button, 1 axis/throttle)
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,
                   JOYSTICK_TYPE_GAMEPAD,
                   1, 0,                        // 1 button, no hats
                   false, false, false,         // X, Y, Z
                   false, false, false,         // Rx, Ry, Rz
                   true,                        // Throttle (or rotation)
                   false, false, false, false); // Rudder, Accelerator, Brake, Steering

int currentModule = -1;          // Stores the last detected module
int throttleValue = 127;         // Midpoint for encoder module
volatile int encoderCount = 127; // Track encoder throttle

void setup()
{
    // Set mode selection pins as inputs
    for (Side side = 0; side < 2; side ++) 
    {
        for (int idpin = 0; idpin < 3; idpin ++) 
        {
            pinMode(pin_configs[side].IdPin[idpin], INPUT_PULLUP);
        }
    }

    Joystick.begin(false);
}

void loop()
{
    //TODO: loop through both sides and do the following code for both sides
    int newModule = readModule(LEFT); // Dynamically check for module changes

    if (newModule != currentModule)
    { // If module changes, reconfigure
        currentModule = newModule;
        configureModule(currentModule, LEFT);
    }

    // process inputs based on current active module
    if (currentModule == SPEED)
    { // adjust speed (slide potentiometer)
        int rawThrottle = analogRead(potPin);
        throttleValue = map(rawThrottle, 0, 1023, 0, 255);
        // Serial.println(throttleValue);
        Joystick.setThrottle(throttleValue);
    }
    else if (currentModule == STEER || currentModule == AIM || currentModule == SHIELD)
    { // encoder modules (steering, aim gun, aim shield)
        Joystick.setThrottle(encoderCount);
    }
    else if (currentModule == SHOOT || currentModule == CHARGE)
    {                                               // button inputs (fire gun, charge battery)
        bool buttonState = !digitalRead(buttonPin); // active low
        Serial.println(buttonState);
        Joystick.setButton(0, buttonState);
    }

    //TODO: this is for manually sending state. Maybe add some checks so that we aren't redundantly sending state.
    Joystick.sendState();

    delay(10); // Prevent excessive polling
}

// Dynamically reads the module from D6, D5, D4
Module readModule(Side side)
{
    int moduleSelect = (digitalRead(IDPin0)) |
                       (digitalRead(IDPin1) << 1) |
                       digitalRead(IDPin2 << 2);
    moduleSelect = 0b110; // TEMP
    switch (moduleSelect)
    {
    case 0b001:
        return STEER; // Steering (Encoder)
    case 0b010:
        return AIM; // Aim (Gun) (Encoder)
    case 0b011:
        return SHIELD; // Aim (Shield) (Encoder)
    case 0b100:
        return SPEED; // Adjust Speed (Potentiometer)
    case 0b101:
        return SHOOT; // Fire Gun (Button)
    case 0b110:
        return CHARGE; // Charge Battery (Button)
    default:
        return NO_MODULE; // Default to no module
    }
}

// reconfigure input pins dynamically when module changes
void configureModule(Module module, Side side)
{
    if (module == SPEED)
    { // Adjust Speed (Potentiometer)
        pinMode(potPin, INPUT);
        detachInterrupt(digitalPinToInterrupt(encoderPinA)); // Ensure encoder interrupt is off
    }
    else if (module == STEER || module == AIM || module == SHIELD)
    { // Encoders (Steering, Aim Gun, Aim Shield)
        pinMode(encoderPinA, INPUT_PULLUP);
        pinMode(encoderPinB, INPUT_PULLUP); // A0 used as digital input
        attachInterrupt(digitalPinToInterrupt(encoderPinA), encoderISR, CHANGE);
    }
    else if (module == SHOOT || module == CHARGE)
    { // Button Modes (Fire Gun, Charge Battery)
        pinMode(buttonPin, INPUT_PULLUP);
        detachInterrupt(digitalPinToInterrupt(encoderPinA)); // Ensure encoder interrupt is off
    }
}

// interrupt service routine for encoder
void encoderISR()
{
    bool stateA = digitalRead(encoderPinA);
    bool stateB = digitalRead(encoderPinB);

    if (stateA == stateB)
    {
        encoderCount = min(255, encoderCount + 1); // clockwise
    }
    else
    {
        encoderCount = max(0, encoderCount - 1); // counterclockwise
    }
}
