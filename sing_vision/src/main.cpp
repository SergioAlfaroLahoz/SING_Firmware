#include "sing_camera.h"
#include "sing_server_comms.h"
// #include "sing_file_tools.h" // CIRCULAR REFERENCE ERROR WITH sing_server_comms.h
#include "i2c_mssgs_def.h"
#include "comms.hpp"

#include "Update.h"

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

#include "esp32-hal-log.h"
#include "esp_err.h"

static const char *TAG = "Vision&Comms";

char firmwareVersion[] = "d003v3.02";

#define FLASH 4
#define FLASH_DELAY_MS 100

// iteration limits
#define MAX_CAM_TRIALS 3
#define MAX_WRITING_TRIALS 3
#define MAX_HTTP_POST_TRIALS 3
#define MAX_HTTP_CONNECTION_TRIALS 3
#define MAX_MEM_READINGS 3

// DATA LENGTHS
#define MAX_EAN_LENGTH 20
#define USER_ID_LENGTH 4
#define TIMESTAMP_LENGTH 6

#define STATE_PRINTINFO_TIME_MS 10000

// #define TESTING // Uncomment to activate Test code (state changes automated)
// #define TESTING_DELAY 500

// #define NO_COMMS // Uncomment to run without 3G COMMS

// #define NO_CAMERA // Uncomment to run without camera

// #define FORMAT_MEM // Uncomment to format spiffs memory during setup

Comms comms;

camera_fb_t *main_image;
bool main_image_ok;

// TIMING
unsigned long state_ini_time_ms = 0;
unsigned long state_end_time_ms = 0;
unsigned long last_print_time_ms = 0;

bool uploadedFlag = false;

// Trial counters
size_t n_cam_acquisition_trials = 0;
size_t n_mem_writing_trials = 0;
size_t n_http_connection_trials = 0;
size_t n_http_post_trials = 0;
size_t n_mem_reading_trials = 0;

esp_err_t triggerCamera()
{
    main_image = NULL;
    esp_err_t err = ESP_OK;

    digitalWrite(FLASH, HIGH);
    delay(FLASH_DELAY_MS);
#ifndef NO_CAMERA
    main_image = esp_camera_fb_get();
#endif
    // delay(FLASH_DELAY_MS);
    delay(50);
    digitalWrite(FLASH, LOW);

#ifndef NO_CAMERA
    if (!main_image)
    {
        ESP_LOGE(TAG, "Camera capture failed");
        esp_camera_fb_return(main_image);
        err = ESP_FAIL;
    }
    else
    {
        ESP_LOGI(TAG, "new image [%dx%d] (buffer length: %d)", main_image->height, main_image->width, main_image->len);
    }
#endif

    return err;
}

esp_err_t releaseCameraBuffer()
{
    esp_err_t err = ESP_OK;
#ifndef NO_CAMERA
    esp_camera_fb_return(main_image);
    if (main_image)
    {
        ESP_LOGE(TAG, "Camera release failed!");
        err = ESP_FAIL;
    }
    else
    {
        ESP_LOGI(TAG, "Image buffer released");
    }
#endif
    return err;
}

void resetReading()
{
    // reading->image = NULL;
    releaseCameraBuffer();
    main_image_ok = false;

    for (uint8_t i = 0; i < TIMESTAMP_LENGTH; i++)
    {
        item.timestamp[i] = 0;
    }
    main_timestamp_ok = false;

    for (uint8_t i = 0; i < USER_ID_LENGTH; i++)
    {
        item.userID[i] = 0x00;
    }
    main_user_id_ok = false;

    for (uint8_t i = 0; i < MAX_EAN_LENGTH; i++)
    {
        item.EAN[i] = ' ';
    }
    main_ean_length = 0;
    main_ean_ok = false;

    item.pet = 0;
    main_pet_ok = false;

    item.metal = 0;
    main_metal_ok = false;

    item.batteryLvl = 0;
    main_batteryLvl_ok = false;

    ESP_LOGI(TAG, "System reading reset!");
}

