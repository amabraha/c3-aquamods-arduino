#include <Joystick.h>

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
    pinMode(IDPin0, INPUT_PULLUP); // D6
    pinMode(IDPin1, INPUT_PULLUP); // D5
    pinMode(IDPin2, INPUT_PULLUP); // D4

    Joystick.begin();
}

void loop()
{
    int newModule = readModule(); // Dynamically check for module changes

    if (newModule != currentModule)
    { // If module changes, reconfigure
        currentModule = newModule;
        configureModule(currentModule);
    }

    // process inputs based on current active module
    if (currentModule == 3)
    { // adjust speed (slide potentiometer)
        int rawThrottle = analogRead(potPin);
        throttleValue = map(rawThrottle, 0, 1023, 0, 255);
        // Serial.println(throttleValue);
        Joystick.setThrottle(throttleValue);
    }
    else if (currentModule == 0 || currentModule == 1 || currentModule == 2)
    { // encoder modules (steering, aim gun, aim shield)
        Joystick.setThrottle(encoderCount);
    }
    else if (currentModule == 4 || currentModule == 5)
    {                                               // button inputs (fire gun, charge battery)
        bool buttonState = !digitalRead(buttonPin); // active low
        Serial.println(buttonState);
        Joystick.setButton(0, buttonState);
    }

    delay(10); // Prevent excessive polling
}

// Dynamically reads the module from D6, D5, D4
int readModule()
{
    int moduleSelect = (digitalRead(IDPin0)) |
                       (digitalRead(IDPin1) << 1) |
                       digitalRead(IDPin2 << 2);
    moduleSelect = 0b110; // TEMP
    switch (moduleSelect)
    {
    case 0b001:
        return 0; // Steering (Encoder)
    case 0b010:
        return 1; // Aim (Gun) (Encoder)
    case 0b011:
        return 2; // Aim (Shield) (Encoder)
    case 0b100:
        return 3; // Adjust Speed (Potentiometer)
    case 0b101:
        return 4; // Fire Gun (Button)
    case 0b110:
        return 5; // Charge Battery (Button)
    default:
        return 3; // Default to Adjust Speed (Potentiometer)
    }
}

// reconfigure input pins dynamically when ul changes
void configureModule(int module)
{
    if (module == 3)
    { // Adjust Speed (Potentiometer)
        pinMode(potPin, INPUT);
        detachInterrupt(digitalPinToInterrupt(encoderPinA)); // Ensure encoder interrupt is off
    }
    else if (module == 0 || module == 1 || module == 2)
    { // Encoders (Steering, Aim Gun, Aim Shield)
        pinMode(encoderPinA, INPUT_PULLUP);
        pinMode(encoderPinB, INPUT_PULLUP); // A0 used as digital input
        attachInterrupt(digitalPinToInterrupt(encoderPinA), encoderISR, CHANGE);
    }
    else if (module == 4 || module == 5)
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
