#include "states.hpp"
#include "device.hpp"
#include "SPIFFS.h"
#include "iostream"
#include "cstdio"

struct lastDate{
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
};


void States::setStatus(MachineStates s){
    if(status != s){
        newState = true;
        if(s==0){
            ESP_LOGD(STATESTAG, "Cambio de estado a: TEST\n");
        }else if(s==1){
            ESP_LOGD(STATESTAG, "Cambio de estado a: OFF\n");
        }else if(s==2){
            ESP_LOGD(STATESTAG, "Cambio de estado a: SLEEP\n");
        }else if(s==3){
            ESP_LOGD(STATESTAG, "Cambio de estado a: LOGIN\n");
        }else if(s==4){
            ESP_LOGD(STATESTAG, "Cambio de estado a: INIT\n");
        }else if(s==5){
            ESP_LOGD(STATESTAG, "Cambio de estado a: BARCODE\n");
        }else if(s==6){
            ESP_LOGD(STATESTAG, "Cambio de estado a: OPENFD\n");
        }else if(s==7){
            ESP_LOGD(STATESTAG, "Cambio de estado a: READ\n");
        }else if(s==8){
            ESP_LOGD(STATESTAG, "Cambio de estado a: SENDINFO\n");
        }else if(s==9){
            ESP_LOGD(STATESTAG, "Cambio de estado a: UPLOAD\n");
        }else if(s==10){
            ESP_LOGD(STATESTAG, "Cambio de estado a: ERROR\n");
        }else{
            ESP_LOGD(STATESTAG, "State not implemented\n");
        }
    }
    lastStatus = status;
    status = s;
}

void States::taskSleep(){
    if(newState == true){
        newState = false;

    }

    //Battery level read
    // long batDigitalLevel = analogRead(BATTERY);
    // long batPowerLevel = (batDigitalLevel*13300)/3300;
    // ESP_LOGI(MAINTAG, "Battery power source level: %ld mV\n", batPowerLevel);
 
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_36, 1); //1 = High, 0 = Low //36
    esp_sleep_enable_ext1_wakeup(WAKEUP_CLOCK_PIN_BITMASK, ESP_EXT1_WAKEUP_ALL_LOW); //1 = High, 0 = Low //Wake up para la alarma del Real Time Cloc
    
    buzzer.beep();
    led.ledsOFF();

    rtc_gpio_set_level(POWEREN, LOW); //set high/low 1/0
    gpio_hold_en(POWEREN);

    ESP_LOGI(STATESTAG, "Going to sleep now\n");
    delay(10);
    esp_deep_sleep_start();
}

