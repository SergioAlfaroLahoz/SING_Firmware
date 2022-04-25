#include "item.hpp"

void Item::init(){
    readMetal = false;
    readPet = false;
    metalTrys = 0;
    petTrys = 0;
    sentMetal = false;
    sentPet = false;
    ESP_LOGI(ITEMTAG, "Item created\n");
}

void Item::clearItem(){
    readMetal = false;
    readPet = false;
    metalTrys = 0;
    petTrys = 0;
    sentMetal = false;
    sentPet = false;
    ESP_LOGI(ITEMTAG, "Reset item\n");
}

