#include "device.hpp"

Item object;
Motors motor;
MetalDetector metal;
PlasticDetector PET;
Comms comms;

bool initFlag = false;
long motorsTime;
long motorsTime2;
long errorTimer;
long petTimer;
int pSensorValue;

/*
Method to set esp logs level
*/
void espLogsLevel(){
  esp_log_level_set(MAINTAG, ESP_LOG_VERBOSE); //ESP_LOG_INFO
  esp_log_level_set(ITEMTAG, ESP_LOG_VERBOSE);
  esp_log_level_set(METALTAG, ESP_LOG_VERBOSE);
  esp_log_level_set(MOTORSTAG, ESP_LOG_VERBOSE);
  esp_log_level_set(PETTAG, ESP_LOG_VERBOSE);
  esp_log_level_set(STATESTAG, ESP_LOG_VERBOSE);
  esp_log_level_set(COMMSTAG, ESP_LOG_VERBOSE);
}