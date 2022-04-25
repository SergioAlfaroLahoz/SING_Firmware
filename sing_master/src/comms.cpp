#include "comms.hpp"

//WIFI
uint8_t sensorsAddress[];
uint8_t visionAddress[];
String success;
bool datasent;
uint8_t receiveFW = 0;
bool updated = false;
File FWFile;
File DBFile;

mssg_struct mssg;
sns_values sensorsValues;
item_struct item;
item_bools itemBool;
version_struct fwVersion;
SensorsMachineStates sensorsState;
VisionMachineStates visionState;

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
  //dataRcv = false;
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  uint8_t id;
  memcpy(&id, incomingData, sizeof(id));
  // memcpy(&myData, incomingData, sizeof(myData));
  ESP_LOGI(COMMSTAG, "Bytes received: %i", len);
  ESP_LOGI(COMMSTAG, "Message id: %x\n", id);
  
  if(receiveFW==0){
    switch(id){

      case SNS_SEND_STATE:
        memcpy(&mssg, incomingData, sizeof(mssg));
        changeSensorsState(mssg.mssg);
        break;

      case VIS_SEND_STATE:
        memcpy(&mssg, incomingData, sizeof(mssg));
        changeVisionState(mssg.mssg);
        if(mssg.mssg==VIS_STATE_WAITING_VARS){
          itemBool.readPhoto = true;
        }
        break;

      case SNS_SEND_DATA:
        memcpy(&sensorsValues, incomingData, sizeof(sensorsValues));
        item.metal = sensorsValues.metal_val;
        item.pet = sensorsValues.pet_val;
        itemBool.readSensors = true;
        break;

      case VIS_SEND_FW:
        memcpy(&mssg, incomingData, sizeof(mssg));
        receiveFW = 1;
        if (!SPIFFS.begin(true)){
          ESP_LOGE(COMMSTAG, "Error mounting SPIFFS");
        }
        delay(100);
        SPIFFS.remove("/firmware.bin");
        delay(250);
        FWFile = SPIFFS.open("/firmware.bin", FILE_APPEND);
        break;

      case VIS_SEND_DB:
        memcpy(&mssg, incomingData, sizeof(mssg));
        receiveFW = 2;
        if (!SPIFFS.begin(true)){
          ESP_LOGE(COMMSTAG, "Error mounting SPIFFS");
        }
        delay(100);
        SPIFFS.remove("/bbdd.txt");
        delay(250);
        DBFile = SPIFFS.open("/bbdd.txt", FILE_APPEND);
        break;

    }

  }else if(receiveFW==1){
    if(len<5 && id==VIS_END_SEND_FW){
      memcpy(&mssg, incomingData, sizeof(mssg));
      receiveFW = 0;
      updated = true;
      ESP_LOGI(COMMSTAG, "firmware received!");
      FWFile.close();
      delay(10);
    }else{
      uint8_t bytesMssg[len];
      memcpy(&bytesMssg, incomingData, sizeof(bytesMssg));
      for(int i=0; i<len; i++){
        FWFile.write((byte)bytesMssg[i]);
      }
    }
    delay(1);

  }else if(receiveFW==2){
    if(len<5 && id==VIS_END_SEND_DB){
      memcpy(&mssg, incomingData, sizeof(mssg));
      receiveFW = 0;
      ESP_LOGI(COMMSTAG, "database received!");
      DBFile.close();
      delay(10);
    }else{
      uint8_t bytesMssg[len];
      memcpy(&bytesMssg, incomingData, sizeof(bytesMssg));
      for(int i=0; i<len; i++){
        DBFile.write((byte)bytesMssg[i]);
      }
    }

    delay(1);
  }


  // int metal;
  // memcpy(&metal, &myData.mssg, sizeof(metal));
  // Serial.printf("Content: %i\n", metal);
}