void createRandomReading()
{

    if (!main_image_ok)
    {
        ESP_LOGE(TAG, "Image not captured by reading");
    }

    // esp_fill_random(&main_timestamp, TIMESTAMP_LENGTH);
    item.timestamp[0] = (uint8_t)random(0, 99);
    item.timestamp[1] = (uint8_t)random(0, 12);
    item.timestamp[2] = (uint8_t)random(0, 31);
    item.timestamp[3] = (uint8_t)random(0, 24);
    item.timestamp[4] = (uint8_t)random(0, 60);
    item.timestamp[5] = (uint8_t)random(0, 60);
    main_timestamp_ok = true;
    // ESP_LOGI(TAG, "Random timestamp created");

    for (size_t i = 0; i < MAX_EAN_LENGTH; i++)
    {
        item.EAN[i] = (uint8_t)random(0, 16);
    }
    main_ean_length = MAX_EAN_LENGTH;
    main_ean_ok = true;
    // ESP_LOGI(TAG, "Random ean created");

    for (size_t i = 0; i < USER_ID_LENGTH; i++)
    {
        item.userID[i] = (uint8_t)random(0, 16);
    }
    main_user_id_ok = true;
    // ESP_LOGI(TAG, "Random user_id created");

    item.pet = (float)random(0, 100);
    main_pet_ok = true;
    // ESP_LOGI(TAG, "Random pet value created");

    item.metal = (int)random(100, 200);
    main_metal_ok = true;
    // ESP_LOGI(TAG, "Random metal value created");

    item.batteryLvl = (int)random(100, 1000);
    main_batteryLvl_ok = true;
    // ESP_LOGI(TAG, "Random metal value created");
    
}

void printReading()
{
    if (main_image_ok)
    {
        ESP_LOGD(TAG, "System Reading image:        [%dx%d] (buffer length: %d)",
                 main_image->height, main_image->width, main_image->len);
    }
    else
    {
        ESP_LOGE(TAG, "System Reading missing image");
    }

    if (main_timestamp_ok)
    {
        ESP_LOGD(TAG, "System Reading timestamp:    %02d-%02d-%02d %02d:%02d:%02d",
                 item.timestamp[0],
                 item.timestamp[1],
                 item.timestamp[2],
                 item.timestamp[3],
                 item.timestamp[4],
                 item.timestamp[5]);
    }
    else
    {
        ESP_LOGE(TAG, "System Reading missing timestamp");
    }

    if (main_user_id_ok)
    {
        ESP_LOGD(TAG, "System Reading user id:      0x%02X 0x%02X 0x%02X 0x%02X",
                 item.userID[0],
                 item.userID[1],
                 item.userID[2],
                 item.userID[3]);
    }
    else
    {
        ESP_LOGE(TAG, "System Reading missing user id");
    }

    if (main_ean_ok)
    {
        ESP_LOGD(TAG, "System Reading ean:          0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X",
                 item.EAN[0],
                 item.EAN[1],
                 item.EAN[2],
                 item.EAN[3],
                 item.EAN[4],
                 item.EAN[5],
                 item.EAN[6],
                 item.EAN[7],
                 item.EAN[8],
                 item.EAN[9],
                 item.EAN[10],
                 item.EAN[11],
                 item.EAN[12],
                 item.EAN[13],
                 item.EAN[14],
                 item.EAN[15],
                 item.EAN[16],
                 item.EAN[17],
                 item.EAN[18],
                 item.EAN[19]);
    }
    else
    {
        ESP_LOGE(TAG, "System Reading missing ean");
    }

    if (main_pet_ok)
    {
        ESP_LOGD(TAG, "System Reading pet value:    %.4f", item.pet);
    }
    else
    {
        ESP_LOGE(TAG, "System Reading missing pet value");
    }

    if (main_metal_ok)
    {
        ESP_LOGD(TAG, "System Reading metal value:  %d",item.metal);
    }
    else
    {
        ESP_LOGE(TAG, "System Reading missing metal value");
    }
    if (main_batteryLvl_ok)
    {
        ESP_LOGD(TAG, "System Reading battery level value:  %u",item.batteryLvl);
    }
    else
    {
        ESP_LOGE(TAG, "System Reading missing battery level");
    }
}

