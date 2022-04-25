#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>
#include "metalDetector.hpp"

#define M1FC1 36
#define M1FC2 35
#define M2FC1 32     
#define M2FC2 33

#define MRIGHT 0
#define MLEFT 1

#define TIMEOUTMOTORS 3000 //1300

#define MOTORSTAG "Motors"

class Motors{
    private:
        //The number of the Motor pin
        //motor1
        const int M1RP = 27;  // Motor1_speed_dir1
        const int M1LP = 23; // Motor1_speed_dir2
        //motor2
        const int M2LP = 19; // Motor2_speed_dir1
        const int M2RP = 18; // Motor2_speed_dir2

        // setting PWM properties
        const int freq = 50000;
        const int resolution = 8;
        const int M1Left = 0;
        const int M1Right = 1;
        const int M2Left = 2;
        const int M2Right = 3;

        const int velocityM1 = 255; //230
        const int velocityM2 = 255;

        bool fOpen = false;
        bool rOpen = false;
    
    public:
        Motors() {;} //Constructor
        bool init();
        bool openM1();
        bool openM1Static(bool initM); //Returns true if door is opened, false if not
        bool closeM1();
        bool closeM1Static(bool initM); //Returns true if door is closed, false if not
        bool openM2();
        bool openM2Static(bool initM); //Returns true if door is opened, false if not
        bool closeM2();
        bool closeM2Static(bool initM); //Returns true if door is closed, false if not
        bool home();
        bool unblockM1(int direction);
        bool unblockM2(int direction);
        bool getFront() {return fOpen;}
        bool getRear() {return rOpen;}
};

#endif