void changeSensorsState(uint8_t state){
  switch(state)
  {
    case SNS_STATE_TEST:
      sensorsState = SNS_STATE_TEST;
      ESP_LOGI(COMMSTAG, "Cambio de estado de sensores a: TEST\n");
      break;
    case SNS_STATE_OFF:
      sensorsState = SNS_STATE_OFF;
      ESP_LOGI(COMMSTAG, "Cambio de estado de sensores a: OFF\n");
      break;
    case SNS_STATE_INIT:
      sensorsState = SNS_STATE_INIT;
      ESP_LOGI(COMMSTAG, "Cambio de estado de sensores a: INIT\n");
      break;
    case SNS_STATE_IDLE:
      sensorsState = SNS_STATE_IDLE;
      ESP_LOGI(COMMSTAG, "Cambio de estado de sensores a: IDLE\n");
      break;
    case SNS_STATE_OPENFD:
      sensorsState = SNS_STATE_OPENFD;
      ESP_LOGI(COMMSTAG, "Cambio de estado de sensores a: OPENFD\n");
      break;
    case SNS_STATE_PSENSOR:
      sensorsState = SNS_STATE_PSENSOR;
      ESP_LOGI(COMMSTAG, "Cambio de estado de sensores a: PSENSOR\n");
      break;
    case SNS_STATE_CLOSEFD:
      sensorsState = SNS_STATE_CLOSEFD;
      ESP_LOGI(COMMSTAG, "Cambio de estado de sensores a: CLOSEFD\n");
      break;
    case SNS_STATE_WFREAD:
      sensorsState = SNS_STATE_WFREAD;
      ESP_LOGI(COMMSTAG, "Cambio de estado de sensores a: WFREAD\n");
      break;
    case SNS_STATE_READ:
      sensorsState = SNS_STATE_READ;
      ESP_LOGI(COMMSTAG, "Cambio de estado de sensores a: READ\n");
      break;
    case SNS_STATE_SENDINFO:
      sensorsState = SNS_STATE_SENDINFO;
      ESP_LOGI(COMMSTAG, "Cambio de estado de sensores a: SENDINFO\n");
      break;
    case SNS_STATE_OPENRD:
      sensorsState = SNS_STATE_OPENRD;
      ESP_LOGI(COMMSTAG, "Cambio de estado de sensores a: OPENRD\n");
      break;
    case SNS_STATE_CLOSERD:
      sensorsState = SNS_STATE_CLOSERD;
      ESP_LOGI(COMMSTAG, "Cambio de estado de sensores a: CLOSERD\n");
      break;
    case SNS_STATE_ERRORD:
      sensorsState = SNS_STATE_ERRORD;
      ESP_LOGI(COMMSTAG, "Cambio de estado de sensores a: ERRORD\n");
      break;
    default:
      sensorsState = SNS_STATE_OFF;
      ESP_LOGI(COMMSTAG, "Undefined state\n");
  }
}

