#include <Arduino.h>
#include "device.hpp"

void setup()
{

    disableCore0WDT();
    // disableCore1WDT();
    disableLoopWDT();

    Serial.begin(115200); // Initialize serial communications with the PC

    espLogsLevel();

    Serial.print(F("- Compiled with c++ version "));
    Serial.print(F(__VERSION__));
    Serial.print(F("\n- On "));
    Serial.print(F(__DATE__));
    Serial.print(F(" at "));
    Serial.print(F(__TIME__));
    Serial.print(F("\n"));

    ESP_LOGI(MAINTAG, "---------------------- SENSORS SETUP INIT ----------------------\n");

    ESP_LOGI(MAINTAG, "Firmware version: %s\n", firmwareVersion);

    //Wi-Fi
    comms.init();
    delay(5);

    //motors
    if(!motor.init()){
        ESP_LOGE(MAINTAG, "Can't init motors\n");
        state.setStatus(ERRORD);
    }else{
        state.setStatus(INIT);

        ESP_LOGI(MAINTAG, "---------------------- END SETUP ----------------------\n"); 

        //enableLoopWDT(); //Enable Watch Dog
    }
}

void loop()
{
    delay(10);

    switch (state.getStatus())
    {

    case OFF:
        break;

    case INIT:
        state.taskInit();
        break;

    case IDLE:
        state.taskIDLE();
        break;

    case OPENFD:
        state.taskOpenFD();
        break;

    case PSENSOR:
        state.taskPSensor();
        break;

    case CLOSEFD:
        state.taskCloseFD();
        break;

    case WFREAD:
        state.taskWFRead();
        break;

    case READ:
        state.taskRead();
        break;

    case OPENRD:
        state.taskOpenRD();
        break;

    case CLOSERD:
        state.taskCloseRD();
        break;

    case SENDINFO:
        state.taskSendInfo();
        break;

    case TEST:
        state.taskTest();
        break;

    case ERRORD:
        state.taskErrorD();
        break;

    case UPLOAD:
        state.taskUpload();
        break;

    default:
        break;
    }
}