void States::taskBarcode(){
    if(newState == true){
        newState = false;

        programExit = true;
        led.led1ON(GREEN);
        sleepTimer = millis();
        if(!itemBool.readNextEAN) scanner.aimOn();
    }

    if(itemBool.readNextEAN){
        memcpy(item.EAN, itemBool.nextEAN, EANLENGTH);
        memset(itemBool.nextEAN, 0, EANLENGTH);
        itemBool.readNextEAN = false;
        uint8_t bbddAnswer = scanner.comparebbdd(item.EAN);
        if(bbddAnswer == 1){
            buzzer.beep();
            scanner.printEAN(item.EAN);
            itemBool.readEAN = true;
            led.led2ON(GREEN);
            setStatus(OPENFD);
        }else if(bbddAnswer == 2){
            buzzer.beep();
            led.led2ON(ORANGE);
            scanner.printEAN(item.EAN);
            itemBool.readEAN = true;
            setStatus(OPENFD);
        }else{
            buzzer.beep();
            led.led2BlinkLoop(RED);
            scanner.printEAN(item.EAN);
            itemBool.readEAN = false;
            scanner.aimOn();
            setStatus(BARCODE);
        }
    }else{
        led.led2Blink(GREEN);
        itemBool.readEAN = scanner.readCode(item.EAN);
        if(itemBool.readEAN){
            buzzer.beep();
            uint8_t bbddAnswer = scanner.comparebbdd(item.EAN);
            if(bbddAnswer == 1){
                scanner.printEAN(item.EAN);
                itemBool.readEAN = true;
                led.led2ON(GREEN);
                setStatus(OPENFD);
            }else if(bbddAnswer == 2){
                led.led2ON(ORANGE);
                scanner.printEAN(item.EAN);
                itemBool.readEAN = true;
                setStatus(OPENFD);
            }else{
                led.led2BlinkLoop(RED);
                scanner.printEAN(item.EAN);
                itemBool.readEAN = false;
                scanner.aimOn();
                setStatus(BARCODE);
            }
        }
    }

    rfid.activateRec();
    if(bNewInt){
        delay(100);
        bNewInt = false;
        if(rfid.validKey()){
            bool sameUser = rfid.cmpUsers();
            if(sameUser){
                setStatus(SLEEP); //! Problema en la funcion cmpUsers, casi siempre detecta 2 tarjetas iguales. 
            }else{
                buzzer.tripleBeep();
                byte printUser[4];
                memcpy(printUser, rfid.getUser(), 4);
                ESP_LOGI(RFIDTAG, "Card UID: %02X %02X %02X %02X\n", printUser[0], printUser[1], printUser[2], printUser[3]);
                led.led1BlinkLoop(BLUE);
                led.led1ON(GREEN);
                setStatus(BARCODE);
            } 
        }else{
            buzzer.beep();
            led.led1BlinkLoop(RED);
            setStatus(SLEEP);
        }
    }


    if(millis()-sleepTimer>TIMEOUTSLEEP){
        setStatus(SLEEP);
    }
}

void States::taskOpenFD(){
    if(newState == true){
        newState = false;

        //led.led2ON(GREEN);  

        errorTimer = millis();       
    }

    if(sensorsState==SNS_STATE_IDLE){
        delay(10);
        comms.sendMssg(sensorsAddress, SNS_OPEN_FRONT);
        delay(1000); //Time for open & close front door - //?Revisar!
    }else if(sensorsState==SNS_STATE_ERRORD){
        ESP_LOGE(STATESTAG, "Error message received");
        setStatus(ERROR);
    }else if(sensorsState==SNS_STATE_WFREAD){
        delay(50);

        if(visionState==VIS_STATE_READY_LAST){
            memFullFlag = true;
            setStatus(READ);
        }else if(visionState==VIS_STATE_READY){
            setStatus(READ);
        }else if(visionState==VIS_STATE_ERROR_INIT){
            setStatus(ERROR);
        }
    }else{
        delay(50);
    }

    if(millis()-errorTimer>TIMEOUTERROR){
        setStatus(READ);
    }
}

void States::taskRead(){
    if(newState == true){
        newState = false;

        comms.sendMssg(visionAddress, VIS_RUN_CAMERA);

        delay(10);

        comms.sendMssg(sensorsAddress, SNS_RUN_READING);

        errorTimer = millis();

        readTimer = millis(); 

        //Battery level read
        unsigned int batDigitalLevel = analogRead(BATTERY);
        item.batteryLvl = (batDigitalLevel*13300)/3300;
        ESP_LOGI(MAINTAG, "Battery power source level: %u mV\n", item.batteryLvl);

        myClock.getTime(item.timestamp);

        scanner.clearSerial();

        scanner.aimOn();

        memcpy(item.userID, rfid.getUser(), sizeof(item.userID));
    }

    if(!itemBool.readNextEAN){
        led.led2Blink(GREEN);
        itemBool.readNextEAN = scanner.readCode(itemBool.nextEAN);
        if(itemBool.readNextEAN){
            buzzer.beep();
            scanner.printEAN(item.EAN);
            itemBool.readNextEAN = true;
            led.led2ON(GREEN);
        }
    }

    if(itemBool.readSensors && itemBool.readPhoto){
        setStatus(SENDINFO);
    }else{
        delay(10);
    }

    if(millis()-errorTimer>TIMEOUTERROR){
        setStatus(SENDINFO);
    }
}

