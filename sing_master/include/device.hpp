#ifndef DEVICE_H
#define DEVICE_H

#include <driver/rtc_io.h>

//INCLUDES
#include <Arduino.h>
#include "comms.hpp"
#include "states.hpp"
#include "leds.hpp"
#include "buzzer.hpp"
#include "rfid.hpp"
#include "clock.hpp"
#include "barcode.hpp"

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp32-hal-log.h" //esplogi https://demo-dijiudu.readthedocs.io/en/stable/api-reference/system/log.html
 
#define MAINTAG "Main"

//DEFINES
#define PIR 36
//#define PSENSOR1 39
#define BATTERY 34
#define POWEREN GPIO_NUM_33
#define CLOCKEN 25

//TIMEOUTS
#define TIMEOUTSLEEP 35000
#define TIMEOUTREAD 5000
#define TIMEOUTERROR 20000

//MAIN OBJECTS
extern LEDs led;
extern Buzzer buzzer;
extern RFID rfid;
extern Barcode scanner;
extern Clock myClock;
extern States state;
extern Comms comms;

//MAIN VARIABLES
extern volatile bool bNewInt;
extern long sleepTimer;
extern bool programExit;
extern bool sleepFlag;
extern long readTimer;
extern bool memFullFlag;
extern long errorTimer;
extern long errorStateTime;

//MAIN FUNCTIONS
void IRAM_ATTR readCard();
esp_sleep_wakeup_cause_t print_wakeup_reason();
void espLogsLevel();


#endif