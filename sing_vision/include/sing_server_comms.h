#include <MKRGSM.h>
#include "esp_err.h"
#include "yoigo.h"

#include "sing_camera.h"
#include "sing_file_tools.h"

esp_err_t httpIsConnected();

esp_err_t httpConnect();

esp_err_t httpDisconnect();

esp_err_t httpPOSTcomplete(img_struct_t *img, data_struct_t *data);

esp_err_t httpPOSTimage(img_struct_t *img);

esp_err_t httpWaitForResponse(uint32_t ms);

int GETAvailableFirmwares(char *MFWV, char *DBV, char *SFWV, char *VFWV);

esp_err_t waitForFirmwareResponse(uint32_t ms, updateFWs *fws);

void availableFirmwares(updateFWs *fws, char *MFWV, char *DBV, char *SFWV, char *VFWV);

int GETUpdateFirmware(char c);

int waitForFirmwareUpdateResponse(uint32_t ms, uint8_t *receivedData);

void updateFirmware(char c);

int waitForDBUpdateResponse(uint32_t ms, uint8_t *receivedData);

void updateDB();

