#include "clock.hpp"

void Clock::init(){
  ESP_LOGI(CLOCKTAG, "Starting Clock...");
  while (!DS3231M.begin())  // Initialize RTC communications
  {
  ESP_LOGE(CLOCKTAG, "Unable to find DS3231MM. Checking again.");
  delay(500);
  }                         // of loop until device is located
  float printTemp = DS3231M.temperature() / 100.0;  // Value is in 100ths of a degree
  ESP_LOGI(CLOCKTAG, "DS3231M chip temperature is %f""\xC2\xB0""C", printTemp);
  ESP_LOGI(CLOCKTAG, "DS3231M initialized");

  uint8_t initTime[TIMESTAMPLENGTH];
  getTime(initTime);
}

void Clock::readCommand() {
  /*!
   @brief    Read incoming data from the Serial port
   @details  This function checks the serial port to see if there has been any input. If there is
             data it is read until a terminator is discovered and then the command is parsed and
             acted upon
   @return   void
  */
  static char    text_buffer[SPRINTF_BUFFER_SIZE];  ///< Buffer for sprintf()/sscanf()
  static uint8_t text_index = 0;                    ///< Variable for buffer position
  while (Serial.available())                        // Loop while there is incoming serial data
  {
    text_buffer[text_index] = Serial.read();  // Get the next byte of data
    // keep on reading until a newline shows up or the buffer is full
    if (text_buffer[text_index] != '\n' && text_index < SPRINTF_BUFFER_SIZE) {
      text_index++;
    } else {
      text_buffer[text_index] = 0;              // Add the termination character
      for (uint8_t i = 0; i < text_index; i++)  // Convert the whole input buffer to uppercase
      {
        text_buffer[i] = toupper(text_buffer[i]);
      }  // for-next all characters in buffer
      Serial.print(F("\nCommand \""));
      Serial.write(text_buffer);
      Serial.print(F("\" received.\n"));
      /*********************************************************************************************
      ** Parse the single-line command and perform the appropriate action. The current list of    **
      ** commands understood are as follows:                                                      **
      **                                                                                          **
      ** SETDATE      - Set the device time                                                       **
      **                                                                                          **
      *********************************************************************************************/
      enum commands { SetDate, Unknown_Command };  // enumerate all commands
      commands command;                            // declare enumerated type
      char     workBuffer[SPRINTF_BUFFER_SIZE];    // Buffer to hold string compare
      sscanf(text_buffer, "%s %*s", workBuffer);   // Parse the string for first word
      if (!strcmp(workBuffer, "SETDATE")) {
        command = SetDate;  // Set command number when found
      } else {
        command = Unknown_Command;  // Otherwise set to not found
      }                             // if-then-else a known command
      unsigned int tokens, year, month, day, hour, minute,
          second;  // Variables to hold parsed date/time
      switch (command) {
        /*********************************
        ** Set the device time and date **
        *********************************/
        case SetDate:
          // Use sscanf() to parse the date/time into component variables
          tokens = sscanf(text_buffer, "%*s %u-%u-%u %u:%u:%u;", &year, &month, &day, &hour,
                          &minute, &second);
          if (tokens != 6)  // Check to see if it was parsed correctly
          {
            Serial.print(F("Unable to parse date/time\n"));
          } else {
            DS3231M.adjust(
                DateTime(year, month, day, hour, minute, second));  // Adjust the RTC date/time
            Serial.print(F("Date has been set."));
            checkAlarm();
          }  // of if-then-else the date could be parsed
          break;
        /********************
        ** Unknown command **
        ********************/
        case Unknown_Command:  // Show options on bad command
        default:
          Serial.println(F("Unknown command. Valid commands are:"));
          Serial.println(F("SETDATE yyyy-mm-dd hh:mm:ss"));
      }                // of switch statement to execute commands
      text_index = 0;  // reset the counter
    }                  // of if-then-else we've received full command
  }                    // of if-then there is something in our input buffer
}  // of method readCommand

void Clock::setTime(){
  DS3231M.pinSquareWave();  // Make INT/SQW pin toggle at 1Hz
  DS3231M.adjust();  // Set to library compile Date/Time 
  Serial.println(F("\nEnter the following serial command:"));
  Serial.println(F("SETDATE yyyy-mm-dd hh:mm:ss"));

  while(true){
    readCommand();
    delay(500);
  }
}

void Clock::getTime(uint8_t* time){
  DateTime       now = DS3231M.now();  // get the current time from device
  time[0] = (uint8_t)now.year()-2000;
  time[1] = now.month();
  time[2] = now.day();
  time[3] = now.hour();
  time[4] = now.minute();
  time[5] = now.second();  

  ESP_LOGI(CLOCKTAG, "Current time: 20%02d-%02d-%02d %02d:%02d:%02d\n", time[0], time[1], time[2], time[3], time[4], time[5]);  
}

void Clock::setAlarm(uint8_t aHour, uint8_t aMinute){
  DS3231M.pinAlarm();
  DS3231M.setAlarm(minutesHoursMatch, DateTime(2021, 1, 1, aHour, aMinute, 0));
  ESP_LOGI(CLOCKTAG, "Alarm set for: %02d:%02d\n", aHour, aMinute);
}

bool Clock::checkAlarm(){
  DS3231M.pinAlarm();
  delay(10);
  if(DS3231M.isAlarm()){
    ESP_LOGI(CLOCKTAG, "Alarm detected!");
    delay(10);
    DS3231M.clearAlarm();
    return true;
  }
  return false;
}
