#ifndef BARCODE_H
#define BARCODE_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include "SPIFFS.h"
//#include "device.hpp"

#define EANLENGTH 20
#define AIMON 27    
#define RX_B 2 //Cambiar por 14
#define TX_B 15

#define BARCODETAG "BARCODE"

class Barcode{
    private: 
        bool iniFlag;
        long AimOnTimer;
    public:
        Barcode(){;}
        void init();
        void aimOn();
        void decToAscii(byte * input, char * output, int length);
        bool readCode(char * output); 
        void printEAN(char * EAN);
        void clearSerial();
        void readBBDDVersion();
        uint8_t comparebbdd(char * EAN);
        char bbddV[6];
        int bbddLen;
        //uint8_t read(); // 1-Article found, 2-Article not found, 3-Read nothing. Difference between can, bottle or brick?, Deprecated
};

#endif