#include "sing_file_tools.h"

static const char *TAG = "Memory";

const char *usedReadingIdsFile = "/usedReadingIdsFile.dat";

#define MAX_READINGS_MEM 100
// FILE PATH FORMAT: /<%010d>.XXX
#define FILE_PATH_LENGTH 15

bool usedReadingIds[MAX_READINGS_MEM];

void getJpgFilePath(char *path, size_t reading_id)
{
    sprintf(path, "/%06d.jpg", reading_id);
}

void getDatFilePath(char *path, size_t reading_id)
{
    sprintf(path, "/%06d.dat", reading_id);
}

void printDataStruct(data_struct_t *data)
{
    ESP_LOGI(TAG, "TIMESTAMP length:    %d", data->timestamp_len);
    for (size_t i = 0; i < data->timestamp_len; i++)
    {
        ESP_LOGI(TAG, "     %02d : %d", i, data->timestamp[i]);
    }
    ESP_LOGI(TAG, "EAN length:          %d", data->ean_len);
    for (size_t i = 0; i < data->ean_len; i++)
    {
        ESP_LOGI(TAG, "     %02d : 0x%02X", i, data->ean[i]);
    }
    ESP_LOGI(TAG, "USER ID length:      %d", data->user_id_len);
    for (size_t i = 0; i < data->user_id_len; i++)
    {
        ESP_LOGI(TAG, "     %02d : 0x%02X", i, data->user_id[i]);
    }
    ESP_LOGI(TAG, "PET:                 %f", data-> pet_val);
    ESP_LOGI(TAG, "METAL:               %d", data-> metal_val);
    ESP_LOGI(TAG, "BATTERY LEVEL:               %u", data-> battery_val);
}

esp_err_t formatMemory()
{
    if (!SPIFFS.format())
    {
        ESP_LOGE(TAG, "Error formatting SPIFFS memory");
        return ESP_FAIL;
    }
    else
    {
        ESP_LOGI(TAG, "SPIFFS memory formatted");
        return ESP_OK;
    }
}

void listFiles()
{
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while (file)
    {
        ESP_LOGI(TAG, "    %s - % 10d bytes", file.name(), file.size());
        file = root.openNextFile();
    }
    file.close();
    root.close();
}

size_t getFreeBytes()
{
    return (SPIFFS.totalBytes() - SPIFFS.usedBytes());
}

void checkMemory(bool useSlotArray, bool *usedSlotsArray)
{
    if (useSlotArray)
    {
        ESP_LOGI(TAG, "Checking memory from loaded information ...");
        for (size_t i = 0; i < MAX_READINGS_MEM; i++)
        {
            if (usedSlotsArray[i] == true)
            {
                char img_path[FILE_PATH_LENGTH];
                char data_path[FILE_PATH_LENGTH];
                getJpgFilePath(img_path, i);
                getDatFilePath(data_path, i);
                bool img_exists = SPIFFS.exists(img_path);
                bool data_exists = SPIFFS.exists(data_path);
                if ((img_exists) & (!data_exists))
                {
                    ESP_LOGE(TAG, "Unpaired image (%s) removed", img_path);
                    SPIFFS.remove(img_path);
                    img_exists = false;
                }
                if ((!img_exists) & (data_exists))
                {
                    ESP_LOGE(TAG, "Unpaired data (%s) removed", data_path);
                    SPIFFS.remove(data_path);
                    data_exists = false;
                }
                usedSlotsArray[i] = img_exists & data_exists;
                // ESP_LOGI(TAG, "Memory slot %d is %s", i, (usedSlotsArray[i]) ? "used" : "free");
            }
        }
    }
    else
    {
        ESP_LOGI(TAG, "Checking memory from existing files ...");
        File root = SPIFFS.open("/");
        File file = root.openNextFile();
        while (file)
        {
            ESP_LOGI(TAG, "    %s - % 10d bytes", file.name(), file.size());
            size_t readingId;
            bool img_exists = false;
            char img_path[FILE_PATH_LENGTH];
            bool data_exists = false;
            char data_path[FILE_PATH_LENGTH];
            if (sscanf(file.name(), "/%06d.dat", &readingId))
            {
                data_exists = true;
                getJpgFilePath(img_path, readingId);
                img_exists = SPIFFS.exists(img_path);
            }
            else if (sscanf(file.name(), "/%06d.jpg", &readingId))
            {
                img_exists = true;
                getDatFilePath(data_path, readingId);
                data_exists = SPIFFS.exists(data_path);
            }
            if ((img_exists) & (!data_exists))
            {
                ESP_LOGE(TAG, "Unpaired image (%s) removed", img_path);
                SPIFFS.remove(img_path);
                img_exists = false;
            }
            if ((!img_exists) & (data_exists))
            {
                ESP_LOGE(TAG, "Unpaired data (%s) removed", data_path);
                SPIFFS.remove(data_path);
                data_exists = false;
            }
            if (img_exists & data_exists)
            {
                usedSlotsArray[readingId] = true;
            }
            file = root.openNextFile();
        }
        file.close();
        root.close();
    }
}

