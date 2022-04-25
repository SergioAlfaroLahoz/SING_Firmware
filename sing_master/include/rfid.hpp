#ifndef RFID_H
#define RFID_H

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>

//#define RST_PIN         32  //Reset (Not used)  
#define SS_PIN          18  //Comunicación SPI, selecciona y habilita al esclavo   
#define RF_CLK          19  //Serial Clock, señal de reloj para sincronizar los dispositivos
#define RF_MISO         35  //Master In Slave Out, bits del maestro al esclavo
#define RF_MOSI         4  //Master Out Slave In, bits del esclavo al maestro
#define IRQ_PIN         32 //Interrupción presencia de tarjeta RFID

#define USERLENGTH  4

#define RFIDTAG "RFID"

class RFID{
    private: 
        //Create MFRC522 instance.
        MFRC522 mfrc522; //NULL value in RST pin (not used)
        MFRC522::MIFARE_Key key;
        byte regVal = 0x7F;

        byte user[USERLENGTH];
        byte userAnt[USERLENGTH];

        //Claves validas
        byte validKey1[4] = {0x47, 0xB9, 0x5F, 0x40};  //Ejemplo de clave valida
        byte validKey2[4] = {0xA7, 0xF3, 0x87, 0x40};
        byte validKey3[4] = {0xB7, 0xF5, 0x93, 0x40};
        byte validKey4[4] = {0x97, 0x82, 0x8E, 0x40};
        byte validKey5[4] = {0x6B, 0x44, 0xF6, 0x21};
        byte validKey6[4] = {0xAB, 0x03, 0x2D, 0x21};
        byte validKey7[4] = {0xEB, 0x59, 0x86, 0x21};
        byte validKey8[4] = {0x7B, 0x9C, 0x91, 0x21};
        byte validKey9[4] = {0x97, 0x01, 0x8B, 0x40};
        byte validKey10[4] = {0x03, 0xB2, 0x94, 0x02};
        byte validKey11[4] = {0x17, 0x94, 0x46, 0x3B};
        byte validKey12[4] = {0x07, 0x7E, 0x11, 0x3E};
        
    public:
        RFID(){;}
        void init();
        byte * getUser() {return user;}
        void activateRec(); //The function sending to the MFRC522 the needed commands to activate the reception
        void clearInt(); //The function to clear the pending interrupt bits after interrupt serving routine
        void printArray(byte *buffer, byte bufferSize); //Helper routine to dump a byte array as hex values to Serial
        void readUID(); //Reads the RFID tag and identify user
        bool isEqualArray(byte* arrayA, byte* arrayB, int length); //Compare two arrays, return true if equals
        bool validKey();
        bool userPresent();
        bool cmpUsers();
        void setUser(byte* newUser);
};

#endif