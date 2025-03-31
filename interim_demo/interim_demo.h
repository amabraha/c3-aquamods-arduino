typedef struct {
    const int IDPin[3];    // IDPin[0] will be the LSB, IDPin[1] will be the middle bit, IDPin[2] will be the MSB

    const int DataPin[3];
                           // DataPin[0] will be used as the analog pin
                           // DataPin[1] will be used as pin B for the encoder
                           // DataPin[2] is the data pin for all buttons. It is also encoder pin A
} PinConfig;

enum Side {
    LEFT  = 0,
    RIGHT = 1
};

enum Module {
    MOD_STEER  = 0,  // steering (encoder)
    MOD_AIM    = 1,  // gun aiming (encoder)
    MOD_SHIELD = 2,  // shield aiming (encoder)
    MOD_SPEED  = 3,  // speed (slide potentiometer)
    MOD_SHOOT  = 4,  // gun shooting (button)
    MOD_CHARGE = 5,  // charging battery (button)
    MOD_NONE   = 6   // no module
};

enum Module_Type {
    TYPE_BUTTON        = 0,  // module uses a button
    TYPE_POTENTIOMETER = 1,  // module uses a potentiometer
    TYPE_ENCODER       = 2,  // module uses an encoder
    TYPE_NONE          = 3   // no module
};
