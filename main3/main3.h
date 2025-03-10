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

    const int DataPin[3];  //DataPin0, DataPin1, DataPin2
                           //DataPin0 is your catch all data pin for all buttons. It is also encoder pin A
                           //DataPin1 will be used as pin B for the encoder
                           //DataPin2 will beu used as the analog pin
} PinConfig;

typedef struct {
    volatile Module module;
    volatile Side side;
    volatile bool pinAState;
    volatile bool pinBState;
    volatile int encoderCountPrev;
    volatile unsigned long timePrev;
    volatile int encoderCountCurr;
    volatile unsigned long timeCurr;
} EncoderState;