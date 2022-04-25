#include "comms.hpp"

//WIFI
uint8_t masterAddress[];
uint8_t visionAddress[];
String success;
bool datasent;
bool receiveFW = false;
bool updated = false;
File FWFile;

version_struct fwVersion;
States state;
sns_values sensorsRead;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status == 0){
    success = "Delivery Success :)";
    datasent = true;
  }
  else{
    success = "Delivery Fail :(";
    datasent = false;
  }
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  uint8_t id;
  memcpy(&id, incomingData, sizeof(id));
  // memcpy(&myData, incomingData, sizeof(myData));
  ESP_LOGI(COMMSTAG, "Bytes received: %i", len);
  ESP_LOGI(COMMSTAG, "Message id: %x\n", id);
  Serial.printf("id: %x\n", id);
  
  if(!receiveFW){
    switch(id){
      case SNS_OPEN_FRONT:
        state.setStatus(OPENFD);
        break;

      case SNS_RUN_READING:
        state.setStatus(READ);
        break;

      case SNS_EVACUATE:
      break;

      case VIS_SEND_FW:
        receiveFW = true;
        if (!SPIFFS.begin(true)){
          ESP_LOGE(COMMSTAG, "Error mounting SPIFFS");
        }
        delay(100);
        SPIFFS.remove("/firmware.bin");
        state.setStatus(UPLOAD);
        delay(350);
        FWFile = SPIFFS.open("/firmware.bin", FILE_APPEND);
        break;
    }
  }else{
    uint8_t bytesMssg[len];
    memcpy(&bytesMssg, incomingData, sizeof(bytesMssg));
    if(len<5 && bytesMssg[0]==VIS_END_SEND_FW){
      receiveFW = false;
      updated = true;
      ESP_LOGI(COMMSTAG, "firmware received!");
      FWFile.close();
      delay(5);
    }else{
      for(int i=0; i<len; i++){
        FWFile.write((byte)bytesMssg[i]);
      }
    }

    delay(1);
  }

  // int metal;
  // memcpy(&metal, &myData.mssg, sizeof(metal));
  // Serial.printf("Content: %i\n", metal);
}

bool Comms::init(){
  if(!getMACs()){
    return false;
  }

  memcpy(&fwVersion.version, firmwareVersion, sizeof(firmwareVersion));

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
      ESP_LOGE(COMMSTAG, "Error initializing ESP-NOW");
  }
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  // Register peer
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  // Add peer
  memcpy(peerInfo.peer_addr, masterAddress, 6); 
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    ESP_LOGE(COMMSTAG, "Failed to add sensors peer");
  }
  memcpy(peerInfo.peer_addr, visionAddress, 6); 
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    ESP_LOGE(COMMSTAG, "Failed to add sensors peer");
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);

  return true;
}

bool Comms::sendMssg(uint8_t address[], uint8_t id, uint8_t mssg){
  mssg_struct mssgSend;
  mssgSend.id = id;
  mssgSend.mssg = mssg;
  
  esp_err_t mssg1 = esp_now_send(masterAddress, (uint8_t *)&mssgSend, sizeof(mssgSend));

  if(mssg1 == ESP_OK){
    ESP_LOGD(COMMSTAG, "mssg sent with success");
  }else{
    ESP_LOGE(COMMSTAG, "Error sending mssg");
    delay(100);
    return sendMssg(address, id, mssg);
  }

  return true;
}

bool Comms::sendSensors(){

  esp_err_t mssg1 = esp_now_send(masterAddress, (uint8_t *)&sensorsRead, sizeof(sensorsRead));

  if(mssg1 == ESP_OK){
    ESP_LOGD(COMMSTAG, "mssg sent with success");
  }else{
    ESP_LOGE(COMMSTAG, "Error sending mssg");
    delay(100);
    return sendSensors();
  }

  return true;
}

bool Comms::sendVersion(){

  esp_err_t mssg1 = esp_now_send(visionAddress, (uint8_t *)&fwVersion, sizeof(fwVersion));

  if(mssg1 == ESP_OK){
    ESP_LOGD(COMMSTAG, "mssg sent with success");
  }else{
    ESP_LOGE(COMMSTAG, "Error sending mssg, retrying...");
    delay(100);
    return sendVersion();
  }

  return true;
}

bool Comms::getMACs(){
    if(!SPIFFS.begin(true)){
      ESP_LOGE(COMMSTAG, "Error mounting SPIFFS");
    }
    File macAddress = SPIFFS.open("/macAddress.json", "r");
    if(!macAddress){
      ESP_LOGE(COMMSTAG, "Failed to open macAdress config file");
      return false;
    }
    size_t size = macAddress.size();
    std::unique_ptr<char[]> buf(new char[size]);
    macAddress.readBytes(buf.get(), size);
    StaticJsonDocument<600> json;
    DeserializationError error = deserializeJson(json, buf.get());
    if(error){
      ESP_LOGE(COMMSTAG, "Failed to parse macAdress config file");
    }
    
    const char *masterMAC = json["master"];
    if(masterMAC!=NULL){
      for(int i=0; i<6; i++){
        char hexN[2];
        memcpy(hexN, &masterMAC[i*2], 2);
        masterAddress[i] = strtol(hexN, NULL, 16);
      }
    }

    const char *visionMAC = json["vision"];
    if(visionMAC!=NULL){
      for(int i=0; i<6; i++){
        char hexN[2];
        memcpy(hexN, &visionMAC[i*2], 2);
        visionAddress[i] = strtol(hexN, NULL, 16);
      }
    }

    return true;
}
