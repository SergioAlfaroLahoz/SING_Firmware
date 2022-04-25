#include "comms.hpp"

//WIFI
uint8_t masterAddress[];
uint8_t sensorsAddress[];
String success;
bool datasent;

item_struct item;
version_struct masterFWV;
version_struct sensorsFWV;
version_struct bbdd;

// MAIN VARIABLES
viscomms_system_state_t prev_state;
viscomms_system_state_t main_state;
bool main_state_change = false;
esp_err_t main_loop_error;

// SYSTEM READINGS
bool main_timestamp_ok;
bool main_user_id_ok;
size_t main_ean_length;
bool main_ean_ok;
bool main_pet_ok;
bool main_metal_ok;
bool main_batteryLvl_ok;

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
  
  switch(id)
  {
    case VIS_SEND_ITEM:
      memcpy(&item, incomingData, sizeof(item));
      main_timestamp_ok = true;
      main_user_id_ok = true;
      main_ean_ok = true;
      main_pet_ok = true;
      main_metal_ok = true;
      main_batteryLvl_ok = true;
      break;

    case VIS_RUN_CAMERA:
      prev_state = main_state;
      main_state = VISCOMMS_STATE_TRIGGER_RCVD;
      main_state_change = true; 
      break;

    case VIS_RUN_UPLOAD:
      prev_state = main_state;
      main_state = VISCOMMS_STATE_UPLOADING;
      main_state_change = true;
      break;

    case MST_SEND_VERSION: 
      memcpy(&masterFWV, incomingData, sizeof(masterFWV));
      break;

    case MST_SEND_DB_VERSION:
      memcpy(&bbdd, incomingData, sizeof(bbdd));
      break;

    case SNS_SEND_VERSION:
      memcpy(&sensorsFWV, incomingData, sizeof(sensorsFWV));
      break;
  }

  // int metal;
  // memcpy(&metal, &myData.mssg, sizeof(metal));
  // Serial.printf("Content: %i\n", metal);
}

bool Comms::init(){
  if(!getMACs()){
    return false;
  }

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
  memcpy(peerInfo.peer_addr, sensorsAddress, 6); 
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

bool Comms::sendFW(uint8_t address[]){
  byte uploadFileMssg = VIS_SEND_FW;

  esp_err_t mssg1 = esp_now_send(address, (uint8_t *)&uploadFileMssg, sizeof(uploadFileMssg));
  if (mssg1 == ESP_OK) {
  Serial.println("Upload mssg sent with success");
  }
  else {
  Serial.println("Error sending upload mssg");
  }

  delay(15000); // Wait for sending

  // Initialize SPIFFS
  if (!SPIFFS.begin(true))
  {
    ESP_LOGE(COMMSTAG, "Error mounting SPIFFS");
    return ESP_FAIL;
  }

  File fileRead = SPIFFS.open("/firmware.bin", FILE_READ);

  if(!fileRead){
    ESP_LOGE(COMMSTAG, "Failed to open file for reading");
  }

  int fSize = fileRead.size();
  int TotalFSize = fSize;
  ESP_LOGI(COMMSTAG, "file size: %zd", TotalFSize);

  while(fSize>50){
    int percentageUploaded = ((float)((float)(float)TotalFSize-(float)fSize)/TotalFSize)*100;
    ESP_LOGI(COMMSTAG, "Percentage passed: %i%%", percentageUploaded);
    char bytesMssg[50];
    fileRead.readBytes((char*)&bytesMssg[0], 50);
    esp_now_send(address, (uint8_t *)&bytesMssg, sizeof(bytesMssg));
    int delTime = 150;
    while(!datasent){
     ESP_LOGE(COMMSTAG, "Error sending data, trying again...");
      delay(delTime);
      esp_now_send(address, (uint8_t *)&bytesMssg, sizeof(bytesMssg));
      delTime += 150;
    }
    fSize -= 50;
    delay(93); //150
  }

  char bytesMssgF[fSize];
  fileRead.readBytes((char*)&bytesMssgF[0], fSize);
  esp_now_send(address, (uint8_t *)&bytesMssgF, sizeof(bytesMssgF));
  int delTime = 150;
  while(!datasent){
    ESP_LOGE(COMMSTAG, "Error sending data, trying again...");
    delay(delTime);
    esp_now_send(address, (uint8_t *)&bytesMssgF, sizeof(bytesMssgF));
    delTime += 50;
  }

  fileRead.close();
  ESP_LOGI(COMMSTAG, "Done!");
  delay(50);
  SPIFFS.remove("/firmware.bin");

  uint8_t mssgSend = VIS_END_SEND_FW;
  esp_now_send(address, (uint8_t *)&mssgSend, sizeof(mssgSend));
}

bool Comms::sendDB(uint8_t address[]){
  byte uploadFileMssg = VIS_SEND_DB;

  esp_err_t mssg1 = esp_now_send(address, (uint8_t *)&uploadFileMssg, sizeof(uploadFileMssg));
  if (mssg1 == ESP_OK) {
  Serial.println("Upload mssg sent with success");
  }
  else {
  Serial.println("Error sending upload mssg");
  }

  delay(5000); // Wait for sending

  // Initialize SPIFFS
  if (!SPIFFS.begin(true))
  {
    ESP_LOGE(COMMSTAG, "Error mounting SPIFFS");
    return ESP_FAIL;
  }

  File fileRead = SPIFFS.open("/bbdd.txt", FILE_READ);

  if(!fileRead){
    ESP_LOGE(COMMSTAG, "Failed to open file for reading");
  }

  int fSize = fileRead.size();
  int TotalFSize = fSize;
  ESP_LOGI(COMMSTAG, "file size: %zd", TotalFSize);

  while(fSize>50){
    int percentageUploaded = ((float)((float)(float)TotalFSize-(float)fSize)/TotalFSize)*100;
    ESP_LOGI(COMMSTAG, "Percentage passed: %i%%", percentageUploaded);
    char bytesMssg[50];
    fileRead.readBytes((char*)&bytesMssg[0], 50);
    esp_now_send(address, (uint8_t *)&bytesMssg, sizeof(bytesMssg));
    int delTime = 150;
    while(!datasent){
     ESP_LOGE(COMMSTAG, "Error sending data, trying again...");
      delay(delTime);
      esp_now_send(address, (uint8_t *)&bytesMssg, sizeof(bytesMssg));
      delTime += 150;
    }
    fSize -= 50;
    delay(68); //150
  }

  char bytesMssgF[fSize];
  fileRead.readBytes((char*)&bytesMssgF[0], fSize);
  esp_now_send(address, (uint8_t *)&bytesMssgF, sizeof(bytesMssgF));
  int delTime = 150;
  while(!datasent){
    ESP_LOGE(COMMSTAG, "Error sending data, trying again...");
    delay(delTime);
    esp_now_send(address, (uint8_t *)&bytesMssgF, sizeof(bytesMssgF));
    delTime += 50;
  }

  fileRead.close();
  ESP_LOGI(COMMSTAG, "Done!");
  delay(50);
  SPIFFS.remove("/bbdd.txt");

  uint8_t mssgSend = VIS_END_SEND_DB;
  esp_now_send(address, (uint8_t *)&mssgSend, sizeof(mssgSend));
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

    const char *sensorsMAC = json["sensors"];
    if(sensorsMAC!=NULL){
      for(int i=0; i<6; i++){
        char hexN[2];
        memcpy(hexN, &sensorsMAC[i*2], 2);
        sensorsAddress[i] = strtol(hexN, NULL, 16);
      }
    }

    return true;
}