void States::taskSendInfo(){
    if(newState == true){
        newState = false;

        errorTimer = millis();
    }

    if(!itemBool.readNextEAN){ //Refresh EAN read
        led.led2Blink(GREEN);
        itemBool.readNextEAN = scanner.readCode(itemBool.nextEAN);
        if(itemBool.readNextEAN){
            buzzer.beep();
            scanner.printEAN(item.EAN);
            itemBool.readNextEAN = true;
            led.led2ON(GREEN);
        }
    }

    comms.sendItem();

    if(itemBool.sentItem){
        comms.clearItem();
        if(memFullFlag){
            setStatus(UPLOAD);
        }else{
            setStatus(BARCODE);
        }
    }else if(visionState==VIS_STATE_READY){
        comms.clearItem();
        if(memFullFlag){
            setStatus(UPLOAD);
        }else{
            setStatus(BARCODE);
        }
    }

    if(millis()-errorTimer>TIMEOUTERROR){
        setStatus(BARCODE);
    }

}

void States::taskUpload(){
    if(newState == true){
        newState = false;

        led.ledsOFF();
        delay(15);
        led.led1ON(BLUE);

        comms.sendVersion();
        delay(20);
        comms.sendDBVersion(scanner.bbddV);

        while(visionState!=VIS_STATE_MEMFULL){
            delay(50);
        }

        comms.sendMssg(visionAddress, VIS_RUN_UPLOAD);

        ESP_LOGI(STATESTAG, "Data uploading...");

        delay(10000);

    }  

    // while(receiveFW){
    //     delay(1);
    // }

    if(visionState==VIS_STATE_READY || visionState==VIS_STATE_READY_LAST){
        ESP_LOGI(STATESTAG, "Data uploaded!");
        delay(10000);
        if(updated){
            ESP_LOGI(STATESTAG, "Starting update..");
            File file = SPIFFS.open("/firmware.bin");
            if(!file){
                ESP_LOGI(STATESTAG, "Failed to open file for reading");
                return;
            }
            size_t fileSize = file.size();
            ESP_LOGI(STATESTAG, "file size: %zd", fileSize);
            if(!Update.begin(fileSize)){                       
                ESP_LOGI(STATESTAG, "Cannot do the update");
                return;
            };
            Update.writeStream(file);                
            if(Update.end()){                        
                ESP_LOGI(STATESTAG, "Successful update");  
            }else {                      
                ESP_LOGI(STATESTAG, "Error Occurred: %s", String(Update.getError()));
            }     
            file.close();               
            ESP_LOGI(STATESTAG, "Reset in 2 seconds...");
            delay(2000);   
            SPIFFS.remove("/firmware.bin");
            delay(250);   
            ESP.restart();
        }
        setStatus(SLEEP); 
    }else if(visionState==VIS_STATE_ERROR_INIT){
        ESP_LOGE(STATESTAG, "Vision Error");
        setStatus(ERROR); 
    }else if(visionState==VIS_STATE_MEMFULL){
        ESP_LOGE(STATESTAG, "Vision has restarted, restarting master");
        ESP.restart();
    }else if(visionState==VIS_STATE_ERROR_UPLOAD){
        ESP_LOGE(STATESTAG, "Vision has an error uploading, restarting master");
        ESP.restart();        
    }else{  
        delay(5000);
    }
}

void States::taskError(){
    if(newState == true){
        newState = false;

        led.ledsOFF();
        errorStateTime = millis();
    }

    led.led1Blink(RED);
    if(millis()-errorStateTime>TIMEOUTERROR){
        setStatus(SLEEP);
    }
}

void States::taskTest(){
    if(newState == true){
        newState = false;
    }

}