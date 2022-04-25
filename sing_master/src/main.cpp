#include <Arduino.h>
#include "device.hpp"

#include <driver/rtc_io.h>

void setup(){

    disableCore0WDT();
    //disableCore1WDT(); //Only with espidf
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

    esp_sleep_wakeup_cause_t wakeupReason = print_wakeup_reason();

    // EEPROM.begin(EEPROM_SIZE);
    ESP_LOGI(MAINTAG, "Firmware version: %s\n", firmwareVersion);
 
    ESP_LOGI(MAINTAG, "---------------------- MASTER SETUP INIT ----------------------\n");

    //BATTERY
    pinMode(BATTERY, INPUT);

    //POWER SENSORS
    rtc_gpio_init(POWEREN); //initialize the RTC GPIO port
    rtc_gpio_set_direction(POWEREN, RTC_GPIO_MODE_OUTPUT_ONLY); //set the port to output only mode
    rtc_gpio_hold_dis(POWEREN); //disable hold before setting the level
    rtc_gpio_set_level(POWEREN, LOW); //set high/lo
    delay(250);
    rtc_gpio_set_level(POWEREN, HIGH);
    ESP_LOGI(MAINTAG, "Power in SENSORS & VISION boards activated\n");

    //WIFI COMMUNICATIONS
    comms.init();

    //BUZZER 
    buzzer.init();

    //LEDs
    led.init();

    //PIR
    pinMode(PIR, INPUT);

    //BARCODE SCANNER & BBDD
    scanner.init();
    scanner.readBBDDVersion();

    //Clock
    pinMode(CLOCKEN, OUTPUT);
    digitalWrite(CLOCKEN, HIGH);
    
    while((visionState!=VIS_STATE_READY) && (visionState!=VIS_STATE_MEMFULL) && (visionState!=VIS_STATE_READY_LAST)){
        delay(50);
    } 

    if((wakeupReason==ESP_SLEEP_WAKEUP_EXT1)||(visionState==VIS_STATE_MEMFULL)){
        myClock.init();
        myClock.checkAlarm();
        delay(10);
        // state.setStatus(UPLOAD);
    }

    while((sensorsState!=SNS_STATE_INIT) && (sensorsState!=SNS_STATE_ERRORD) && (sensorsState!=SNS_STATE_IDLE)){
        delay(50);
    }
    
    if((sensorsState==SNS_STATE_INIT)||(sensorsState==SNS_STATE_IDLE)){
        //RFID
        rfid.init();
        bNewInt = false; //interrupt flag
        pinMode(IRQ_PIN, INPUT_PULLUP); //setup the IRQ pin
        attachInterrupt(digitalPinToInterrupt(IRQ_PIN), readCard, FALLING); //Activate the interrupt
        rfid.activateRec();
        buzzer.beep();
        sleepFlag = false;
        sleepTimer = millis();
        if(EEPROM.read(0)==1){
            EEPROM.write(0, 0);
            EEPROM.commit();
            byte i2cUser[USERLENGTH];
            for(int i=0; i<USERLENGTH; i++){
                i2cUser[i] = EEPROM.read(i+1);
            }
            rfid.setUser(i2cUser);
        }else{
            bool keyFlag = false;
            while(!keyFlag && !sleepFlag){
                led.led1Blink(GREEN);
                rfid.activateRec(); //The receiving block needs regular retriggering (tell the tag it should transmit)
                delay(20);
                if(rfid.userPresent()){
                    if(rfid.validKey()){
                        keyFlag = true;
                    }else{
                        buzzer.beep();
                        led.led1BlinkLoop(RED);
                        byte zero[USERLENGTH];
                        memset(zero, 0, USERLENGTH);
                        rfid.setUser(zero);
                    }
                }
                if(millis()-sleepTimer>TIMEOUTSLEEP){
                    sleepFlag = true;
                }
            }
        }
        if(!sleepFlag){
            buzzer.beep();
            bNewInt = false;
            byte printUser[4];
            memcpy(printUser, rfid.getUser(), 4);
            ESP_LOGI(RFIDTAG, "Card UID: %02X %02X %02X %02X\n", printUser[0], printUser[1], printUser[2], printUser[3]);
            led.led1ON(GREEN);

            myClock.init();
            myClock.setAlarm(3, 0); //Hour, Minute
            //myClock.setTime();

            //BATTERY
            long batDigitalLevel = analogRead(BATTERY);
            long batPowerLevel = (batDigitalLevel*13300)/3300;
            ESP_LOGI(MAINTAG, "Battery power source level: %ld mV\n", batPowerLevel);

            delay(10);

            state.setStatus(BARCODE); //BARCODE

        }else{
            delay(10);
            state.setStatus(SLEEP); //SLEEP
        }
    }else{
        buzzer.beep();
        state.setStatus(ERROR);
    }

    ESP_LOGI(MAINTAG, "---------------------- END SETUP ----------------------\n"); 

    //enableLoopWDT(); //*Enable Watch Dog
}

void loop(){
    delay(10);
    rfid.activateRec(); //The receiving block needs regular retriggering (tell the tag it should transmit??)

    switch(state.getStatus()){

        case OFF:
            break;

        case SLEEP:
            state.taskSleep();
            break;

        case LOGIN:
            break;

        case INIT:
            break;

        case BARCODE:
            state.taskBarcode();
            break;
        
        case OPENFD:
            state.taskOpenFD();
            break;

        case READ:
            state.taskRead();
            break;

        case SENDINFO:
            state.taskSendInfo();
            break;

        case UPLOAD:
            state.taskUpload();
            break;

        case ERROR:
            state.taskError();
            break;

        case TEST:
            state.taskTest();
            break;
        
        default:
            break;
    }
}
 
