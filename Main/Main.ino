/*
  AnalogReadSerial

  Reads an analog input on pin 0, prints the result to the Serial Monitor.
  Graphical representation is available using Serial Plotter (Tools > Serial Plotter menu).
  Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/AnalogReadSerial
*/

// might be useful to look into if we need smth more complicated, but I do doubt it
//https://andybrown.me.uk/2011/01/15/the-standard-template-library-stl-for-avr-with-c-streams/

/*
IDEAS for stuff:

//probably want defines for pins of various things
//make an array that contains all this information for both the left and right side

//function that returns the module ID given the side
enum module_id(enum side) {

  read id pins
  return according to some predefined map
}


void setup() {
  not really sure what to put in here atm, maybe some usb setup stuff needs to happen. We can also keep the serial communication for debugging

  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

void loop() {
  enum id1 = module_id(left);
  enum id2 = module_id(right);

  for (i = 0; i < 2; i ++) {
    id = id1 or id2 depending on i;


    update game state depending on id.
    prolly should make a separate function for each id, which should also take into account the side (for which pins to read from)
  }

  do usb sending stuff

  delay(1); //idk if we need this, prolly depends on our current latency and whether we have enough delays within our code.
}


*/


// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

// the loop routine runs over and over again forever:
void loop() {
  // read the input on analog pin 0:
  int sensorValue = analogRead(A0);
  // print out the value you read:
  Serial.println(sensorValue);
  delay(1);  // delay in between reads for stability
}
