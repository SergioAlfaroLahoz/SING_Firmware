#include "metalDetector.hpp"

bool MetalDetector::init(){
    pinMode(METALSENSOR, INPUT);
    delay(5); //Revisar, mucho tiempo

    ESP_LOGI(METALTAG, "Metal sensor initialized\n");

    return true;
}

int MetalDetector::readFrequency(){
    unsigned int  counter = 0;
    unsigned int initTime = millis();
    bool value = digitalRead(METALSENSOR);

    while((millis()-initTime)<TIMEOUT){
        if(value!=digitalRead(METALSENSOR)){
            counter ++;
            value = !value;
        }
    }
    counter = counter/2;

    return ((float)counter/TIMEOUT)*1000;
}

int MetalDetector::baseline(int measures){
    unsigned int frequencies[measures];
    unsigned int averageFreq = 0;

    for(int i=0; i<measures; i++){
        frequencies[i]=readFrequency();
        averageFreq = averageFreq + frequencies[i];
        delay(13);
    }
    averageFreq = averageFreq/measures;

    return averageFreq;
}

bool MetalDetector::detectCan(int * freq){

    int frequency = readFrequency();
    int vFrequency = frequency-initFreq;

    ESP_LOGI(METALTAG, "Variation frequency: %i", vFrequency);

    memcpy(freq, &vFrequency, sizeof(vFrequency));

    if(vFrequency>ALUMINUMVAR){
        ESP_LOGI(METALTAG, "Aluminum can detected\n");
        return true;
    }else if(vFrequency<STEELVAR){
        ESP_LOGI(METALTAG, "Ferromagnetic steel can detected\n");
        return true;
    }else{
        ESP_LOGI(METALTAG, "No can detected\n");
    }
    return false;
}

void MetalDetector::calibrate(){
    initFreq = readFrequency();
    ESP_LOGI(METALTAG, "Metal sensor baseline: %i\n", initFreq);
}