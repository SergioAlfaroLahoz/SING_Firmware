#ifndef METALDETECTOR_H
#define METALDETECTOR_H

#include <Arduino.h>

#define METALSENSOR 34
#define TIMEOUT 250 
#define ALUMINUMVAR 50
#define STEELVAR -20

#define METALTAG "Metal sensor"

class MetalDetector{
    private:
        int initFreq;
    public:
        MetalDetector() {;} //Constructor
        bool init();
        int readFrequency();
        int baseline(int measures);
        bool detectCan(int * freq);
        void calibrate();
};

#endif