#include "buzzer.hpp"

void Buzzer::init(){
    ledcSetup(bChannel, 1700, 8);
    ledcAttachPin(B_PIN, bChannel);
    ledcWrite(bChannel, 0);
}

void Buzzer::beep(){
    ledcWrite(bChannel, 125);
    delay(50);
    ledcWrite(bChannel, 0);
    delay(50);
    ledcWrite(bChannel, 125);
    delay(50);
    ledcWrite(bChannel, 0);
    delay(50);
}

void Buzzer::tripleBeep(){
    ledcWrite(bChannel, 125);
    delay(50);
    ledcWrite(bChannel, 0);
    delay(50);
    ledcWrite(bChannel, 125);
    delay(50);
    ledcWrite(bChannel, 0);
    delay(50);
    ledcWrite(bChannel, 125);
    delay(50);
    ledcWrite(bChannel, 0);
    delay(50);
}