esp_err_t initMemory(bool formatMem)
{
    // Initialize SPIFFS
    if (!SPIFFS.begin(true))
    {
        ESP_LOGE(TAG, "Error mounting SPIFFS");
        return ESP_FAIL;
    }
    else
    {
        ESP_LOGI(TAG, "SPIFFS mounted");
        ESP_LOGI(TAG, "SPIFFS: %d / %d bytes used", SPIFFS.usedBytes(), SPIFFS.totalBytes());
    }
    if (formatMem)
    {
        ESP_LOGI(TAG, "Format memory");
        formatMemory();
        ESP_LOGI(TAG, "SPIFFS: %d / %d bytes used", SPIFFS.usedBytes(), SPIFFS.totalBytes());
    }
    // listFiles();
    // Get files in memory, pair images and data files and fill usedReadingIds array
    bool fileLoaded = SPIFFS.exists(usedReadingIdsFile);
    if (fileLoaded)
    {
        File initFile = SPIFFS.open(usedReadingIdsFile, FILE_READ);
        size_t ret = initFile.read((byte *)&usedReadingIds, initFile.size());
        if (ret != 0)
        {
            ESP_LOGI(TAG, "Readings in memory data loaded!");
        }
        else
        {
            ESP_LOGE(TAG, "Data load failed");
            fileLoaded = false;
        }
    }
    ESP_LOGI(TAG, "Checking memory ...");
    checkMemory(fileLoaded, usedReadingIds);
    ESP_LOGI(TAG, "Memory checked: %d free slots - %d free bytes", getFreeMemorySlots(), getFreeBytes());
    ESP_LOGD(TAG, "getFreeMemorySlots(): %d", (int)getFreeMemorySlots());
    ESP_LOGD(TAG, "getFreeBytes(): %d bytes [MIN_FREE_BYTES: %d bytes]", getFreeBytes(), MIN_FREE_BYTES);
    return ESP_OK;
}

size_t getNextFreeReadingId()
{
    size_t reading_id = -1;
    if (getFreeBytes() > MIN_FREE_BYTES)
    {
        for (size_t i = 0; i < MAX_READINGS_MEM; i++)
        {
            if (!usedReadingIds[i])
            {
                reading_id = i;
                break;
            }
        }
    }
    return reading_id;
}

size_t getNextUsedReadingId()
{
    size_t reading_id = -1;
    for (size_t i = 0; i < MAX_READINGS_MEM; i++)
    {
        if (usedReadingIds[i])
        {
            reading_id = i;
            break;
        }
    }
    return reading_id;
}

size_t getFreeMemorySlots()
{
    size_t n_free = 0;
    for (size_t i = 0; i < MAX_READINGS_MEM; i++)
    {
        if (!usedReadingIds[i])
        {
            n_free++;
        }
    }
    // ESP_LOGI(TAG, "%d free slots in memory", n_free);
    return n_free;
}

size_t getUsedMemorySlots()
{
    size_t n_used = 0;
    for (size_t i = 0; i < MAX_READINGS_MEM; i++)
    {
        if (usedReadingIds[i])
        {
            n_used++;
        }
    }
    // ESP_LOGI(TAG, "%d used slots in memory", n_used);
    return n_used;
}

esp_err_t saveUsedReadingIdsData()
{

    esp_err_t out_err = ESP_FAIL;
    File file = SPIFFS.open(usedReadingIdsFile, FILE_WRITE);
    if (!file)
    {
        ESP_LOGE(TAG, "Failed to load file %s in writing mode", usedReadingIdsFile);
    }
    else
    {
        size_t ret = file.write((byte *)&usedReadingIds, sizeof(usedReadingIds));
        if (ret != 0)
        {
            ESP_LOGI(TAG, "Data saved: %s [%d bytes]", usedReadingIdsFile, file.size());
            out_err = ESP_OK;
        }
    }
    file.close();

    return out_err;
}

