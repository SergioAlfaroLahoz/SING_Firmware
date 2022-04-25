#include <Arduino.h>
#include "esp_camera.h"
#include <MKRGSM.h>
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
#include "camera_pins.h"

// Fichero con los datos del proveedor y de la tarjeta SIM
#include "movistar.h"

// Inicializacion de la libreria
//GSMHttpUtils httpClient;
GSMClient client;
GPRS gprs;
GSM gsmAccess;  // include a 'true' parameter for debug enabled
bool connected; // estado de red (esta conectado no a una red)
// PIN Number
const char PINNUMBER[] = SECRET_PINNUMBER;
// APN data
const char GPRS_APN[] = SECRET_GPRS_APN;
const char GPRS_LOGIN[] = SECRET_GPRS_LOGIN;
const char GPRS_PASSWORD[] = SECRET_GPRS_PASSWORD;

GSMFileUtils fileUtils(false);

// Archivos
constexpr char *filename{"testfile"};           // nombre del archivo donde se almacena la respuesta a la peticion
constexpr char *filenamePicture{"CIF_001.jpg"}; // nombre del archivo donde se almacena la imagen
uint8_t bufferResultRead[2000];                 // buffer para almacenar la respuesta

// Datos del servidor
char server[] = "51.75.124.30";
char path[] = "/";
int port = 5000;

// Photo File Name to save in SPIFFS
camera_fb_t *fb;

// Temporizacion de envios
const int timerInterval = 30000; // Tiempo entre peticiones en ms
unsigned long previousMillis = 0;

int16_t configureCamera()
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

  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.

  config.frame_size = FRAMESIZE_CIF;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    Serial.printf("Camera init failed with error 0x%x", err);
    return -1;
  }

  sensor_t *s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID)
  {
    s->set_vflip(s, 1);       // flip it back
    s->set_brightness(s, 1);  // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);

  return 0;
}

boolean tryConnectGsm()
{
  boolean connected = false;
  int trials = 2;
  while ((!connected) && (trials > 0))
  {
    if ((gsmAccess.begin(PINNUMBER) == GSM_READY) &&
        (gprs.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD) == GPRS_READY))
    {
      connected = true;
    }
    else
    {
      trials--;
      Serial.print("Sin red, intento num: ");
      Serial.println(trials);
      delay(10000);
    }
  }

  Serial.println("Establecida la conexion de red");
  return connected;
}

void setup()
{
  // initialize serial communications and wait for port to open:
  Serial.begin(115200);
  Serial.println("Programa de test de conexion");
  Serial.println("============================");
  Serial.println("Ejemplo 1 Realiza una peticion HTTP POST de datos e imagen periodicamente");
  if (configureCamera() == 0)
  {
    Serial.println("Camara configurada correctamente");
  }
  else
  {
    Serial.println("Error en cámara");
    ESP.restart();
  }
  // connection state
  connected = false;
}

int waitForSocketResponse(uint32_t ms, uint8_t *receivedData)
{
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
    client.readBytes(receivedData, availableBytes);
    readBytes += availableBytes;
    availableBytes = client.available();
  }
  return readBytes;
}

int sendPOST(camera_fb_t *fb, char *userId, char *ean, int metalRead, int petRead)
{
  if (!fb)
  {
    Serial.println("Imagen no valida");
    return -1;
  }

  Serial.print("Conectando con el servidor: ");
  Serial.println(server);

  if (client.connect(server, port))
  {
    int result = -3;
    const char *textEAN = "--c14f2185\r\nContent-Disposition: form-data; name=\"EAN\"\r\n\r\n";
    const char *textUSER_ID = "--c14f2185\r\nContent-Disposition: form-data; name=\"USER_ID\"\r\n\r\n";
    const char *textMETAL_READ = "--c14f2185\r\nContent-Disposition: form-data; name=\"METAL_READ\"\r\n\r\n";
    const char *textPET_READ = "--c14f2185\r\nContent-Disposition: form-data; name=\"PET_READ\"\r\n\r\n";
    const char *textIMAGE = "--c14f2185\r\nContent-Disposition: form-data; name=\"image\"; filename=\"CIF_001.jpg\"\r\n\r\n";
    const char *textClose = "--c14f2185--";

    char metalReadStr[5];
    int metalRead = 1;
    int petRead = 0;
    char petReadStr[5];
    sprintf(metalReadStr, "%i", metalRead);
    sprintf(petReadStr, "%i", petRead);

    int8_t sizeLn = 2; // tamaño del retorno de carro y fin de línea tras introducir el dato correspondiente
    int32_t postSize = strlen(textEAN) + strlen(ean) + sizeLn + strlen(textUSER_ID) + strlen(userId) + sizeLn + strlen(textMETAL_READ) + strlen(metalReadStr) + sizeLn + +strlen(textPET_READ) + strlen(petReadStr) + sizeLn + strlen(textIMAGE) + fb->len + sizeLn + strlen(textClose);

    client.println("POST / HTTP/1.1");
    // headers
    client.print("Host: ");
    client.print(server);
    client.print(":");
    client.println(port);
    client.println("api-key:TESTTOKENtesttokenTESTTOKEN");
    client.println("Connection: keep-alive");
    client.print("Content-Length:");
    client.println(postSize);
    client.println("Content-Type:multipart/form-data; boundary=c14f2185");
    client.println();

    client.print(textEAN);
    client.println(ean);

    client.print(textUSER_ID);
    client.println(userId);

    client.print(textMETAL_READ);
    client.println(metalReadStr);

    client.print(textPET_READ);
    client.println(petReadStr);

    client.print(textIMAGE);
    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
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
    return result = 0;
  }
  Serial.println("No se ha podido establecer conexion");
  return -2; // no hay conexion con el servidor
}

void loop()
{
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= timerInterval)
  {
    Serial.println("Tomando foto..");
    camera_fb_t *fb = NULL;
    fb = esp_camera_fb_get();
    if (!connected)
    {
      connected = tryConnectGsm();
    }
    else
    {
      int resultPost = sendPOST(fb, "inarTecnologias", "ZaC", 1, 0);
      Serial.print("Resultado POST:");
      Serial.println(resultPost);
      esp_camera_fb_return(fb);
      uint32_t numBytesRead = waitForSocketResponse(10000, bufferResultRead);
      if (numBytesRead != 0)
      {
        Serial.printf("Leidos %i bytes en fichero\r\n", numBytesRead);
        Serial.println("*************************************************************");
        Serial.write(bufferResultRead, numBytesRead);
        Serial.println();
        Serial.println("*************************************************************");
      }
      else
      {
        Serial.println("Timeout, no ha llegado respuesta");
      }
      // if the server's disconnected, stop the client:
      if (!client.connected())
      {
        Serial.println();
        Serial.println("Cliente desconectado");
        client.stop();
      }
      Serial.println("Apagando gsm");
      gsmAccess.secureShutdown(); // se apaga utilizando pwrkey// se puede apagar por comando utilizando el metodo  gsmAccess.shutdown()
      connected = false;
      previousMillis = currentMillis;
    }
  }
}
