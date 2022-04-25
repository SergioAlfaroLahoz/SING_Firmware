#ifndef LED_H
#define LED_H

#include <Arduino.h>
#include <FastLED.h>

enum Colors{
    GREEN = 0,
    YELLOW = 45,
    ORANGE = 85,
    RED = 95,
    PINK = 120,
    PURPLE = 140, 
    BLUE = 160, 
    CYAN = 220
};

class LEDs{
    private:
        #define NUM_LEDS 2
        #define DATA_PIN 23
        CRGB leds[NUM_LEDS];
    public:
        LEDs() {;} //Constructor
        void init();
        void led1ON(Colors color);
        void led2ON(Colors color);
        void ledsOFF();
        void led1Blink(Colors color);
        void led2Blink(Colors color);
        void led1BlinkLoop(Colors color);
        void led2BlinkLoop(Colors color);
        void gradient();
};

#endif