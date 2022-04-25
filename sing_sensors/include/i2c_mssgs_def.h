#ifndef I2C_MSSGS_DEF_H 
#define I2C_MSSGS_DEF_H 

#define CHECKBIT(byte_var, int_pos) ((byte_var) & (1 << (int_pos)))
#define SETBIT(byte_var, int_pos) ((byte_var) |= (1 << (int_pos)))

typedef enum {
    // VISION MSSGS ID
    VIS_GET_STATE =         0x10, // Ask VISION for its STATE
    VIS_GET_CAMERA_OK =     0x12, // Ask VISION if camera acquisition was OK (and it finished)
    VIS_RUN_CAMERA =        0x18, // Tell VISION to acquire a new image
    VIS_RUN_UPLOAD =        0x19, // Tell VISION to upload the stored data
    VIS_SEND_TIME =         0x1A, // Send VISION variable TIMESTAMP (???)
    VIS_SEND_USER =         0x1B, // Send VISION variable USER (byte[4])
    VIS_SEND_EAN =          0x1C, // Send VISION variable EAN (char[20])
    VIS_SEND_METAL =        0x1D, // Send VISION variable METAL (int)
    VIS_SEND_PET =          0x1E, // Send VISION variable PET (float)
    VIS_GET_READY =         0x11, // Tell VISION to get READY for acquisition
    // SENSORS MSSGS ID
    SNS_GET_STATE =         0x20, // Ask SENSORS for its STATE
    SNS_RUN_READING =       0x21, // Tell SENSORS to run the sensors reading
    SNS_OPEN_FRONT =        0x22, // Tell SENSORS to open the front door
    SNS_CLOSE_FRONT =       0x23, // Tell SENSORS to open the front door
    SNS_GET_FRONT_OPEN =    0x24, // Ask SENSORS if front door is open (OK) or closed (NOK)
    SNS_GET_METAL =         0x25, // Ask SENSORS for variable METAL
    SNS_GET_PET =           0x26, // Ask SENSORS for variable PET
    SNS_EVACUATE =          0x27, //Evacuate item
    // MASTER MSSGS ID
    MST_SEND_VERSION =      0x50,
    MST_SEND_DB_VERSION =   0x51,
} master_mssgs_id;

typedef enum {
    VIS_SEND_STATE =        0x32,
    VIS_SEND_FW =           0x33,
    VIS_END_SEND_FW =       0x34,
} vision_mssgs_id;

typedef enum {
    // RUN MESSAGES OK/NOK RESPONSE
    SNS_OK =                0x40, // Response to message: received and processed
    SNS_NOK =               0x41, // Response to message: received but not processed
    SNS_SEND_DATA =         0X42,
    SNS_SEND_STATE =        0x43,
    SNS_SEND_VERSION =      0x44,
} sensors_mssgs_id;

#endif