const char *getStateStr(viscomms_system_state_t state)
{
    switch (state)
    {
    case VISCOMMS_STATE_INIT:
        return "VISCOMMS_STATE_INIT";
    case VISCOMMS_STATE_READY:
        return "VISCOMMS_STATE_READY";
    case VISCOMMS_STATE_READY_LAST:
        return "VISCOMMS_STATE_READY_LAST";
    case VISCOMMS_STATE_TRIGGER_RCVD:
        return "VISCOMMS_STATE_TRIGGER_RCVD";
    case VISCOMMS_STATE_WAITING_VARS:
        return "VISCOMMS_STATE_WAITING_VARS";
    case VISCOMMS_STATE_MEMWRITE:
        return "VISCOMMS_STATE_MEMWRITE";
    case VISCOMMS_STATE_UPLOADING:
        return "VISCOMMS_STATE_UPLOADING";
    case VISCOMMS_STATE_MEMFULL:
        return "VISCOMMS_STATE_MEMFULL";
    case VISCOMMS_STATE_ERROR_INIT:
        return "VISCOMMS_STATE_ERROR_INIT";
    case VISCOMMS_STATE_ERROR_CAMERA:
        return "VISCOMMS_STATE_ERROR_CAMERA";
    case VISCOMMS_STATE_ERROR_MEM:
        return "VISCOMMS_STATE_ERROR_MEM";
    case VISCOMMS_STATE_ERROR_UPLOAD:
        return "VISCOMMS_STATE_ERROR_UPLOAD";
    case VISCOMMS_STATE_UPLOADED:
        return "VISCOMMS_STATE_ERROR_UPLOAD";
    default:
        return "UNKNOWN";
    }
}

void timedPrintInfo()
{
    if ((millis() - last_print_time_ms) > STATE_PRINTINFO_TIME_MS)
    {
        ESP_LOGI(TAG, "%s active for % 10lu ms (previous state: %s)", getStateStr(main_state), millis() - state_ini_time_ms, getStateStr(prev_state));
        switch (main_state)
        {
        case VISCOMMS_STATE_WAITING_VARS:
            if (!main_image_ok)
                ESP_LOGD(TAG, "IMAGE not acquired");
            if (!main_ean_ok)
                ESP_LOGD(TAG, "EAN not received");
            if (!main_user_id_ok)
                ESP_LOGD(TAG, "USER_ID not received");
            if (!main_metal_ok)
                ESP_LOGD(TAG, "METAL not received");
            if (!main_pet_ok)
                ESP_LOGD(TAG, "PET not received");
            if (!main_batteryLvl_ok)
                ESP_LOGD(TAG, "Battery level not received");
            break;

        default:
            break;
        }
        last_print_time_ms = millis();
    }
}

void newStateRun()
{
    state_end_time_ms = millis();
    ESP_LOGD(TAG, "%s -> %s [%d ms]", getStateStr(prev_state), getStateStr(main_state), (state_end_time_ms - state_ini_time_ms));
    ESP_LOGI(TAG, "Current state: %s", getStateStr(main_state));
    state_ini_time_ms = millis();
    last_print_time_ms = state_ini_time_ms;
    main_state_change = false;
    comms.sendMssg(masterAddress, VIS_SEND_STATE, main_state);
}

void checkForReady()
{
    prev_state = main_state;
    if (getFreeBytes() < MIN_FREE_BYTES)
    {
        if (getFreeMemorySlots() > 0)
        {
            ESP_LOGE(TAG, "Free slots [%d bytes] but not enough free memory [%d free slots]", getFreeBytes(), getFreeMemorySlots());
            main_state = VISCOMMS_STATE_ERROR_MEM;
        }
        else
        {
            main_state = VISCOMMS_STATE_MEMFULL;
        }
    }
    else if (getFreeMemorySlots() > 1)
    {
        main_state = VISCOMMS_STATE_READY;
    }
    else if (getFreeMemorySlots() == 1)
    {
        main_state = VISCOMMS_STATE_READY_LAST;
    }
    else
    {
        main_state = VISCOMMS_STATE_MEMFULL;
    }
    if (main_state != prev_state)
    {
        main_state_change = true;
    }
}

