#include "sing_server_comms.h"
#include "esp32-hal-log.h"

#define MAX_HTTP_OUTPUT_BUFFER 4096
#define MAX_CONNECTION_TRIALS 3

static const char *TAG = "CommsHTTP";

GSMClient client;
GPRS gprs;
// GSM gsmAccess(true);  // include a 'true' parameter for debug enabled
GSM gsmAccess;
// PIN Number
const char PINNUMBER[] = SECRET_PINNUMBER;
// APN data
const char GPRS_APN[] = SECRET_GPRS_APN;
const char GPRS_LOGIN[] = SECRET_GPRS_LOGIN;
const char GPRS_PASSWORD[] = SECRET_GPRS_PASSWORD;

// GSMFileUtils fileUtils(false);

// Server data
char server[] = "51.75.124.30";
char path[] = "/";
int port = 5000;

const int BUFFER_SIZE = 100;

uint8_t bufferResultRead[8000];

esp_err_t httpIsConnected()
{
    esp_err_t out_err = ESP_FAIL;
    GSM3_NetworkStatus_t gsm_status = gsmAccess.status();
    switch (gsm_status)
    {
    case ERROR:
        ESP_LOGD(TAG, "GSM STATUS: ERROR");
        break;
    case IDLE:
        ESP_LOGD(TAG, "GSM STATUS: IDLE");
        break;
    case CONNECTING:
        ESP_LOGD(TAG, "GSM STATUS: CONNECTING");
        break;
    case GSM_READY:
        ESP_LOGD(TAG, "GSM STATUS: GSM_READY");
        out_err = ESP_OK;
        break;
    case GPRS_READY:
        ESP_LOGD(TAG, "GSM STATUS: GPRS_READY");
        out_err = ESP_OK;
        break;
    case TRANSPARENT_CONNECTED:
        ESP_LOGD(TAG, "GSM STATUS: TRANSPARENT_CONNECTED");
        break;
    case GSM_OFF:
        ESP_LOGD(TAG, "GSM STATUS: GSM_OFF");
        break;
    }
    return out_err;
}

esp_err_t httpConnect()
{
    esp_err_t out_err = ESP_FAIL;
    if ((gsmAccess.begin(PINNUMBER) == GSM_READY) &&
        (gprs.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD) == GPRS_READY))
    {
        httpIsConnected();
        out_err = ESP_OK;
    }
    return out_err;
}

esp_err_t httpDisconnect()
{
    if (gsmAccess.secureShutdown())
    {
        ESP_LOGI(TAG, "HTTP Disconnected");
        return ESP_OK;
    }
    return ESP_FAIL;
}

