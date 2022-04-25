#include "motors.hpp"

bool Motors::init(){
  
    //Finales de carrera
    pinMode(M1FC1, INPUT);
    pinMode(M1FC2, INPUT);
    pinMode(M2FC1, INPUT);
    pinMode(M2FC2, INPUT);

    // configure LED PWM functionalitites
    //motor1
    ledcSetup(M1Left, freq, resolution);
    ledcSetup(M1Right, freq, resolution);
    //motor2
    ledcSetup(M2Left, freq, resolution);
    ledcSetup(M2Right, freq, resolution);

    // attach the channel to the GPIO to be controlled
    //motor1
    ledcAttachPin(M1LP, M1Left);
    ledcAttachPin(M1RP, M1Right);
    //motor2
    ledcAttachPin(M2LP, M2Left);
    ledcAttachPin(M2RP, M2Right);

    ledcWrite(M1Left, 0);
    ledcWrite(M1Right, 0);
    ledcWrite(M2Left, 0);
    ledcWrite(M2Right, 0);

    if(!home()){
        ESP_LOGE(MOTORSTAG, "Motors blocked, please unblock them");
        return false;
    }

    ESP_LOGI(MOTORSTAG, "Motors initialized\n");
    return true;
}

bool Motors::openM1(){
    long mTime = millis(); 
    ledcWrite(M1Right, velocityM1); //iniciamos movimiento
    while(digitalRead(M1FC1)==LOW){ //Lectura del sensor
        delay(10);
        if(millis()-mTime>TIMEOUTMOTORS){
            ESP_LOGE(MOTORSTAG, "Motor 1 can't reach open position");
            if(unblockM1(MRIGHT)){ //Intentamos desbloquear el motor
                ESP_LOGI(MOTORSTAG, "Front door opened\n");
                return true;
            }else{
                return false;
            }
        }
    }
    ledcWrite(M1Right, 0); //cortamos movimiento
    fOpen = true;
    ESP_LOGI(MOTORSTAG, "Front door opened\n");
    return true;
}

bool Motors::openM1Static(bool initM){
    if(initM){
        ledcWrite(M1Right, velocityM1);
    }
    if(digitalRead(M1FC1)==HIGH){
        ledcWrite(M1Right, 0);
        //ESP_LOGI(MOTORSTAG, "Front door opened\n");
        return true;
    }
    return false;
}

bool Motors::closeM1(){
    long mTime = millis(); 
    ledcWrite(M1Left, velocityM1); //iniciamos movimiento
    while(digitalRead(M1FC2)==LOW){ //Lectura del sensor
        delay(10);
        if(millis()-mTime>TIMEOUTMOTORS){
            ESP_LOGE(MOTORSTAG, "Motor 1 can't reach close position");
            if(unblockM1(MLEFT)){ //Intentamos desbloquear el motor
                ESP_LOGI(MOTORSTAG, "Front door closed\n");
                return true;
            }else{
                return false;
            }
        }
    }
    ledcWrite(M1Left, 0); //cortamos movimiento
    fOpen = false;
    ESP_LOGI(MOTORSTAG, "Front door closed\n");
    return true;
}

bool Motors::closeM1Static(bool initM){
    static bool endDelay = false;
    if(initM){
        ledcWrite(M1Left, velocityM1);
        endDelay = true;
    }
    if(digitalRead(M1FC2)==HIGH){
        if(endDelay){
            delay(150);
            endDelay = false;
        }
        ledcWrite(M1Left, 0);
        //ESP_LOGI(MOTORSTAG, "Front door closed\n");
        return true;
    }
    return false;
}

bool Motors::openM2(){
    long mTime = millis(); 
    ledcWrite(M2Right, velocityM2); //iniciamos movimiento
    while(digitalRead(M2FC1)==LOW){ //Lectura del sensor
        delay(10);
        if(millis()-mTime>TIMEOUTMOTORS){
            ESP_LOGE(MOTORSTAG, "Motor 2 can't reach open position");
            if(unblockM2(MRIGHT)){ //Intentamos desbloquear el motor
                ESP_LOGI(MOTORSTAG, "Rear door opened\n");
                return true;
            }else{
                return false;
            }
        }
    }
    ledcWrite(M2Right, 0); //cortamos movimiento
    rOpen = true;
    ESP_LOGI(MOTORSTAG, "Rear door opened\n");
    return true;
}

bool Motors::openM2Static(bool initM){
    if(initM){
        ledcWrite(M2Right, velocityM2);
    }
    if(digitalRead(M2FC1)==HIGH){
        ledcWrite(M2Right, 0);
        //ESP_LOGI(MOTORSTAG, "Rear door opened\n");
        return true;
    }
    return false;
}

bool Motors::closeM2(){
    long mTime = millis();
    ledcWrite(M2Left, velocityM2); //iniciamos movimiento
    while(digitalRead(M2FC2)==LOW){ //Lectura del sensor
        delay(150);
        if(millis()-mTime>TIMEOUTMOTORS){
            ESP_LOGE(MOTORSTAG, "Motor 2 can't reach close position");
            if(unblockM2(MLEFT)){ //Intentamos desbloquear el motor
                ESP_LOGI(MOTORSTAG, "Rear door closed\n");
                return true;
            }else{
                return false;
            }
        }
    }
    ledcWrite(M2Left, 0); //cortamos movimiento
    rOpen = false;
    ESP_LOGI(MOTORSTAG, "Rear door closed\n");
    return true;
}