esp_err_t init_camera()
{
    main_loop_error = connect_camera();
    if (main_loop_error == ESP_FAIL)
    {
        ESP_LOGE(TAG, "Camera connection failed");
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t init_mem()
{
#ifdef FORMAT_MEM
    main_loop_error = initMemory(true);
#else
    main_loop_error = initMemory(false);
#endif
    if (main_loop_error == ESP_FAIL)
    {
        ESP_LOGE(TAG, "Memory initialization failed");
        return ESP_FAIL;
    }
    return ESP_OK;
}

void setup()
{
    Serial.begin(115200);

    comms.init();

    // Enable flash
    pinMode(FLASH, OUTPUT);
    digitalWrite(FLASH, LOW);

    esp_log_level_set("", ESP_LOG_DEBUG);

    ESP_LOGI(TAG, "Firmware version: %s", firmwareVersion);

    state_ini_time_ms = millis();
    prev_state = VISCOMMS_STATE_INIT;
    main_state = VISCOMMS_STATE_INIT;
    main_state_change = true;

    // Set initial main_reading
    main_image = NULL;
    resetReading();
}

void loop()
{
    switch (main_state)
    {
    ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////
    case VISCOMMS_STATE_INIT:
        if (main_state_change)
        {
            newStateRun();
        }

#ifndef NO_CAMERA
        // Connect camera
        main_loop_error = init_camera();
#endif
        // Initialize memory
        init_mem();
        // Go to ready
        prev_state = main_state;
        checkForReady();
        break;
    case VISCOMMS_STATE_READY_LAST:
    case VISCOMMS_STATE_READY:
        if (main_state_change)
        {
            newStateRun();
        }
#ifdef TESTING
        {
            delay(TESTING_DELAY);
            ESP_LOGD(TAG, "TESTING - State changed automatically > VISCOMMS_STATE_TRIGGER_RCVD");
            prev_state = main_state;
            main_state = VISCOMMS_STATE_TRIGGER_RCVD;
            main_state_change = true;
        }
#endif
        break;
    ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////
    case VISCOMMS_STATE_TRIGGER_RCVD:
        if (main_state_change)
        {
            newStateRun();
            n_cam_acquisition_trials = 0;
        }
        // Capture new image
        main_loop_error = triggerCamera();
        if (main_loop_error == ESP_OK)
        {
            // Update main_reading
            main_image_ok = true;
            // Change main_state
            prev_state = main_state;
            main_state = VISCOMMS_STATE_WAITING_VARS;
            main_state_change = true;
        }
        else
        {
            n_cam_acquisition_trials++;
#ifndef NO_CAMERA
            releaseCameraBuffer();
            disconnect_camera();
            connect_camera();
#endif
            if (n_cam_acquisition_trials > MAX_CAM_TRIALS)
            {
                ESP_LOGE(TAG, "Unable to capture image > VISCOMMS_STATE_ERROR_CAMERA");
                prev_state = main_state;
                main_state = VISCOMMS_STATE_ERROR_CAMERA;
                main_state_change = true;
            }
        }
        break;
    ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////
    case VISCOMMS_STATE_WAITING_VARS:
        if (main_state_change)
        {
            newStateRun();
        }
#ifdef TESTING
        {
            ESP_LOGI(TAG, "TESTING - Create random system reading");
            createRandomReading();
        }
#endif
        if (main_image_ok &
            main_timestamp_ok &
            main_ean_ok &
            main_user_id_ok &
            main_metal_ok &
            main_pet_ok &
            main_batteryLvl_ok)
        {
            // Change main state
            ESP_LOGI(TAG, "All data received! > VISCOMMS_STATE_MEMWRITE");
            printReading();
            prev_state = main_state;
            main_state = VISCOMMS_STATE_MEMWRITE;
            main_state_change = true;
        }
        break;
    ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////
    case VISCOMMS_STATE_MEMWRITE:
        if (main_state_change)
        {
            newStateRun();
            n_mem_writing_trials = 0;
        }
        {
            size_t reading_id = getNextFreeReadingId();
            if (reading_id >= 0)
            {
                main_loop_error = writeReading(
                    reading_id,
                    main_image,
                    item.timestamp,
                    TIMESTAMP_LENGTH,
                    item.userID,
                    USER_ID_LENGTH,
                    item.EAN,
                    MAX_EAN_LENGTH,
                    &item.pet,
                    &item.metal,
                    &item.batteryLvl);
                if (main_loop_error == ESP_OK)
                {
                    resetReading();
                    // Change main_state
                    checkForReady();
                }
                else
                {
                    ESP_LOGE(TAG, "Error saving data on slot %d", reading_id);
                    deleteReading(reading_id);
                    n_mem_writing_trials++;
                }
            }
            else
            {
                ESP_LOGE(TAG, "No free slot found in memory");
                main_loop_error = ESP_FAIL;
                n_mem_writing_trials++;
                // TODO What to do in this case?
            }
            if (n_mem_writing_trials > MAX_WRITING_TRIALS)
            {
                ESP_LOGI(TAG, "Unable to write data to memory > VISCOMMS_STATE_ERROR_MEM");
                prev_state = main_state;
                main_state = VISCOMMS_STATE_ERROR_MEM;
                main_state_change = true;
            }
        }
        break;
    ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////
    case VISCOMMS_STATE_MEMFULL:
        if (main_state_change)
        {
            newStateRun();
        }
#ifdef TESTING
        else
        {
            ESP_LOGD(TAG, "TESTING - State changed automatically > VISCOMMS_STATE_UPLOADING");
            prev_state = main_state;
            main_state = VISCOMMS_STATE_UPLOADING;
            main_state_change = true;
        }
#else
        // Change main_state
        // checkForReady();
#endif
        break;
    ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////
    case VISCOMMS_STATE_UPLOADING:
        if (main_state_change)
        {
            newStateRun();
            n_http_connection_trials = 0;
            n_http_post_trials = 0;
            n_mem_reading_trials = 0;
        }
        uploadedFlag = false;

        if (getUsedMemorySlots() > 0) //If there's data in memory...
        {
            ESP_LOGI(TAG, "%d readings in memory -> Keep uploading", getUsedMemorySlots());
#ifdef NO_COMMS
            main_loop_error = ESP_OK;
#else
            main_loop_error = httpIsConnected();
#endif
            if (main_loop_error == ESP_FAIL)
            {
                ESP_LOGI(TAG, "Trying to connect to HTTP (trial %d)", n_http_connection_trials);
#ifdef NO_COMMS
                main_loop_error = ESP_OK;
#else
                main_loop_error = httpConnect();
                n_http_connection_trials++;
#endif
            }
            else
            {
                ESP_LOGI(TAG, "System connected to HTTP");

                // Send Data
                size_t reading_id = getNextUsedReadingId();
                img_struct_t image_read;
                data_struct_t data_read;
                main_loop_error = getSavedImage(reading_id, &image_read);
                if (main_loop_error == ESP_FAIL)
                {
                    ESP_LOGE(TAG, "Error loading saved image");
                    n_mem_reading_trials++;
                }
                else
                {
                    main_loop_error = getSavedData(reading_id, &data_read);
                    if (main_loop_error == ESP_FAIL)
                    {
                        ESP_LOGE(TAG, "Error loading saved data");
                        n_mem_reading_trials++;
                    }
                    else
                    {

                        ESP_LOGI(TAG, "Data read from memory");
                        ESP_LOGI(TAG, "Sending reading %d to server", reading_id);
#ifdef NO_COMMS
                        main_loop_error = ESP_OK;
#else
                        main_loop_error = httpPOSTcomplete(&image_read, &data_read);

#endif
                        if (main_loop_error == ESP_OK)
                        {
                            deleteReading(reading_id);
                        }
                        else
                        {
                            ESP_LOGE(TAG, "HTTP POST error (trial %d)", n_http_post_trials);
                            // n_http_post_trials++;
                            deleteReading(reading_id);
                            main_loop_error = ESP_OK;
                        }
                    }
                }
                // FREE MEMORY!!!
                releaseImage(&image_read);
            }
        }
        else //Once data is uploaded...
        {
#ifdef NO_COMMS
            main_loop_error = ESP_OK;
#else
            main_loop_error = httpIsConnected();
#endif
            if (main_loop_error == ESP_FAIL)
            {
                ESP_LOGI(TAG, "Trying to connect to HTTP (trial %d)", n_http_connection_trials);
#ifdef NO_COMMS
                main_loop_error = ESP_OK;
#else
                main_loop_error = httpConnect();
                n_http_connection_trials++;
#endif
            }
            else
            {
                ESP_LOGI(TAG, "System connected to HTTP");
                // Check for Firmwares updates
                char MFWV[6];
                char SFWV[6];
                char DBV[6];
                memcpy(MFWV, comms.getMasterFWV(), sizeof(MFWV));
                memcpy(SFWV, comms.getSensorsFWV(), sizeof(SFWV));
                memcpy(DBV, comms.getDBV(), sizeof(DBV));
                updateFWs fws;
                availableFirmwares(&fws, MFWV, DBV, SFWV, firmwareVersion);
                ESP_LOGI(TAG, "master: %c", fws.masterFW);
                ESP_LOGI(TAG, "sensors: %c", fws.sensorsFW);
                ESP_LOGI(TAG, "vision: %c", fws.visionFW);
                ESP_LOGI(TAG, "bbdd: %c", fws.bbdd);
                if((char)fws.masterFW=='1'){
                    ESP_LOGI(TAG, "master Firmware update available!");
                    char masterTag = 'm';
                    updateFirmware(masterTag);
                    comms.sendFW(masterAddress);
                }
                if((char)fws.sensorsFW=='1'){
                    ESP_LOGI(TAG, "sensors Firmware update available!");
                    char sensorsTag = 's';
                    updateFirmware(sensorsTag);
                    comms.sendFW(sensorsAddress);
                }
                if((char)fws.bbdd=='1'){
                    ESP_LOGI(TAG, "data base update available!");
                    updateDB();
                    comms.sendDB(masterAddress);
                }
                if((char)fws.visionFW=='1'){
                    ESP_LOGI(TAG, "vision Firmware update available!");
                    char visionTag = 'v';
                    updateFirmware(visionTag);

                    uploadedFlag = true;
                    prev_state = main_state;
                    main_state = VISCOMMS_STATE_UPLOADED;
                    main_state_change = true;
                }
            }

            // Disconnect!
#ifndef NO_COMMS
            if (httpIsConnected() == ESP_OK)
            {
                esp_err_t err = httpDisconnect();
                if (err == ESP_FAIL)
                {
                    ESP_LOGE(TAG, "Error disconnecting from HTTP server");
                }
            }
#endif
            ESP_LOGI(TAG, "%d slots free!", getUsedMemorySlots());
            if(!uploadedFlag){
                checkForReady();
            }
        }
        if (n_http_connection_trials > MAX_HTTP_CONNECTION_TRIALS)
        {
            ESP_LOGE(TAG, "%d / %d attemps to connect to HTTP", n_http_connection_trials, MAX_HTTP_CONNECTION_TRIALS);
            ESP_LOGI(TAG, "Unable to connect to HTTP > VISCOMMS_STATE_ERROR_UPLOAD");
            prev_state = main_state;
            main_state = VISCOMMS_STATE_ERROR_UPLOAD;
            main_state_change = true;
        }
        if (n_http_post_trials > MAX_HTTP_POST_TRIALS)
        {
            ESP_LOGE(TAG, "%d / %d attemps to POST message to server", n_http_post_trials, MAX_HTTP_POST_TRIALS);
            ESP_LOGI(TAG, "Unable to connect to HTTP > VISCOMMS_STATE_ERROR_UPLOAD");
            prev_state = main_state;
            main_state = VISCOMMS_STATE_ERROR_UPLOAD;
            main_state_change = true;
        }
        if (n_mem_reading_trials > MAX_MEM_READINGS)
        {
            ESP_LOGE(TAG, "%d / %d attemps to read memory", n_mem_reading_trials, MAX_MEM_READINGS);
            ESP_LOGI(TAG, "Unable to connect to HTTP > VISCOMMS_STATE_ERROR_MEM");
            prev_state = main_state;
            main_state = VISCOMMS_STATE_ERROR_MEM;
            main_state_change = true;
        }
        break;
    ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////
    case VISCOMMS_STATE_UPLOADED:
    {
        if (main_state_change){
            newStateRun();
        }

        ESP_LOGI(TAG, "Starting update..");

        File file = SPIFFS.open("/firmware.bin");
        if(!file){
            ESP_LOGI(TAG, "Failed to open file for reading");
            return;
        }

        size_t fileSize = file.size();
        if(!Update.begin(fileSize)){                       
            ESP_LOGI(TAG, "Cannot do the update");
            return;
        };

        Update.writeStream(file);                
        if(Update.end()){                        
            ESP_LOGI(TAG, "Successful update");  
        }else {                      
            ESP_LOGI(TAG, "Error Occurred: %s", String(Update.getError()));
            return;
        }     

        file.close();               
        ESP_LOGI(TAG, "Reset in 2 seconds...");
        delay(2000);   

        SPIFFS.remove("/firmware.bin");
        delay(250);   

        ESP.restart();
        // comms.sendMssg(masterAddress, VIS_SEND_STATE, VISCOMMS_STATE_UPLOADED);
        break;
    }
    ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////
    case VISCOMMS_STATE_ERROR_INIT:
    case VISCOMMS_STATE_ERROR_CAMERA:
    case VISCOMMS_STATE_ERROR_UPLOAD:
    case VISCOMMS_STATE_ERROR_MEM:
        if (main_state_change)
        {
            newStateRun();
        }
        break;
    }
    ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////

    timedPrintInfo();
    // delay(100);
}