esp_err_t httpPOSTcomplete(img_struct_t *img, data_struct_t *data)
{
    esp_err_t out_err = ESP_FAIL;

    if (!img->buff)
    {
        ESP_LOGE(TAG, "Not image found!");
    }
    else
    {
        ESP_LOGI(TAG, "Connect to server: %s:%d", server, port);

        if (client.connect(server, port))
        {
            ESP_LOGI(TAG, "Socket connected!");

            const char *textID = "--c14f2185\r\nContent-Disposition: form-data; name=\"DEVICE_ID\"\r\n\r\n";
            const char *textEAN = "--c14f2185\r\nContent-Disposition: form-data; name=\"EAN\"\r\n\r\n";
            const char *textUSER_ID = "--c14f2185\r\nContent-Disposition: form-data; name=\"USER_ID\"\r\n\r\n";
            const char *textTIMESTAMP = "--c14f2185\r\nContent-Disposition: form-data; name=\"TIMESTAMP\"\r\n\r\n";
            const char *textMETAL_READ = "--c14f2185\r\nContent-Disposition: form-data; name=\"METAL_READ\"\r\n\r\n";
            const char *textPET_READ = "--c14f2185\r\nContent-Disposition: form-data; name=\"PET_READ\"\r\n\r\n";
            const char *textBATTERY_READ = "--c14f2185\r\nContent-Disposition: form-data; name=\"BATTERY_READ\"\r\n\r\n";
            const char *textIMAGE = "--c14f2185\r\nContent-Disposition: form-data; name=\"image\"; filename=\"CIF_001.jpg\"\r\n\r\n";
            const char *textClose = "--c14f2185--";

            ESP_LOGI(TAG, "idReadStr: %s", deviceID);

            char eanReadStr[data->ean_len * 2];
            for (size_t i = 0; i < data->ean_len; i++)
            {
                sprintf(&eanReadStr[2 * i], "%02X", data->ean[i]);
            }
            ESP_LOGI(TAG, "eanReadStr: %s", eanReadStr);

            char timestampReadStr[data->timestamp_len * 2];
            for (size_t i = 0; i < data->timestamp_len; i++)
            {
                sprintf(&timestampReadStr[2 * i], "%02d", data->timestamp[i]);
            }
            ESP_LOGI(TAG, "timestampReadStr: %s", timestampReadStr);

            char useridReadStr[data->user_id_len * 2];
            for (size_t i = 0; i < data->user_id_len; i++)
            {
                sprintf(&useridReadStr[2 * i], "%02X", data->user_id[i]);
            }
            ESP_LOGI(TAG, "useridReadStr: %s", useridReadStr);

            char metalReadStr[5];
            sprintf(metalReadStr, "%d", data->metal_val);
            ESP_LOGI(TAG, "metalReadStr: %s", metalReadStr);

            char petReadStr[7];
            sprintf(petReadStr, "%6.2f", data->pet_val);
            petReadStr[6] = 0;
            ESP_LOGI(TAG, "petReadStr: %s", petReadStr);

            char batteryLvlReadStr[7];
            sprintf(batteryLvlReadStr, "%u", data->battery_val);
            ESP_LOGI(TAG, "batteryLvlReadStr: %s", batteryLvlReadStr);

            int8_t sizeLn = 2; // tamaño del retorno de carro y fin de línea tras introducir el dato correspondiente
            int32_t postSize =
                strlen(textID) + strlen(deviceID) + sizeLn +
                strlen(textEAN) + strlen(eanReadStr) + sizeLn +
                strlen(textTIMESTAMP) + strlen(timestampReadStr) + sizeLn +
                strlen(textUSER_ID) + strlen(useridReadStr) + sizeLn +
                strlen(textMETAL_READ) + strlen(metalReadStr) + sizeLn +
                strlen(textPET_READ) + strlen(petReadStr) + sizeLn +
                strlen(textBATTERY_READ) + strlen(batteryLvlReadStr) + sizeLn +
                strlen(textIMAGE) + img->len + sizeLn +
                strlen(textClose);

            client.print("POST ");
            client.print(path);
            client.println(" HTTP/1.1");
            client.print("Host: ");
            client.print(server);
            client.print(":");
            client.println(port);
            client.println("api-key: BML?xuLJHpfq7uL=26SuBHA%5_*SxnChQq*Hu*G7pkB8krXx=J3st6J8^mSDSZ^f");
            client.println("Connection: keep-alive");
            client.print("Content-Length:");
            client.println(postSize);
            client.println("Content-Type:multipart/form-data; boundary=c14f2185");
            client.println();

            client.print(textID);
            client.println(deviceID);

            client.print(textEAN);
            client.println(eanReadStr);

            client.print(textTIMESTAMP);
            client.println(timestampReadStr);

            client.print(textUSER_ID);
            client.println(useridReadStr);

            client.print(textMETAL_READ);
            client.println(metalReadStr);

            client.print(textPET_READ);
            client.println(petReadStr);

            client.print(textBATTERY_READ);
            client.println(batteryLvlReadStr);

            client.print(textIMAGE);
            uint8_t *imgBuf = img->buff;
            size_t imgLen = img->len;
            uint16_t sizeBuf = 1000;
            for (size_t n = 0; n < imgLen; n = n + sizeBuf)
            {
                if (n + sizeBuf < imgLen)
                {
                    client.writeBinary(imgBuf, sizeBuf);
                    imgBuf += sizeBuf;
                }
                else if (imgLen % sizeBuf > 0)
                {
                    size_t remainder = imgLen % sizeBuf;
                    client.writeBinary(imgBuf, remainder);
                }
            }
            client.println();
            client.println(textClose);
            out_err = ESP_OK;
        }
        else
        {
            ESP_LOGE(TAG, "Unable to connect to server");
        }
        if (out_err == ESP_OK)
        {
            ESP_LOGI(TAG, "Wait for server response");
            out_err = httpWaitForResponse(500);
            if (out_err == ESP_OK)
            {
                ESP_LOGI(TAG, "Data sent over HTTP and received");
            }
        }
        client.stop();
    }

    return out_err;
}

