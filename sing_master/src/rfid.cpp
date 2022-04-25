#include "rfid.hpp"

void RFID::init(){
  mfrc522 = MFRC522(SS_PIN, NULL); 

  memset(user, 0, USERLENGTH);
  memset(userAnt, 0, USERLENGTH);

  SPI.begin(RF_CLK, RF_MISO, RF_MOSI, SS_PIN); //Función que inicializa SPI
  mfrc522.PCD_Init(); // Init MFRC522 card

  /* read and printout the MFRC522 version (valid values 0x12)*/
  byte readReg = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  ESP_LOGI(RFIDTAG, "RFID Ver: 0x%02x", readReg);

  /*
  * Allow the ... irq to be propagated to the IRQ pin
  * For test purposes propagate the IdleIrq and loAlert
  */
  regVal = 0xA0; //rx irq
  mfrc522.PCD_WriteRegister(mfrc522.ComIEnReg, regVal);

  ESP_LOGI(RFIDTAG, "RFID initialized\n");
}

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void RFID::printArray(byte *buffer, byte bufferSize){ //Actually not used
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
  Serial.println();
}

/*
 * The function sending to the MFRC522 the needed commands to activate the reception
 */
void RFID::activateRec(){
  mfrc522.PCD_WriteRegister(mfrc522.FIFODataReg, mfrc522.PICC_CMD_REQA);
  mfrc522.PCD_WriteRegister(mfrc522.CommandReg, mfrc522.PCD_Transceive);
  mfrc522.PCD_WriteRegister(mfrc522.BitFramingReg, 0x87);
}

/*
 * The function to clear the pending interrupt bits after interrupt serving routine
 */
void RFID::clearInt(){
  mfrc522.PCD_WriteRegister(mfrc522.ComIrqReg, 0x7F);
}

/*
 * Reads the RFID tag and identify user
 */
void RFID::readUID(){
  if(mfrc522.PICC_ReadCardSerial()){ //read the tag data
    memcpy(user, mfrc522.uid.uidByte, mfrc522.uid.size);
  }
  clearInt();
  mfrc522.PICC_HaltA();
}

/*
 * Compare two arrays, return true if equals
 */
bool RFID::isEqualArray(byte* arrayA, byte* arrayB, int length){
  for(int index = 0; index < length; index++){
    if(arrayA[index] != arrayB[index]){
        return false;
    }
  }
  return true;
}

/*
 * Return true if user UID card is equal to users saved
 */
bool RFID::validKey(){
  if(isEqualArray(user, validKey1, USERLENGTH)) return true;
  else if(isEqualArray(user, validKey2, USERLENGTH)) return true;
  else if(isEqualArray(user, validKey3, USERLENGTH)) return true;
  else if(isEqualArray(user, validKey4, USERLENGTH)) return true;
  else if(isEqualArray(user, validKey5, USERLENGTH)) return true;
  else if(isEqualArray(user, validKey6, USERLENGTH)) return true;
  else if(isEqualArray(user, validKey7, USERLENGTH)) return true;
  else if(isEqualArray(user, validKey8, USERLENGTH)) return true;
  else if(isEqualArray(user, validKey9, USERLENGTH)) return true;
  else if(isEqualArray(user, validKey10, USERLENGTH)) return true;
  else if(isEqualArray(user, validKey11, USERLENGTH)) return true;
  else if(isEqualArray(user, validKey12, USERLENGTH)) return true;
  else return false;
}

/*
 * Return true if user UID card is different from zero
 */
bool RFID::userPresent(){
  byte zero[USERLENGTH];
  memset(zero, 0, USERLENGTH);
  if(memcmp(user, zero, USERLENGTH)!=0){
    memcpy(userAnt, user, USERLENGTH);
    EEPROM.write(1, user[0]);
    EEPROM.write(2, user[1]);
    EEPROM.write(3, user[2]);
    EEPROM.write(4, user[3]);
    EEPROM.commit();
    return true;
  }
  return false;
}

/*
 * Return true if user is equal to userAnt, false if different
 */
bool RFID::cmpUsers(){
  if(isEqualArray(user, userAnt, USERLENGTH)){
    return true;
  }
  ESP_LOGI(RFIDTAG, "New user");
  memcpy(userAnt, user, USERLENGTH);
  EEPROM.write(1, user[0]);
  EEPROM.write(2, user[1]);
  EEPROM.write(3, user[2]);
  EEPROM.write(4, user[3]);
  EEPROM.commit();
  return false;
}

void RFID::setUser(byte* newUser){
  memcpy(user, newUser, USERLENGTH);
}