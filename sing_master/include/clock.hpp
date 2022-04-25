#ifndef CLOCK_H
#define CLOCK_H

#include <Arduino.h>
#include <DS3231M.h>  // Include the DS3231M RTC library
#include <Wire.h>

/***************************************************************************************************
** Declare all program constants                                                                  **
***************************************************************************************************/
const uint32_t SERIAL_SPEED{115200};     ///< Set the baud rate for Serial I/O
const uint8_t  SPRINTF_BUFFER_SIZE{32};  ///< Buffer size for sprintf()

#define SDA_PIN 21
#define SCL_PIN 22
#define CLOCKTAG "CLOCK"
#define TIMESTAMPLENGTH 6

class Clock{
    private:
        DS3231M_Class DS3231M;  ///< Create an instance of the DS3231M class
    public:
        Clock(){;}
        void init();
        void readCommand();
        void setTime();
        void getTime(uint8_t* time);
        void setAlarm(uint8_t aHour, uint8_t aMinute);
        bool checkAlarm();
};

#endif