esp_err_t httpPOSTimage(img_struct_t *img)
{
    esp_err_t out_err = ESP_FAIL;

    if (!img->buff)
    {
        ESP_LOGE(TAG, "Not image found!");
    }
    else
    {
        ESP_LOGI(TAG, "Connect to server: %s:%d", server, port);

        if (client.connect(server, port))
        {
            ESP_LOGI(TAG, "Socket connected!");

            const char *textIMAGE = "--c14f2185\r\nContent-Disposition: form-data; name=\"image\"; filename=\"CIF_001.jpg\"\r\n\r\n";
            const char *textClose = "--c14f2185--";

            int8_t sizeLn = 2; // tamaño del retorno de carro y fin de línea tras introducir el dato correspondiente
            int32_t postSize = strlen(textIMAGE) + img->len + sizeLn +
                               strlen(textClose);
            ESP_LOGI(TAG, "postSize: %d", postSize);

            client.println("POST / HTTP/1.1");
            // headers
            client.print("Host: ");
            client.print(server);
            client.print(":");
            client.println(port);
            // client.println("api-key: BML?xuLJHpfq7uL=26SuBHA%5_*SxnChQq*Hu*G7pkB8krXx=J3st6J8^mSDSZ^f");
            client.println("Connection: keep-alive");
            client.print("Content-Length:");
            client.println(postSize);
            client.println("Content-Type: multipart/form-data; boundary=c14f2185");
            client.println();

            client.print(textIMAGE);
            uint8_t *fbBuf = img->buff;
            size_t fbLen = img->len;
            uint16_t sizeBuf = 1000;
            for (size_t n = 0; n < fbLen; n = n + sizeBuf)
            {
                if (n + sizeBuf < fbLen)
                {
                    client.writeBinary(fbBuf, sizeBuf);
                    fbBuf += sizeBuf;
                }
                else if (fbLen % sizeBuf > 0)
                {
                    size_t remainder = fbLen % sizeBuf;
                    client.writeBinary(fbBuf, remainder);
                }
            }
            client.println();
            client.println(textClose);

            out_err = ESP_OK;
        }
        else
        {
            ESP_LOGE(TAG, "Unable to connect to server");
        }
        if (out_err == ESP_OK)
        {
            ESP_LOGI(TAG, "Wait for server response");
            out_err = httpWaitForResponse(3000);
            if (out_err == ESP_OK)
            {
                ESP_LOGI(TAG, "Data sent over HTTP and received");
            }
            else
            {
                ESP_LOGE(TAG, "No response from server");
            }
        }
        client.stop();
    }
    return out_err;
}

esp_err_t httpWaitForResponse(uint32_t ms)
{
    uint8_t bufferResultRead[2000];
    int readBytes = 0;
    int availableBytes = client.available();

    while ((ms > 0) && (availableBytes == 0))
    {
        delay(1);
        availableBytes = client.available();
        ms--;
    }

    while (availableBytes != 0)
    {
        client.readBytes(bufferResultRead, availableBytes);
        readBytes += availableBytes;
        availableBytes = client.available();
    }

    if (readBytes != 0)
    {
        ESP_LOGI(TAG, "Response: %i bytes\r\n", readBytes);
        ESP_LOGI(TAG, "===================================");
        ESP_LOGI(TAG, "%.*s", readBytes, bufferResultRead);
        ESP_LOGI(TAG, "===================================");
        return ESP_OK;
    }
    else
    {
        ESP_LOGE(TAG, "Socket respone timed out! No response from server!");
        return ESP_FAIL;
    }
}


