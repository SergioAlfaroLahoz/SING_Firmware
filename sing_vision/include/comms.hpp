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

//WIFI
extern uint8_t masterAddress[6];
extern uint8_t sensorsAddress[6];
extern String success;
extern bool datasent;

typedef enum{
    VISCOMMS_STATE_INIT,
    VISCOMMS_STATE_READY,
    VISCOMMS_STATE_READY_LAST,
    VISCOMMS_STATE_TRIGGER_RCVD,
    VISCOMMS_STATE_WAITING_VARS,
    VISCOMMS_STATE_MEMWRITE,
    VISCOMMS_STATE_UPLOADING,
    VISCOMMS_STATE_MEMFULL,
    VISCOMMS_STATE_ERROR_INIT,
    VISCOMMS_STATE_ERROR_CAMERA,
    VISCOMMS_STATE_ERROR_MEM,
    VISCOMMS_STATE_ERROR_UPLOAD,
    VISCOMMS_STATE_UPLOADED,
}viscomms_system_state_t;

typedef struct{
    uint8_t id = VIS_SEND_ITEM;
    uint8_t timestamp[TIMESTAMP_LENGTH];
    byte userID[USER_LENGTH];
    char EAN[EAN_LENGTH];
    int metal;
    float pet;
    unsigned int batteryLvl;
}item_struct;

typedef struct mssg_struct {
    uint8_t id;
    byte mssg;
}mssg_struct;

typedef struct{
    uint8_t id;
    char version[6];
}version_struct;

extern item_struct item;
extern version_struct masterFWV;
extern version_struct sensorsFWV;
extern version_struct bbdd;

// MAIN VARIABLES
extern viscomms_system_state_t prev_state;
extern viscomms_system_state_t main_state;
extern bool main_state_change;
extern esp_err_t main_loop_error;

// SYSTEM READINGS
extern bool main_timestamp_ok;
extern bool main_user_id_ok;
extern size_t main_ean_length;
extern bool main_ean_ok;
extern bool main_pet_ok;
extern bool main_metal_ok;
extern bool main_batteryLvl_ok;

// extern test_struct myData;
extern void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);
extern void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);

class Comms{
    private:
        esp_now_peer_info_t peerInfo;
    public:
        Comms(){;}
        bool init();
        bool sendSensors();
        bool sendMssg(uint8_t address[], uint8_t id, uint8_t mssg);
        bool sendFW(uint8_t address[]);
        bool sendDB(uint8_t address[]);
        char *getMasterFWV() {return masterFWV.version;}
        char *getSensorsFWV() {return sensorsFWV.version;}
        char *getDBV() {return bbdd.version;}
        bool getMACs();
};

#endif