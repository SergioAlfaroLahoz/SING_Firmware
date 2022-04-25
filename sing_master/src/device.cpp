#include "device.hpp"

LEDs led;
Buzzer buzzer;
RFID rfid;
Barcode scanner;
States state;
Clock myClock;
Comms comms;

volatile bool bNewInt = false;
long sleepTimer;
bool programExit = false;
bool sleepFlag = false;
long readTimer;
bool memFullFlag = false;
long errorTimer;
long errorStateTime;

/**
 * MFRC522 interrupt serving routine
 */
void IRAM_ATTR readCard(){ //Declarar en main para poder llamar a rf.read
  //Serial.println("Interrupt");
  bNewInt = true; //intentar leer tarjeta rfid dentro de la interrupcion
  rfid.readUID(); //Modificar, optimizar interrupcion para que sea el menor tiempo posible (solo leer uid)
}

/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
esp_sleep_wakeup_cause_t print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason){
    case ESP_SLEEP_WAKEUP_EXT0 : 
      Serial.println("Wakeup caused by external signal using RTC_IO\n"); 
      break;
    case ESP_SLEEP_WAKEUP_EXT1 : 
      Serial.println("Wakeup caused by external signal using RTC_CNTL\n"); 
      break;
    case ESP_SLEEP_WAKEUP_TIMER : 
      Serial.println("Wakeup caused by timer\n"); 
      break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : 
      Serial.println("Wakeup caused by touchpad\n"); 
      break;
    case ESP_SLEEP_WAKEUP_ULP : 
      Serial.println("Wakeup caused by ULP program\n"); 
      break;
    default : 
      Serial.printf("Wakeup was not caused by deep sleep: %d\n\n",wakeup_reason); 
      break;
  }
  
  return wakeup_reason;
}

/*
Method to set esp logs level
*/
void espLogsLevel(){
  esp_log_level_set(MAINTAG, ESP_LOG_VERBOSE); //ESP_LOG_INFO
  esp_log_level_set(BARCODETAG, ESP_LOG_VERBOSE);
  esp_log_level_set(CLOCKTAG, ESP_LOG_VERBOSE);
  esp_log_level_set(RFIDTAG, ESP_LOG_VERBOSE);
  esp_log_level_set(STATESTAG, ESP_LOG_VERBOSE);
  esp_log_level_set(COMMSTAG, ESP_LOG_VERBOSE);
}