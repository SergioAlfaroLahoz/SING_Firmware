#include "leds.hpp"

void LEDs::init(){
    LEDS.addLeds<WS2812,DATA_PIN,RGB>(leds,NUM_LEDS);
	LEDS.setBrightness(255);
    ledsOFF();
}

void LEDs::led1ON(Colors color){
    leds[0] = CHSV(color, 255, 255);
    FastLED.show();
}

void LEDs::led2ON(Colors color){
    leds[1] = CHSV(color, 255, 255);
    FastLED.show();
}

void LEDs::ledsOFF(){
    leds[0] = CRGB::Black;
    leds[1] = CRGB::Black;
    FastLED.show();
}

void LEDs::led1Blink(Colors color){
    static bool active = true;
    static long time = millis();
    if(millis()-time>600){
        if(active){
            leds[0] = CRGB::Black;
        }else{
            leds[0] = CHSV(color, 255, 255);
        }
        FastLED.show();
        active = !active;
        time = millis();
    }
}

void LEDs::led2Blink(Colors color){
    static bool active = true;
    static long time = millis();
    if(millis()-time>600){
        if(active){
            leds[1] = CRGB::Black;
        }else{
            leds[1] = CHSV(color, 255, 255);
        }
        FastLED.show();
        active = !active;
        time = millis();
    }
}

void LEDs::led1BlinkLoop(Colors color){
    leds[0] = CHSV(color, 255, 255);
    FastLED.show();
    delay(600);
    leds[0] = CRGB::Black;
    FastLED.show();
    delay(600);
    leds[0] = CHSV(color, 255, 255);
    FastLED.show();
    delay(600);
}

void LEDs::led2BlinkLoop(Colors color){
    leds[1] = CHSV(color, 255, 255);
    FastLED.show();
    delay(600);
    leds[1] = CRGB::Black;
    FastLED.show();
    delay(600);
    leds[1] = CHSV(color, 255, 255);
    FastLED.show();
    delay(600);
}

void LEDs::gradient(){
    for(int i=0; i<255; i++){
        leds[0] = CHSV(i, 255, 255);
        leds[1] = CHSV(i, 255, 255);
        FastLED.show();
        Serial.println(i);
        delay(100);
    }
}