esp_err_t writeReading(size_t reading_id, camera_fb_t *image, uint8_t *timestamp, size_t timestamp_len, byte *user_id, size_t user_id_len, char *ean, uint8_t ean_len, float *pet_val, int *metal_val, unsigned int *battery_val)
{
    esp_err_t out_err = ESP_FAIL;

    bool image_saved = false;

    if (reading_id < 0)
    {
        ESP_LOGE(TAG, "Not free slots in memory");
        return out_err;
    }

    // Photo file name
    char image_path[FILE_PATH_LENGTH];
    getJpgFilePath(image_path, reading_id);
    char data_path[FILE_PATH_LENGTH];
    getDatFilePath(data_path, reading_id);

    // ESP_LOGD(TAG, "Saving image: %s", image_path);
    File file = SPIFFS.open(image_path, FILE_WRITE);
    if (!file)
    {
        ESP_LOGE(TAG, "Failed to load file %s in writing mode", image_path);
    }
    else
    {
        size_t ret = file.write((byte *)image->buf, image->len);
        if (ret != 0)
        {
            ESP_LOGI(TAG, "Image saved: %s [%d bytes]", image_path, file.size());
            image_saved = true;
        }
    }
    file.close();

    if (image_saved)
    {
        // ESP_LOGD(TAG, "Saving data: %s", data_path);
        file = SPIFFS.open(data_path, FILE_WRITE);
        if (!file)
        {
            ESP_LOGE(TAG, "Failed to load file %s in writing mode", data_path);
            bool ret = SPIFFS.remove(image_path);
            if (!ret)
            {
                ESP_LOGE(TAG, "Failed to delete file %s", image_path);
            }
            ret = SPIFFS.remove(data_path);
            if (!ret)
            {
                ESP_LOGE(TAG, "Failed to delete file %s", data_path);
            }
        }
        else
        {
            data_struct_t data;
            data.timestamp_len = timestamp_len;
            for (uint8_t i = 0; i < timestamp_len; i++)
            {
                data.timestamp[i] = timestamp[i];
            }
            data.user_id_len = user_id_len;
            for (uint8_t i = 0; i < user_id_len; i++)
            {
                data.user_id[i] = user_id[i];
            }
            data.ean_len = ean_len;
            for (uint8_t i = 0; i < ean_len; i++)
            {
                data.ean[i] = ean[i];
            }
            data.pet_val = *pet_val;
            data.metal_val = *metal_val;
            data.battery_val = *battery_val;

            size_t ret = file.write((byte *)&data, sizeof(data_struct_t));

            if (ret != 0)
            {
                ESP_LOGI(TAG, "Data saved: %s [%d bytes]", data_path, file.size());
                out_err = ESP_OK;
            }
        }
        file.close();
    }
    else
    {
        bool ret = SPIFFS.remove(image_path);
        if (!ret)
        {
            ESP_LOGE(TAG, "Failed to delete file %s", image_path);
        }
    }

    if (out_err == ESP_OK)
    {
        usedReadingIds[reading_id] = true;
        saveUsedReadingIdsData();
        ESP_LOGI(TAG, "Memory slot %d used", reading_id);
    }

    return out_err;
}

esp_err_t getReading(size_t reading_id, byte *image_buff, size_t image_len, uint8_t *timestamp, size_t timestamp_len, byte *user_id, size_t user_id_len, char *ean, size_t ean_len, float *pet_val, int *metal_val, unsigned int *battery_val)
{
    esp_err_t out_err = ESP_FAIL;
    bool image_loaded = false;

    // Photo file name
    char image_path[FILE_PATH_LENGTH];
    getJpgFilePath(image_path, reading_id);
    char data_path[FILE_PATH_LENGTH];
    getDatFilePath(data_path, reading_id);

    ESP_LOGD(TAG, "Loading image: %s", image_path);
    File file = SPIFFS.open(image_path, FILE_READ);
    if (!file)
    {
        ESP_LOGE(TAG, "Failed to load file %s in reading mode", image_path);
    }
    else
    {
        // ESP_LOGI(TAG, "%s [%d bytes] loaded in reading mode", image_path, file.size());
        image_buff = (byte *)malloc(file.size());
        if (!image_buff)
        {
            ESP_LOGE(TAG, "Memory malloc failed");
        }
        size_t ret = file.read(image_buff, file.size());
        if (ret != 0)
        {
            ESP_LOGI(TAG, "Image loaded: %s [%d bytes]", image_path, file.size());
            image_len = file.size();
            image_loaded = true;
        }
        else
        {
            ESP_LOGE(TAG, "Image load failed");
        }
    }
    file.close();

    if (image_loaded)
    {
        ESP_LOGD(TAG, "Loading data: %s", data_path);
        file = SPIFFS.open(data_path, FILE_READ);
        if (!file)
        {
            ESP_LOGE(TAG, "Failed to load file %s in reading mode", data_path);
        }
        else
        {
            // ESP_LOGI(TAG, "%s [%d bytes] loaded in reading mode", data_path, file.size());
            data_struct_t *data_buff = (data_struct_t *)malloc(file.size());
            size_t ret = file.read((byte *)data_buff, file.size());

            if (ret != 0)
            {
                ESP_LOGI(TAG, "Data loaded: %s [%d bytes]", data_path, file.size());

                Serial.println(data_buff->ean_len);
                Serial.println(data_buff->user_id_len);
                Serial.println(data_buff->timestamp_len);
                Serial.println(data_buff->pet_val);
                Serial.println(data_buff->metal_val);
                Serial.println(data_buff->battery_val);

                // FREE MEMORY!!
                free(data_buff);

                out_err = ESP_OK;
            }
            else
            {
                ESP_LOGE(TAG, "Data load failed");
            }
        }
        file.close();
    }

    return out_err;
}