bool Motors::closeM2Static(bool initM){
    if(initM){
        ledcWrite(M2Left, velocityM2);
    }
    if(digitalRead(M2FC2)==HIGH){
        ledcWrite(M2Left, 0);
        //ESP_LOGI(MOTORSTAG, "Rear door closed\n");
        return true;
    }
    return false;
}

bool Motors::home(){
    if(closeM1()){
        if(closeM2()){
            return true;
        }
    }
    return false;
}

bool Motors::unblockM1(int direction){
    ledcWrite(M2Left, 0);
    ledcWrite(M2Right, 0);
    ledcWrite(M1Left, 0);
    ledcWrite(M1Right, 0);
    ESP_LOGW(MOTORSTAG, "Trying to unblock motor 1...");
    for(int i=1; i<3; i++){
        ESP_LOGW(MOTORSTAG, "Try number: %i", i);
        if(direction==MRIGHT){
            long mUnblockTime = millis();
            ledcWrite(M1Left, velocityM1);
            while(millis()-mUnblockTime<TIMEOUTMOTORS){
                if(digitalRead(M1FC2)==HIGH){
                    ledcWrite(M1Left, 0);
                    ledcWrite(M1Right, velocityM1);
                    long mUnblockTime2 = millis();
                    while(millis()-mUnblockTime2<TIMEOUTMOTORS){
                        if(digitalRead(M1FC1)==HIGH){
                            ledcWrite(M1Right, 0);
                            ESP_LOGI(MOTORSTAG, "Motor 1 unblocked!");
                            fOpen = true;
                            return true;
                        }
                        delay(10);
                    }
                }
                delay(10);
            }
            ledcWrite(M1Left, 0);
            ledcWrite(M1Right, 0);
        }else{
            long mUnblockTime = millis();
            ledcWrite(M1Right, velocityM1);
            while(millis()-mUnblockTime<TIMEOUTMOTORS){
                if(digitalRead(M1FC1)==HIGH){
                    ledcWrite(M1Right, 0);
                    ledcWrite(M1Left, velocityM1);
                    long mUnblockTime2 = millis();
                    while(millis()-mUnblockTime2<TIMEOUTMOTORS){
                        if(digitalRead(M1FC2)==HIGH){
                            ledcWrite(M1Left, 0);
                            ESP_LOGI(MOTORSTAG, "Motor 1 unblocked!");
                            fOpen = false;
                            return true;
                        }
                        delay(10);
                    }
                }
                delay(10);
            }
            ledcWrite(M1Right, 0);
            ledcWrite(M1Left, 0);
        }
    }
    ESP_LOGE(MOTORSTAG, "Can't unblock motor 1");
    return false;
}

bool Motors::unblockM2(int direction){
    ledcWrite(M1Left, 0);
    ledcWrite(M1Right, 0);
    ledcWrite(M2Left, 0);
    ledcWrite(M2Right, 0);
    ESP_LOGW(MOTORSTAG, "Trying to unblock motor 2...");
    for(int i=1; i<3; i++){
        ESP_LOGW(MOTORSTAG, "Try number: %i", i);
        if(direction==MRIGHT){
            long mUnblockTime = millis();
            ledcWrite(M2Left, velocityM2);
            while(millis()-mUnblockTime<TIMEOUTMOTORS){
                if(digitalRead(M2FC2)==HIGH){
                    delay(1000);
                    ledcWrite(M2Left, 0);
                    ledcWrite(M2Right, velocityM2);
                    long mUnblockTime2 = millis();
                    while(millis()-mUnblockTime2<TIMEOUTMOTORS){
                        if(digitalRead(M2FC1)==HIGH){
                            ledcWrite(M2Right, 0);
                            ESP_LOGI(MOTORSTAG, "Motor 2 unblocked!");
                            rOpen = true;
                            return true;
                        }
                        delay(10);
                    }
                }
                delay(10);
            }
            ledcWrite(M2Left, 0);
            ledcWrite(M2Right, 0);
        }else{
            long mUnblockTime = millis();
            ledcWrite(M2Right, velocityM2);
            while(millis()-mUnblockTime<TIMEOUTMOTORS){
                if(digitalRead(M2FC1)==HIGH){
                    ledcWrite(M2Right, 0);
                    ledcWrite(M2Left, velocityM2);
                    long mUnblockTime2 = millis();
                    while(millis()-mUnblockTime2<TIMEOUTMOTORS){
                        if(digitalRead(M2FC2)==HIGH){
                            ledcWrite(M2Left, 0);
                            ESP_LOGI(MOTORSTAG, "Motor 2 unblocked!");
                            rOpen = false;
                            return true;
                        }
                        delay(10);
                    }
                }
                delay(10);
            }
            ledcWrite(M2Right, 0);
            ledcWrite(M2Left, 0);
        }
    }
    ESP_LOGE(MOTORSTAG, "Can't unblock motor 2");
    return false;
}

