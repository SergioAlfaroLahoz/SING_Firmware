#ifndef STATES_H
#define STATES_H

#include <Arduino.h>
#include "Update.h"

#define STATESTAG "MACHINE STATES"

enum MachineStates{
    TEST = 0x00,
    OFF,
    INIT,
    IDLE,
    OPENFD,
    PSENSOR,
    CLOSEFD,
    WFREAD,
    READ,
    SENDINFO,
    OPENRD,
    CLOSERD,
    ERRORD,
    UPLOAD,
};

class States{
    private: 
        MachineStates status = OFF;
        MachineStates lastStatus = OFF;
        long pSensorTime;

    public:
        bool newState;
        States() {status=OFF;}
        void setStatus(MachineStates s);
        MachineStates getStatus(void) {return status;}

        void taskInit();
        void taskTest();
        void taskIDLE();
        void taskOpenFD();
        void taskPSensor();
        void taskCloseFD();
        void taskWFRead();
        void taskRead();
        void taskOpenRD();
        void taskCloseRD();
        void taskSendInfo();
        void taskUpload();
        void taskErrorD();
};

#endif