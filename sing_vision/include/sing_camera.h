#include "Arduino.h"
#include "esp_camera.h"
#include "esp_err.h"

#define CONFIG_CAMERA_MODEL_AI_THINKER 1
// #define CONFIG_CAMERA_MODEL_CUSTOM 1
// #define CONFIG_CAMERA_MODEL_WROVER_KIT 1
// #define CONFIG_CAMERA_MODEL_ESP_EYE 1
// #define CONFIG_CAMERA_MODEL_ESP32_CAM_BOARD 1
#include "app_camera.h"

esp_err_t connect_camera();

esp_err_t disconnect_camera();

// esp_err_t camera_trigger(camera_fb_t *fb, int flash_pin, int flash_delay_ms);

// esp_err_t release_camera_buffer(camera_fb_t *fb);