esp_err_t getSavedImage(size_t reading_id, img_struct_t *read_img)
{

    esp_err_t err = ESP_FAIL;

    char image_path[FILE_PATH_LENGTH];
    getJpgFilePath(image_path, reading_id);

    // ESP_LOGD(TAG, "Loading image: %s", image_path);
    File file = SPIFFS.open(image_path, FILE_READ);
    if (!file)
    {
        ESP_LOGE(TAG, "Failed to load file %s in reading mode", image_path);
    }
    else
    {
        // ESP_LOGI(TAG, "%s [%d bytes] loaded in reading mode", image_path, file.size());
        read_img->buff = (byte *)malloc(file.size());
        if (!read_img->buff)
        {
            ESP_LOGE(TAG, "Memory malloc failed");
        }
        else
        {
            size_t ret = file.read(read_img->buff, file.size());
            if (ret != 0)
            {
                ESP_LOGI(TAG, "Image loaded: %s [%d bytes]", image_path, file.size());
                read_img->len = file.size();
                err = ESP_OK;
            }
            else
            {
                ESP_LOGE(TAG, "Image load failed");
            }
        }
    }
    file.close();

    return err;
}

esp_err_t getSavedData(size_t reading_id, data_struct_t *read_data)
{
    esp_err_t err = ESP_FAIL;

    char data_path[FILE_PATH_LENGTH];
    getDatFilePath(data_path, reading_id);

    // ESP_LOGD(TAG, "Loading data: %s", data_path);
    File file = SPIFFS.open(data_path, FILE_READ);
    if (!file)
    {
        ESP_LOGE(TAG, "Failed to load file %s in reading mode", data_path);
    }
    else
    {
        // ESP_LOGI(TAG, "%s [%d bytes] loaded in reading mode", data_path, file.size());
        // read_data = (data_struct_t *)malloc(file.size());
        size_t ret = file.read((byte *)read_data, file.size());
        if (ret != 0)
        {
            ESP_LOGI(TAG, "Data loaded: %s [%d bytes]", data_path, file.size());
            // printDataStruct(read_data);
            err = ESP_OK;
        }
        else
        {
            ESP_LOGE(TAG, "Data load failed");
        }
    }
    file.close();

    return err;
}

esp_err_t releaseImage(img_struct_t *img)
{
    free(img->buff);
    // TODO Check that free worked
    return ESP_OK;
}

esp_err_t deleteReading(size_t reading_id)
{
    esp_err_t out_err = ESP_FAIL;

    char image_path[FILE_PATH_LENGTH];
    getJpgFilePath(image_path, reading_id);
    char data_path[FILE_PATH_LENGTH];
    getDatFilePath(data_path, reading_id);

    bool ret = SPIFFS.remove(image_path);
    if (!ret)
    {
        ESP_LOGE(TAG, "Error removing %s", image_path);
    }
    SPIFFS.remove(data_path);
    if (!ret)
    {
        ESP_LOGE(TAG, "Error removing %s", data_path);
    }
    usedReadingIds[reading_id] = false;
    saveUsedReadingIdsData();

    out_err = ESP_OK;
    ESP_LOGI(TAG, "Memory slot %d is free", reading_id);

    return out_err;
}