int GETAvailableFirmwares(char *MFWV, char *DBV, char *SFWV, char *VFWV){
  ESP_LOGI(TAG, "Conectando con el servidor: ");
  ESP_LOGI(TAG, "%s", server);

  char masterV[6];
  memcpy(masterV, MFWV, sizeof(masterV));
  ESP_LOGI(TAG, "%s", masterV);
  char bbddV[6];
  memcpy(bbddV, DBV, sizeof(bbddV));
  ESP_LOGI(TAG, "%s", bbddV);
  char sensorsV[6];
  memcpy(sensorsV, SFWV, sizeof(sensorsV));
  ESP_LOGI(TAG, "%s", sensorsV);
  char visionV[10];
  memcpy(visionV, VFWV, sizeof(visionV));
  ESP_LOGI(TAG, "%s", visionV);

  if (client.connect(server, port))
  {
    client.print("GET /availableFWs");
    client.print("?master=");
    client.print(masterV);
    client.print("&sensors=");
    client.print(sensorsV);
    client.print("&vision=");
    client.print(visionV);
    client.print("&bbdd=");
    client.print(bbddV);
    client.println(" HTTP/1.0");
    // headers
    client.print("Host: ");
    client.print(server);
    client.print(":");
    client.print(port);
    client.println("api-key: BML?xuLJHpfq7uL=26SuBHA%5_*SxnChQq*Hu*G7pkB8krXx=J3st6J8^mSDSZ^f");
    client.println();
    return 0;
  }
  ESP_LOGI(TAG, "No se ha podido establecer conexion");
  return -2; // no hay conexion con el servidor
}


esp_err_t waitForFirmwareResponse(uint32_t ms, updateFWs *fws)
{
    uint8_t bufferResultRead[2000];
    int readBytes = 0;
    int availableBytes = client.available();

    while ((ms > 0) && (availableBytes == 0))
    {
        delay(1);
        availableBytes = client.available();
        ms--;
    }

    for(int i=0; i<6; i++){
        uint8_t firstBuffer[500];
        client.readBytesUntil('\n', firstBuffer, BUFFER_SIZE);
        // Serial.write(firstBuffer, sizeof(firstBuffer));
    }

    while (availableBytes != 0)
    {
        client.readBytes(bufferResultRead, availableBytes);
        readBytes += availableBytes;
        availableBytes = client.available();
    }

    if (readBytes != 0)
    {
        ESP_LOGI(TAG, "Response: %i bytes\r\n", readBytes);
        ESP_LOGI(TAG, "===================================");
        ESP_LOGI(TAG, "%.*s", 4, bufferResultRead);
        ESP_LOGI(TAG, "===================================");
        memcpy(&fws->masterFW, &bufferResultRead[0], 1);
        memcpy(&fws->sensorsFW, &bufferResultRead[1], 1);
        memcpy(&fws->visionFW, &bufferResultRead[2], 1);
        memcpy(&fws->bbdd, &bufferResultRead[3], 1);
        return ESP_OK;
    }
    else
    {
        ESP_LOGE(TAG, "Socket respone timed out! No response from server!");
        return ESP_FAIL;
    }
}


void availableFirmwares(updateFWs *fws, char *MFWV, char *DBV, char *SFWV, char *VFWV){
    int resultGet = GETAvailableFirmwares(MFWV, DBV, SFWV, VFWV);
    ESP_LOGI(TAG, "Get result: %i \r\n", resultGet);
    uint32_t numBytesRead = waitForFirmwareResponse(3000, fws);
}