void changeVisionState(uint8_t state){
  switch(state)
  {
    case VIS_STATE_INIT:
      visionState = VIS_STATE_INIT;
      ESP_LOGI(COMMSTAG, "Cambio de estado de vision a: INIT\n");
      break;
    case VIS_STATE_READY:
      visionState = VIS_STATE_READY;
      ESP_LOGI(COMMSTAG, "Cambio de estado de vision a: READY\n");
      break;
    case VIS_STATE_READY_LAST:
      visionState = VIS_STATE_READY_LAST;
      ESP_LOGI(COMMSTAG, "Cambio de estado de vision a: READY_LAST\n");
      break;
    case VIS_STATE_TRIGGER_RCVD:
      visionState = VIS_STATE_TRIGGER_RCVD;
      ESP_LOGI(COMMSTAG, "Cambio de estado de vision a: TRIGGER_RCVD\n");
      break;
    case VIS_STATE_WAITING_VARS:
      visionState = VIS_STATE_WAITING_VARS;
      ESP_LOGI(COMMSTAG, "Cambio de estado de vision a: WAITING_VARS\n");
      break;
    case VIS_STATE_MEMWRITE:
      visionState = VIS_STATE_MEMWRITE;
      ESP_LOGI(COMMSTAG, "Cambio de estado de vision a: MEMWRITE\n");
      break;
    case VIS_STATE_UPLOADING:
      visionState = VIS_STATE_UPLOADING;
      ESP_LOGI(COMMSTAG, "Cambio de estado de vision a: UPLOADING\n");
      break;
    case VIS_STATE_MEMFULL:
      visionState = VIS_STATE_MEMFULL;
      ESP_LOGI(COMMSTAG, "Cambio de estado de vision a: MEMFULL\n");
      break;
    case VIS_STATE_ERROR_INIT:
      visionState = VIS_STATE_ERROR_INIT;
      ESP_LOGI(COMMSTAG, "Cambio de estado de vision a: ERROR_INIT\n");
      break;
    case VIS_STATE_ERROR_CAMERA:
      visionState = VIS_STATE_ERROR_CAMERA;
      ESP_LOGI(COMMSTAG, "Cambio de estado de vision a: ERROR_CAMERA\n");
      break;
    case VIS_STATE_ERROR_MEM:
      visionState = VIS_STATE_ERROR_MEM;
      ESP_LOGI(COMMSTAG, "Cambio de estado de vision a: ERROR_MEM\n");
      break;
    case VIS_STATE_ERROR_UPLOAD:
      visionState = VIS_STATE_ERROR_UPLOAD;
      ESP_LOGI(COMMSTAG, "Cambio de estado de vision a: ERROR_UPLOAD\n");
      break;
    case VIS_STATE_UPLOADED:
      visionState = VIS_STATE_UPLOADED;
      ESP_LOGI(COMMSTAG, "Cambio de estado de vision a: UPLOADED\n");
      break;
    default:
      visionState = VIS_STATE_ERROR_INIT;
      ESP_LOGI(COMMSTAG, "Undefined state\n");
  }
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
  memcpy(peerInfo.peer_addr, sensorsAddress, 6); 
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    ESP_LOGE(COMMSTAG, "Failed to add sensors peer");
  }
  memcpy(peerInfo.peer_addr, visionAddress, 6); 
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    ESP_LOGE(COMMSTAG, "Failed to add vision peer");
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);

  return true;
}

bool Comms::sendMssg(uint8_t address[6], uint8_t id){

  Serial.println("sending mssg...");

  esp_err_t mssg1 = esp_now_send(address, (uint8_t *)&id, sizeof(id));

  if(mssg1 == ESP_OK){
    ESP_LOGD(COMMSTAG, "mssg sent with success");
  }else{
    ESP_LOGE(COMMSTAG, "Error sending mssg, retrying...");
    delay(100);
    return sendMssg(address, id);
  }

  return true;
}

bool Comms::sendItem(){

  esp_err_t mssg1 = esp_now_send(visionAddress, (uint8_t *)&item, sizeof(item));

  if(mssg1 == ESP_OK){
    ESP_LOGD(COMMSTAG, "mssg sent with success");
    itemBool.sentItem = true;
  }else{
    ESP_LOGE(COMMSTAG, "Error sending mssg, retrying...");
    delay(100);
    return sendItem();
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

bool Comms::sendDBVersion(char *v){

  version_struct dbVersion;
  dbVersion.id = MST_SEND_DB_VERSION;
  memcpy(dbVersion.version, v, sizeof(dbVersion));

  esp_err_t mssg1 = esp_now_send(visionAddress, (uint8_t *)&dbVersion, sizeof(dbVersion));

  if(mssg1 == ESP_OK){
    ESP_LOGD(COMMSTAG, "mssg sent with success");
  }else{
    ESP_LOGE(COMMSTAG, "Error sending mssg, retrying...");
    delay(100);
    return sendDBVersion(v);
  }

  return true;
}

void Comms::clearItem(){
  itemBool.readEAN = false;
  itemBool.readPhoto = false;
  itemBool.readSensors = false;
  itemBool.readVision = false;
  itemBool.sentItem = false;
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
    
    const char *sensorsMAC = json["sensors"];
    if(sensorsMAC!=NULL){
      for(int i=0; i<6; i++){
        char hexN[2];
        memcpy(hexN, &sensorsMAC[i*2], 2);
        sensorsAddress[i] = strtol(hexN, NULL, 16);
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
