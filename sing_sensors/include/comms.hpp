#ifndef COMMS_H
#define COMMS_H

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include "states.hpp"
#include "i2c_mssgs_def.h"
#include "SPIFFS.h"
#include "ArduinoJson.h"

#define COMMSTAG "COMMS"

//DATA LENGTHS
#define TIMESTAMP_LENGTH 6
#define USER_LENGTH  4
#define EAN_LENGTH 20

const char firmwareVersion[] = "v3.01";

//WIFI
extern uint8_t masterAddress[6];
extern uint8_t visionAddress[6];
extern String success;
extern bool datasent;
extern bool receiveFW;
extern bool updated;
extern File FWFile;

typedef enum
{
    VIS_STATE_INIT,
    VIS_STATE_READY,
    VIS_STATE_READY_LAST,
    VIS_STATE_TRIGGER_RCVD,
    VIS_STATE_WAITING_VARS,
    VIS_STATE_MEMWRITE,
    VIS_STATE_UPLOADING,
    VIS_STATE_MEMFULL,
    VIS_STATE_ERROR_INIT,
    VIS_STATE_ERROR_CAMERA,
    VIS_STATE_ERROR_MEM,
    VIS_STATE_ERROR_UPLOAD,
    VIS_STATE_UPLOADED,
}VisionMachineStates;

typedef struct mssg_struct {
    uint8_t id;
    byte mssg;
}mssg_struct;

typedef struct{
    uint8_t id = SNS_SEND_DATA;
    float pet_val;
    int metal_val;
}sns_values;

typedef struct{
    uint8_t id = SNS_SEND_VERSION;
    char version[6];
}version_struct;

// extern test_struct myData;
extern void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);
extern void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);

extern States state;
extern sns_values sensorsRead;

class Comms{
    private:
        esp_now_peer_info_t peerInfo;
    public:
        Comms(){;}
        bool init();
        bool sendSensors();
        bool sendMssg(uint8_t address[], uint8_t id, uint8_t mssg);
        bool sendVersion();
        bool getMACs();
};

#endif