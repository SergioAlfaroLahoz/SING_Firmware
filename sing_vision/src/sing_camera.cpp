#include "sing_camera.h"
#include "esp32-hal-log.h"

static const char *TAG = "Vision";

esp_err_t connect_camera()
{

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x\n", err);
        return err;
    }
    else
    {
        ESP_LOGI(TAG, "Camera init worked!");
        return err;
    }
}
esp_err_t disconnect_camera()
{

    // camera init
    esp_err_t err = esp_camera_deinit();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera deinit failed with error 0x%x\n", err);
        return err;
    }
    else
    {
        ESP_LOGI(TAG, "Camera deinit worked!");
        return err;
    }
}

// esp_err_t camera_trigger(camera_fb_t *fb, int flash_pin, int flash_delay_ms)
// {
//     fb = NULL;
//     esp_err_t err = ESP_OK;

//     digitalWrite(flash_pin, HIGH);
//     delay(flash_delay_ms);
//     fb = esp_camera_fb_get();
//     delay(flash_delay_ms);
//     digitalWrite(flash_pin, LOW);

//     if (!fb)
//     {
//         ESP_LOGE(TAG, "Camera capture failed");
//         esp_camera_fb_return(fb);
//         err = ESP_FAIL;
//     }
//     else
//     {
//         ESP_LOGI(TAG, "new image [%dx%d] (buffer length: %d)", fb->height, fb->width, fb->len);
//     }

//     return err;
// }

// esp_err_t release_camera_buffer(camera_fb_t *fb)
// {
//     esp_err_t err = ESP_OK;
//     esp_camera_fb_return(fb);
//     if (fb)
//     {
//         ESP_LOGE(TAG, "Camera release failed!");
//         err = ESP_FAIL;
//     }
//     else
//     {
//         ESP_LOGI(TAG, "Image buffer released");
//     }
//     return err;
// }