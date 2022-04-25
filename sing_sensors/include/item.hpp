#ifndef ITEM_H
#define ITEM_H

#include <Arduino.h>

#define EANLENGTH 20

#define ITEMTAG "ITEM"

class Item{
    private:

    public:
        Item(){;}
        int metal; //Frequency variation
        float pet; //PET index
        bool readMetal = false;
        bool readPet = false;
        uint8_t metalTrys = 0;
        uint8_t petTrys = 0;
        bool sentMetal = false;
        bool sentPet = false;
        void init();
        void clearItem();
};

#endif