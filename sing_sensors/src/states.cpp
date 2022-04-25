#include "states.hpp"
#include "device.hpp"

void States::setStatus(MachineStates s)
{
    if (status != s)
    {
        newState = true;
        if (s == 0)
        {
            ESP_LOGD(STATESTAG, "Cambio de estado a: TEST\n");
        }
        else if (s == 1)
        {
            ESP_LOGD(STATESTAG, "Cambio de estado a: OFF\n");
        }
        else if (s == 2)
        {
            ESP_LOGD(STATESTAG, "Cambio de estado a: INIT\n");
        }
        else if (s == 3)
        {
            ESP_LOGD(STATESTAG, "Cambio de estado a: IDLE\n");
        }
        else if (s == 4)
        {
            ESP_LOGD(STATESTAG, "Cambio de estado a: OPENFD\n");
        }
        else if (s == 5)
        {
            ESP_LOGD(STATESTAG, "Cambio de estado a: PSENSOR\n");
        }
        else if (s == 6)
        {
            ESP_LOGD(STATESTAG, "Cambio de estado a: CLOSEFD\n");
        }
        else if (s == 7)
        {
            ESP_LOGD(STATESTAG, "Cambio de estado a: WFREAD\n");
        }
        else if (s == 8)
        {
            ESP_LOGD(STATESTAG, "Cambio de estado a: READ\n");
        }
        else if (s == 9)
        {
            ESP_LOGD(STATESTAG, "Cambio de estado a: SENDINFO\n");
        }
        else if (s == 10)
        {
            ESP_LOGD(STATESTAG, "Cambio de estado a: OPENRD\n");
        }
        else if (s == 11)
        {
            ESP_LOGD(STATESTAG, "Cambio de estado a: CLOSERD\n");
        }
        else if (s == 12)
        {
            ESP_LOGD(STATESTAG, "Cambio de estado a: ERRORD\n");
        }
        else if (s == 13)
        {
            ESP_LOGD(STATESTAG, "Cambio de estado a: UPLOAD\n");
        }
        else
        {
            ESP_LOGD(STATESTAG, "State not implemented\n");
        }
    }
    lastStatus = status;
    status = s;
    comms.sendMssg(masterAddress, SNS_SEND_STATE, status);
}

void States::taskInit(){
    if (newState == true){
        newState = false;

        initFlag = true;
    }

    if(initFlag){
        //PRESENCE SENSOR
        pinMode(PSENSOR1, INPUT);
        if(!SPIFFS.begin(true)){
        ESP_LOGE(COMMSTAG, "Error mounting SPIFFS");
        }
        File fPSensor = SPIFFS.open("/pSensor.txt");
        if(!fPSensor){
        ESP_LOGE(COMMSTAG, "Failed to open macAdress config file");
        }
        char buffer[10];
        int line = fPSensor.readBytesUntil('\n', buffer, sizeof(buffer)-1);
        // buffer[line-1] = 0;
        pSensorValue = atoi(buffer);
        ESP_LOGI(MAINTAG, "pSensor value: %i", pSensorValue);
        fPSensor.close();

        //PET
        PET.init();

        //Metal
        metal.init();

        // long lectAcum = 0;
        // for(int i=0;i<10;i++){
        //     lectAcum = lectAcum + analogRead(PSENSOR1);
        //     delay(7);
        // }
        // long lecture = lectAcum/10;
        // if(lecture>1000){ //If there's something inside, evacuate 
        //     motor.openM2();
        //     delay(500);
        //     motor.closeM2();
        // }

        comms.sendVersion();

        setStatus(IDLE);
    }
}

void States::taskIDLE()
{
    if (newState == true)
    {
        newState = false;

        motorsTime2 = millis();

        motor.closeM2Static(true);
    }

    if(!motor.closeM2Static(false)){
        if(millis()-motorsTime2>TIMEOUTMOTORS){
            if(motor.unblockM2(MLEFT)){
                ESP_LOGI(STATESTAG, "Rear door closed\n");
            }else{
                ESP_LOGE(STATESTAG, "Rear door blocked, please unblock it\n");
                setStatus(ERRORD);
            }
        }
    }

    delay(5);

}

void States::taskOpenFD()
{
    if (newState == true)
    {
        newState = false;

        motorsTime = millis();
        motorsTime2 = millis();

        motor.closeM2Static(true);

        motor.openM1Static(true);

        metal.calibrate();
    }

    if(motor.openM1Static(false) && motor.closeM2Static(false)){
        ESP_LOGI(STATESTAG, "Front door opened\n");
        state.setStatus(PSENSOR);

    }else{
        if(!motor.openM1Static(false)){
            if(millis()-motorsTime>TIMEOUTMOTORS){
                if(motor.unblockM1(MRIGHT)){
                    ESP_LOGI(STATESTAG, "Front door opened\n");
                    setStatus(OPENFD);
                }else{
                    ESP_LOGE(STATESTAG, "Front door blocked, please unblock it\n");
                    setStatus(ERRORD);
                }
            }
        }
        if(!motor.closeM2Static(false)){
            if(millis()-motorsTime2>TIMEOUTMOTORS){
                if(motor.unblockM2(MLEFT)){
                    ESP_LOGI(STATESTAG, "Rear door closed\n");
                    setStatus(OPENFD);
                }else{
                    ESP_LOGE(STATESTAG, "Rear door blocked, please unblock it\n");
                    setStatus(ERRORD);
                }
            }
        }
    }
}

