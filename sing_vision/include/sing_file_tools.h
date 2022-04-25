
#include "esp32-hal-log.h"
#include "esp_camera.h"
#include "esp_err.h"
#include "SPIFFS.h"

#define MAX_DATA_ARRAY_LENGTH 20
#define MIN_FREE_BYTES 10000

#define MAX_DEVICEID_LENGTH 10
const char deviceID[] = "d003v3.01";


typedef struct
{
    char deviceID[MAX_DEVICEID_LENGTH];
    uint8_t timestamp[MAX_DATA_ARRAY_LENGTH];
    size_t timestamp_len;
    byte user_id[MAX_DATA_ARRAY_LENGTH];
    size_t user_id_len;
    char ean[MAX_DATA_ARRAY_LENGTH];
    size_t ean_len;
    float pet_val;
    int metal_val;
    unsigned int battery_val;
} data_struct_t;

typedef struct
{
    byte *buff;
    size_t len;
} img_struct_t;

typedef struct
{
    uint8_t masterFW;
    uint8_t sensorsFW;
    uint8_t visionFW;
    uint8_t bbdd;
} updateFWs;

void printDataStruct(data_struct_t *data);

size_t getFreeBytes();

void checkMemory(bool useSlotArray, bool *usedSlotsArray);

esp_err_t initMemory(bool formatMem = false);

size_t getNextFreeReadingId();

size_t getNextUsedReadingId();

size_t getFreeMemorySlots();

size_t getUsedMemorySlots();

esp_err_t saveUsedReadingIdsData();

esp_err_t writeReading(size_t reading_id, camera_fb_t *image, uint8_t *timestamp, size_t timestamp_len, byte *user_id, size_t user_id_len, char *ean, uint8_t ean_len, float *pet_val, int *metal_val, unsigned int *battery_val);

esp_err_t getReading(size_t reading_id, byte *image_buff, size_t image_len, uint8_t *timestamp, size_t timestamp_len, byte *user_id, size_t user_id_len, char *ean, size_t ean_len, float *pet_val, int *metal_val, unsigned int *battery_val);

esp_err_t getSavedImage(size_t reading_id, img_struct_t *read_img);

esp_err_t getSavedData(size_t reading_id, data_struct_t *read_data);

esp_err_t releaseImage(img_struct_t *img);

esp_err_t deleteReading(size_t reading_id);