#ifndef DEVICE_H
#define DEVICE_H

//INCLUDES
#include <Arduino.h>
#include "item.hpp"
#include "motors.hpp"
#include "metalDetector.hpp"
#include "plasticDetector.hpp"
#include "comms.hpp"

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp32-hal-log.h" //esplogi https://demo-dijiudu.readthedocs.io/en/stable/api-reference/system/log.html
 
#define MAINTAG "Main"

#define PSENSOR1 39

//TIMEOUTS
#define TIMEOUTPSENSOR 5000
#define TIMEOUTERROR 5000
#define PETREADTIMER 550 

//MAIN OBJECTS
extern Item object;
extern Motors motor;
extern MetalDetector metal;
extern PlasticDetector PET;
extern Comms comms;

//MAIN VARIABLES
extern bool initFlag;
extern long motorsTime;
extern long motorsTime2;
extern long errorTimer;
extern long petTimer;
extern int pSensorValue;

void espLogsLevel();

#endif