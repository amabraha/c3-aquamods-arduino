#include <Joystick.h>
#include "main3.h"





// Pin definitions
PinConfig pin_configs[2] = {
    { //left side pin configuration
        .IDPin =   {6,5,4},   //IDPin0, IDPin1, IDPin2
        .DataPin = {0,1,A0}   //DataPin0, DataPin1, DataPin2
    },
    { //right side pin configuration
    //TODO: change these to be whatever they are supposed to be
        .IDPin =   {6,5,4},
        .DataPin = {2,3,A0}
    }
};

//encoder global state variables
//initialized in setup()
EncoderState encoder_state[3];


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

    //initialize encoder_state module fields to be their corresponding module
    for (Module mod = 0; mod < 3; mod = mod+1)
    {
        encoder_state[mod].module = mod;
    }

    Joystick.begin(false);
}

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

void joystick_reset()
{
    Joystick.setXAxis(-1);
    Joystick.setYAxis(-1);
    Joystick.setZAxis(-1);
    Joystick.setRxAxis(-1);
    Joystick.releaseButton(0);
    Joystick.releaseButton(1);
}

void loop()
{
    joystick_reset();

    
    for (Side side = 0; side < 2; side = side+1)
    {
        int newModule = readModule(side);
    }
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
    int moduleSelect = (digitalRead(pin_configs[side].IDPin[0])) |
                       (digitalRead(pin_configs[side].IDPin[1]) << 1) |
                       (digitalRead(pin_configs[side].IDPin[2]) << 2);
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
    
    switch (get_type(module))
    {
    case TYPE_POTENTIOMETER: // Adjust Speed (Potentiometer)
        pinMode(pin_configs[side].DataPin[2], INPUT);
        detachInterrupt(digitalPinToInterrupt(pin_configs[side].DataPin[0])); // Ensure encoder interrupt is off
        detachInterrupt(digitalPinToInterrupt(pin_configs[side].DataPin[1])); // Ensure encoder interrupt is off
        break;
    case TYPE_ENCODER:      // Encoders (Steering, Aim Gun, Aim Shield)
        pinMode(pin_configs[side].DataPin[0], INPUT_PULLUP);
        pinMode(pin_configs[side].DataPin[1], INPUT_PULLUP);

        //reconfigure encoder state for corresponding module
        encoder_state[module] = 
        {
            .module = module,
            .side = side,
            .pinAState = digitalRead(pin_configs[side].DataPin[0]),
            .pinBState = digitalRead(pin_configs[side].DataPin[1]),
            .encoderCountPrev = 0,
            .timePrev = micros(),
            .encoderCountCurr = 0,
            .timeCurr = micros()
        };

        //attach encoder ISR corresponding to the appropriate side
        switch(side)
        {
        case LEFT:
            attachInterrupt(digitalPinToInterrupt(pin_configs[side].DataPin[0]), encoderLeftISR, CHANGE);
            attachInterrupt(digitalPinToInterrupt(pin_configs[side].DataPin[1]), encoderLeftISR, CHANGE);
            break;
        case RIGHT:
            attachInterrupt(digitalPinToInterrupt(pin_configs[side].DataPin[0]), encoderRightISR, CHANGE);
            attachInterrupt(digitalPinToInterrupt(pin_configs[side].DataPin[1]), encoderRightISR, CHANGE);
            break;
        }
        break;
    case TYPE_BUTTON:       // Button Modes (Fire Gun, Charge Battery)
        pinMode(pin_configs[side].DataPin[0], INPUT_PULLUP);
        detachInterrupt(digitalPinToInterrupt(pin_configs[side].DataPin[0])); // Ensure encoder interrupt is off
        detachInterrupt(digitalPinToInterrupt(pin_configs[side].DataPin[1])); // Ensure encoder interrupt is off
        break;
    case TYPE_NONE:
        break;
    }
}

//general encoder ISR
void encoderISR(Side side)
{
    unsigned long new_time = micros();
    Module encoder_module = readModule(side);
    
    //TODO: idk if we need this, but sanity check
    if(get_type(encoder_module) != TYPE_ENCODER)
    {
        exit(0);
        return;
    }

    bool stateA = digitalRead(pin_configs[side].DataPin[0]);
    bool stateB = digitalRead(pin_configs[side].DataPin[1]);

    //in CW order, the order we see states (B,A) is
    //0: (1,1)
    //1: (1,0)
    //2: (0,0)
    //3: (0,1)

    //quadrature_state_order represents a mapping from the greycode for states B/A to the order they appear in the rotation of the encoder
    //quadrature_state_order[B*2 + A] is the order of state (B,A) as above
    int quadrature_state_order[4] = {2, 3, 1, 0};

    int curr_quadrature_state = quadrature_state_order[stateB*2+stateA];
    int prev_quadrature_state = quadrature_state_order[encoder_state[encoder_module].pinBState*2+encoder_state[encoder_module].pinAState];

    switch ((4+curr_quadrature_state-prev_quadrature_state) % 4)
    {
    case 1:
        encoder_state[encoder_module].encoderCountCurr += 1; // clockwise
        break;
    case 3:
        encoder_state[encoder_module].encoderCountCurr -= 1; // counterclockwise
        break;
    default:
        //we've missed a state oops
        exit(0);
        return;
    }

    //TODO: need to reconfigure encoder_state struct with newfound information.
    
}


//ISRs for encoder pin change interrupts
void encoderLeftISR()
{
    encoderISR(RIGHT);
}

void encoderRightISR()
{
    encoderISR(LEFT);
}
