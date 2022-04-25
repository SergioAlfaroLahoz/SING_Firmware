#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

class Buzzer{
    private:
        #define B_PIN 13
        const int bChannel = 0;
    public:
        Buzzer() {;} //Constructor
        void init();
        void beep();
        void tripleBeep();
};

#endif