#include "barcode.hpp"

HardwareSerial MySerial(1); //Crear instancia HardwareSerial

void Barcode::init(){
    MySerial.begin(9600, SERIAL_8N1, RX_B, TX_B); //RX, TX
    pinMode(AIMON, OUTPUT);
    iniFlag = true;
    digitalWrite(AIMON, HIGH);

    if(!SPIFFS.begin(true)){
        ESP_LOGE(BARCODETAG, "An Error has occurred while mounting SPIFFS");
        return;
    }
    
    ESP_LOGI(BARCODETAG, "Barcode initialized\n");
}

void Barcode::aimOn(){
    digitalWrite(AIMON, HIGH);
    delay(100);
    digitalWrite(AIMON, LOW);
    delay(25);
    AimOnTimer = millis();
}

void Barcode::decToAscii(byte * input, char * output, int length){
    for(int i=0; i<length; i++){
        output[i]=(char)input[i];
    }
}

bool Barcode::readCode(char * output){
    byte decEAN[EANLENGTH];
    int s=0;
    bool flag = false;

    if(millis()-AimOnTimer>10000){
        aimOn();
    }

    if(iniFlag){
        while(MySerial.available()){
            MySerial.read();
        }
        aimOn();
        delay(50);
        iniFlag=false;
    }

    while(MySerial.available()){
        flag=true;
        decEAN[s++]=MySerial.read();
    }

    if(flag){
        decToAscii(decEAN, output, EANLENGTH);
    }

    output[s] = 0;

    return flag;
}

void Barcode::clearSerial(){
    while(MySerial.available()){
        MySerial.read();
    }
}

void Barcode::printEAN(char * EAN){
    ESP_LOGI(BARCODETAG, "EAN: %s\n", EAN);
}

void Barcode::readBBDDVersion(){
    File file = SPIFFS.open("/bbdd.txt");

    if(!file){
        ESP_LOGE(BARCODETAG, "Failed to open file for reading");
    }

    char buffer[64];

    int i = 0;

    while(file.available()){
        int line = file.readBytesUntil('\n', buffer, sizeof(buffer)-1);
        buffer[line-1] = 0;
        if(i==0){
            memcpy(bbddV, buffer, sizeof(bbddV));
            ESP_LOGI(BARCODETAG, "bbdd version: %s", bbddV);
        }else if(i==1){
            bbddLen = atoi(buffer);
            ESP_LOGI(BARCODETAG, "bbdd len: %i\n", bbddLen);
        }else{
            file.close();
            return;
        }
        i++;
    }
    file.close();   
}

uint8_t Barcode::comparebbdd(char * EAN){
    File file = SPIFFS.open("/bbdd.txt");

    if(!file){
        ESP_LOGE(BARCODETAG, "Failed to open file for reading");
        return 0;
    }

    char buffer[64];
    int i = 0;

    while(file.available()){
        int line = file.readBytesUntil('\n', buffer, sizeof(buffer)-1);
        buffer[line-1] = 0;
        i++;
        if(strcmp(EAN, buffer)==0){
            ESP_LOGI(BARCODETAG, "%s\n", buffer);
            file.close();
            if(i<=bbddLen){
                return 1;
            }else{
                return 2;
            }
        }
    }

    file.close(); 
    return 0; 
}
