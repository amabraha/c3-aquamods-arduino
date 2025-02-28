#include <Joystick.h>

enum Module {
    MOD_STEER=0,     //steering (encoder)
    MOD_AIM=1,       //gun aiming (encoder)
    MOD_SHIELD=2,    //shield aiming (encoder)
    MOD_SPEED=3,     //speed (slide potentiometer)
    MOD_SHOOT=4,     //gun shooting (button)
    MOD_CHARGE=5,    //charging battery (button)
    MOD_NONE=6  //no module
};

enum Module_Type {
    TYPE_BUTTON=0,        //module uses a button
    TYPE_POTENTIOMETER=1, //module uses a potentiometer
    TYPE_ENCODER=2,       //module uses an encoder
    TYPE_NONE=3           //no module
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
};


// Pin definitions for module selection (D6, D5, D4)
const int IDPin2 = 4; // Most significant bit (D4)
const int IDPin1 = 5; // Middle bit (D5)
const int IDPin0 = 6; // Least significant bit (D6)

// Pin definitions for inputs
const int potPin = A0;      // Potentiometer (or Encoder B)
const int buttonPin = 2;    // Button or Encoder A
const int encoderPinA = 2;  // Encoder Pin A (D2)
const int encoderPinB = A0; // Encoder Pin B (A0 in digital mode)

// Joystick setup (2 buttons, 4 axis)
// button0: charge
// button1: shoot
// Rx-axis: speed
// X-axis: steering rate
// Y-axis: gun aiming rate
// Z-axis: shield aiming rate

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,
                   JOYSTICK_TYPE_GAMEPAD,
                   2, 0,                        // 2 button, no hats
                   true, true, true,         // X, Y, Z
                   true, false, false,         // Rx, Ry, Rz
                   true,                        // Throttle (or rotation)  //TODO: set throttle to false
                   false, false, false, false); // Rudder, Accelerator, Brake, Steering

int currentModule = -1;          // Stores the last detected module
int throttleValue = 127;         // Midpoint for encoder module
volatile int encoderCount = 127; // Track encoder throttle

void setup()
{
    // Set mode selection pins as inputs
    for (Side side = 0; side < 2; side = side+1) 
    {
        for (int idpin = 0; idpin < 3; idpin ++) 
        {
            pinMode(pin_configs[side].IDPin[idpin], INPUT_PULLUP);
        }
    }

    Joystick.begin(false);
}

Module_Type get_type(Module mod) 
{
    if (module == MOD_SPEED)
    {
        return TYPE_POTENTIOMETER;
    }
    else if (module == MOD_STEER || module == MOD_AIM || module == MOD_SHIELD)
    {
        return TYPE_ENCODER;
    }
    else if (module == MOD_SHOOT || module == MOD_CHARGE)
    {
        return TYPE_BUTTON;
    }
    else
    {
        return TYPE_NONE;
    }
}

void joystick_reset()
{
    Joystick.setXAxis(-1);
}

void loop()
{
    
    for (Side side = 0; side < 2; side = side+1)
    {}
    //TODO: loop through both sides and do the following code for both sides
    int newModule = readModule(LEFT); // Dynamically check for module changes

    if (newModule != currentModule)
    { // If module changes, reconfigure
        currentModule = newModule;
        configureModule(currentModule, LEFT);
    }

    // process inputs based on current active module
    if (currentModule == MOD_SPEED)
    { // adjust speed (slide potentiometer)
        int rawThrottle = analogRead(potPin);
        throttleValue = map(rawThrottle, 0, 1023, 0, 255);
        // Serial.println(throttleValue);
        Joystick.setThrottle(throttleValue);
    }
    else if (currentModule == MOD_STEER || currentModule == MOD_AIM || currentModule == MOD_SHIELD)
    { // encoder modules (steering, aim gun, aim shield)
        Joystick.setThrottle(encoderCount);
    }
    else if (currentModule == MOD_SHOOT || currentModule == MOD_CHARGE)
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
enum Module readModule(enum Side side)
{
    int moduleSelect = (digitalRead(IDPin0)) |
                       (digitalRead(IDPin1) << 1) |
                       digitalRead(IDPin2 << 2);
    moduleSelect = 0b110; // TEMP
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

// reconfigure input pins dynamically when module changes
void configureModule(enum Module module, enum Side side)
{
    if (module == MOD_SPEED)
    { // Adjust Speed (Potentiometer)
        pinMode(potPin, INPUT);
        detachInterrupt(digitalPinToInterrupt(encoderPinA)); // Ensure encoder interrupt is off
    }
    else if (module == MOD_STEER || module == MOD_AIM || module == MOD_SHIELD)
    { // Encoders (Steering, Aim Gun, Aim Shield)
        pinMode(encoderPinA, INPUT_PULLUP);
        pinMode(encoderPinB, INPUT_PULLUP); // A0 used as digital input
        attachInterrupt(digitalPinToInterrupt(encoderPinA), encoderISR, CHANGE);
    }
    else if (module == MOD_SHOOT || module == MOD_CHARGE)
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
