#ifndef COMMS_H
#define COMMS_H

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include "i2c_mssgs_def.h"
#include "SPIFFS.h"
#include "ArduinoJson.h"

#define COMMSTAG "COMMS"

//DATA LENGTHS
#define TIMESTAMP_LENGTH 6
#define USER_LENGTH  4
#define EAN_LENGTH 20

const char firmwareVersion[] = "v3.03";

//WIFI
extern uint8_t sensorsAddress[6];
extern uint8_t visionAddress[6];
extern String success;
extern bool datasent;
extern uint8_t receiveFW;
extern bool updated;
extern File FWFile;
extern File DBFile;

typedef enum{
    SNS_STATE_TEST = 0x00,
    SNS_STATE_OFF,
    SNS_STATE_INIT,
    SNS_STATE_IDLE,
    SNS_STATE_OPENFD,
    SNS_STATE_PSENSOR,
    SNS_STATE_CLOSEFD,
    SNS_STATE_WFREAD,
    SNS_STATE_READ,
    SNS_STATE_SENDINFO,
    SNS_STATE_OPENRD,
    SNS_STATE_CLOSERD,
    SNS_STATE_ERRORD
}SensorsMachineStates;

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

typedef struct{
    uint8_t id = VIS_SEND_ITEM;
    uint8_t timestamp[TIMESTAMP_LENGTH];
    byte userID[USER_LENGTH];
    char EAN[EAN_LENGTH];
    int metal;
    float pet;
    unsigned int batteryLvl;
}item_struct;

typedef struct{
    uint8_t id;
    byte mssg;
}mssg_struct;

typedef struct{
    uint8_t id;
    float pet_val;
    int metal_val;
}sns_values;

typedef struct{
    uint8_t id = MST_SEND_VERSION;
    char version[6];
}version_struct;

typedef struct{
    char nextEAN[EAN_LENGTH];
    bool readEAN = false;
    bool readNextEAN = false;
    bool readSensors = false;
    bool readVision = false;
    bool readPhoto = false;
    bool sentItem = false;
}item_bools;

extern void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);
extern void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
extern void changeSensorsState(uint8_t state);
extern void changeVisionState(uint8_t state);

extern mssg_struct mssg;
extern sns_values sensorsValues;
extern item_struct item;
extern item_bools itemBool;
extern SensorsMachineStates sensorsState;
extern VisionMachineStates visionState;

class Comms{
    private:
        esp_now_peer_info_t peerInfo;
    public:
        Comms(){;}
        bool init();
        void clearItem();
        bool sendMssg(uint8_t address[6], uint8_t id);
        bool sendItem();
        bool sendVersion();
        bool sendDBVersion(char *v);
        bool getMACs();
};

#endif