int GETUpdateFirmware(char c){
  ESP_LOGI(TAG, "Conectando con el servidor: ");
  ESP_LOGI(TAG, "%s", server);

  if (client.connect(server, port))
  {
    if(c=='m'){
        client.println("GET /master HTTP/1.0");
    }else if(c=='s'){
        client.println("GET /sensors HTTP/1.0");
    }else if(c=='v'){
        client.println("GET /vision HTTP/1.0");
    }else if(c=='b'){
        client.println("GET /bbdd HTTP/1.0");
    }
    // headers
    client.print("Host: ");
    client.print(server);
    client.print(":");
    client.print(port);
    client.println("api-key: BML?xuLJHpfq7uL=26SuBHA%5_*SxnChQq*Hu*G7pkB8krXx=J3st6J8^mSDSZ^f");
    client.println();
    return 0;
  }
  ESP_LOGI(TAG, "No se ha podido establecer conexion");
  return -2; // no hay conexion con el servidor
}


int waitForFirmwareUpdateResponse(uint32_t ms, uint8_t *receivedData){
  int readBytes = 0;
  int availableBytes = client.available();
  while ((ms > 0) && (availableBytes == 0))
  {
    delay(1);
    availableBytes = client.available();
    ms--;
  }
  ESP_LOGI(TAG, "%i", availableBytes);
  File FWFile = SPIFFS.open("/firmware.bin", FILE_APPEND);

  for(int i=0; i<6; i++){
    uint8_t firstBuffer[500];
    client.readBytesUntil('\n', firstBuffer, BUFFER_SIZE);
    // Serial.write(firstBuffer, sizeof(firstBuffer));
  }

  ESP_LOGI(TAG, "-----------------END-----------------");
  delay(5000);
 
  while (availableBytes != 0)
  {
    client.readBytes(receivedData, availableBytes);
    // Serial.write(receivedData, availableBytes);
    FWFile.write(receivedData, availableBytes);
    // for(int i=0; i<availableBytes; i++){
    //   FWFile.write((byte)receivedData[i]);
    // }
    readBytes += availableBytes;
    availableBytes = client.available();
  }
  FWFile.close();
  ESP_LOGI(TAG, "File written");
  return readBytes;
}


void updateFirmware(char c){
    SPIFFS.remove("/firmware.bin");
    ESP_LOGI(TAG, "Sending GET mssg...");
    delay(1000);
    int resultGet = GETUpdateFirmware(c);
    ESP_LOGI(TAG, "Get result: %i \r\n", resultGet);
    uint32_t numBytesRead = waitForFirmwareUpdateResponse(3000, bufferResultRead);
}

int waitForDBUpdateResponse(uint32_t ms, uint8_t *receivedData){
  int readBytes = 0;
  int availableBytes = client.available();
  while ((ms > 0) && (availableBytes == 0))
  {
    delay(1);
    availableBytes = client.available();
    ms--;
  }
  ESP_LOGI(TAG, "%i", availableBytes);
  File FWFile = SPIFFS.open("/bbdd.txt", FILE_APPEND);

  for(int i=0; i<6; i++){
    uint8_t firstBuffer[500];
    client.readBytesUntil('\n', firstBuffer, BUFFER_SIZE);
    // Serial.write(firstBuffer, sizeof(firstBuffer));
  }

  ESP_LOGI(TAG, "-----------------END-----------------");
  delay(5000);
 
  while (availableBytes != 0)
  {
    client.readBytes(receivedData, availableBytes);
    // Serial.write(receivedData, availableBytes);
    FWFile.write(receivedData, availableBytes);
    // for(int i=0; i<availableBytes; i++){
    //   FWFile.write((byte)receivedData[i]);
    // }
    readBytes += availableBytes;
    availableBytes = client.available();
  }
  FWFile.close();
  ESP_LOGI(TAG, "File written");
  return readBytes;
}

void updateDB(){
    SPIFFS.remove("/bbdd.txt");
    ESP_LOGI(TAG, "Sending GET mssg...");
    delay(1000);
    int resultGet = GETUpdateFirmware('b');
    ESP_LOGI(TAG, "Get result: %i \r\n", resultGet);
    uint32_t numBytesRead = waitForDBUpdateResponse(3000, bufferResultRead);
}