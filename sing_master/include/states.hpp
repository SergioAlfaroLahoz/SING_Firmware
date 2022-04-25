#ifndef STATES_H
#define STATES_H

#include <Arduino.h>
#include "Update.h"

#define STATESTAG "MACHINE STATES"

#define WAKEUP_CLOCK_PIN_BITMASK 0x1000

enum MachineStates{
    TEST = 0x00,
    OFF,
    SLEEP,
    LOGIN,
    INIT,
    BARCODE,
    OPENFD,
    READ,
    SENDINFO,
    UPLOAD,
    ERROR
};

class States{
    private: 
        MachineStates status = OFF;
        MachineStates lastStatus = OFF;

    public:
        bool newState;
        States() {status=OFF;}
        void setStatus(MachineStates s);
        MachineStates getStatus(void) {return status;}

        void taskSleep();
        void taskTest();
        void taskBarcode();
        void taskOpenFD();
        void taskRead();
        void taskSendInfo();
        void taskUpload();
        void taskError();
};

#endif