void States::taskPSensor(){
    if (newState == true)
    {
        newState = false;

        pSensorTime = millis();
    }

    long lectAcum = 0;
    for(int i=0;i<10;i++){
        lectAcum = lectAcum + analogRead(PSENSOR1);
        delay(13);
    }
    long lecture = lectAcum/10;
    ESP_LOGV(STATESTAG, "Presence sensor value: %ld", lecture);
    if((lecture>pSensorValue) || (millis()-pSensorTime>TIMEOUTPSENSOR)){
        setStatus(CLOSEFD);
    }

}

void States::taskCloseFD()
{
    if (newState == true)
    {
        newState = false;

        motorsTime = millis();

        motor.closeM1Static(true);

        PET.lightON();
    }

    if(motor.closeM1Static(false)){
        state.setStatus(WFREAD);
    }else{
        if(millis()-motorsTime>TIMEOUTMOTORS){
            if(motor.unblockM1(MLEFT)){
                ESP_LOGI(STATESTAG, "front door closed\n");
                state.setStatus(WFREAD);
            }else{
                ESP_LOGE(STATESTAG, "front door blocked, please unblock it\n");
                PET.lightOFF();
                setStatus(ERRORD);
            }
        }
    }

}

void States::taskWFRead()
{
    if (newState == true)
    {
        newState = false;
        object.clearItem();

        errorTimer = millis();
    }

    delay(5);

    if(millis()-errorTimer>TIMEOUTERROR){
        setStatus(READ);
    }

}

void States::taskRead()
{
    if (newState == true){
        newState = false;

        PET.start_meas();

        errorTimer = millis();
        
        //Metal Read
        if (!object.readMetal){
            object.metalTrys++;
            object.readMetal = metal.detectCan(&sensorsRead.metal_val);
            if (object.metalTrys > 0){
                object.readMetal = true;
            }
        }

        petTimer = millis();

    }else{
        //PET Read
        if(millis()-petTimer>PETREADTIMER){
            if(!object.readPet){
                if(PET.read_meas()){
                    object.petTrys++;
                    object.readPet = PET.measure(&sensorsRead.pet_val);
                    if (object.petTrys > 0){
                        object.readPet = true;
                    }
                }
            }
        }

        if (object.readMetal && object.readPet){
            ESP_LOGI(STATESTAG, "Metal variable: %i\n", object.metal);
            ESP_LOGI(STATESTAG, "pet variable: %f\n", object.pet);
            setStatus(SENDINFO);
        }

        if(millis()-errorTimer>TIMEOUTERROR){
            setStatus(SENDINFO);
        }
    }
}

void States::taskSendInfo()
{
    if (newState == true){
        newState = false;

    }

    comms.sendSensors();
    object.sentMetal = true;
    object.sentPet = true;

    setStatus(OPENRD);
}

void States::taskOpenRD()
{
    if (newState == true){
        newState = false;

        motorsTime = millis();

        motor.openM2Static(true);
    }

    if(motor.openM2Static(false)){
        state.setStatus(CLOSERD);
    }else{
        if(millis()-motorsTime>TIMEOUTMOTORS){
            if(motor.unblockM2(MRIGHT)){
                ESP_LOGI(STATESTAG, "Rear door opened\n");
                state.setStatus(CLOSERD);
            }else{
                ESP_LOGE(STATESTAG, "Rear door blocked, please unblock it\n");
                setStatus(ERRORD);
            }
        }
    }
}

void States::taskCloseRD()
{
    if (newState == true){
        newState = false;

        errorTimer = millis();
    }

    if(object.sentMetal && object.sentPet){
        long lectAcum = 0;
        for(int i=0;i<10;i++){
            lectAcum = lectAcum + analogRead(PSENSOR1);
            delay(7);
        }
        long lecture = lectAcum/10;
        if(lecture<pSensorValue){
            setStatus(IDLE);
            delay(100);
        }else{
            delay(250);
        }
    }

    if(millis()-errorTimer>TIMEOUTERROR){
        setStatus(IDLE);
    }

}

void States::taskUpload(){
    if (newState == true){
        newState = false;

        while(!updated){
            delay(1);
        }
    }

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
        return;
    }     
    file.close();               
    ESP_LOGI(STATESTAG, "Reset in 2 seconds...");
    delay(2000);   
    SPIFFS.remove("/firmware.bin");
    delay(250);   
    ESP.restart();
}

void States::taskErrorD(){
    if (newState == true){
        newState = false;

        comms.sendVersion();
    }
    delay(5);
}

void States::taskTest(){ 
    delay(100); 
}