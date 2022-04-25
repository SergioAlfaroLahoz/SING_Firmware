#include "plasticDetector.hpp"

// Configure port UART
HardwareSerial PET_serial(2); 

void PlasticDetector::init(){
  PET_serial.begin(115200, SERIAL_8N1, 26, 25); //25,26
  //delay(10000);  
  // //device info
  // PET_serial.write("h\n");
  // delay(30); 
  // while(PET_serial.available()){
  //   Serial.print(char(PET_serial.read()));
  //   //PET_serial.read();
  // }
  // //delay(1000); 
  // Serial.flush();

  //scans, average
  PET_serial.write("V50,4\n"); //100,5 20,2 50,4
  delay(30); 
  while(PET_serial.available()){
    PET_serial.read();
    //Serial.print(char(PET_serial.read()));
  }
  //delay(1000); 
  Serial.flush();

  //wv meas
  for (size_t i = 0; i < 70; i++) //201, 70
  {
      PET_serial.write("W");
      PET_serial.write(indices[i]);
      PET_serial.write(",");
      PET_serial.write(wv[i]);
      PET_serial.write(".0000\n");
      delay(30); 
      while(PET_serial.available()){
        PET_serial.read();
        //Serial.print(char(PET_serial.read()));
      }
  }
  //delay(2000); 
  Serial.flush();

  //estim_time
  PET_serial.write("E\n");
  delay(30); 
  while(PET_serial.available()){
    //PET_serial.read();
    Serial.print(char(PET_serial.read()));
    //770 ms
  }
  //delay(1000); 
  Serial.flush();
}

bool PlasticDetector::lightON(){
  //light
  PET_serial.write("LI100\n");
  delay(30); 
  while(PET_serial.available()){
    PET_serial.read();
    //Serial.println(char(PET_serial.read()));
  }
  //delay(500); // no quitar //? Necesarios?
  Serial.flush();
  //-----Falta añadir la opcion false si no hay comm--
  return true;
}

bool PlasticDetector::lightOFF(){
  //light
  PET_serial.write("LI0\n");
  delay(30); 
  while(PET_serial.available()){
    PET_serial.read();
    //Serial.print(char(PET_serial.read()));
  }
  delay(10); // no quitar
  Serial.flush();
  //-----Falta añadir la opcion false si no hay comm--
  return true;
}

bool PlasticDetector::start_meas(){
   //start scan
  PET_serial.write("XM\n");
  //delay(800); //3500 ,300

   //-----Falta añadir la opcion false si no hay comm--
  return true;
}

bool PlasticDetector::read_meas(){
  if(PET_serial.available()){
    while(PET_serial.available()){
      PET_serial.read();
      //Serial.print(char(PET_serial.read()));
    }
    delay(10); //1000
    Serial.flush();
    return true;
  }
  return false;
}

bool PlasticDetector::measure(float * index){

  //lightON();
  //delay(500);
  //start_meas();

  //------------------ get raw data----------------------------------------------------------------------------------
  int indice_32bytes=0;
  int indice_values0=0;
  int lenght=0;
  float values0[201];
  //memset(values0, 0, 201);
  long unsigned int temp_bytes[4];

  //start scan
  PET_serial.write("Xm0,70\n"); //320, 70
  delay(29); 
  while(PET_serial.available()){
      char a=PET_serial.read();
      
      temp_bytes[indice_32bytes]=a;
      //Serial.print(a);

      indice_32bytes=indice_32bytes+1;
      if (indice_32bytes==4)
      {
        values0[indice_values0]=float((temp_bytes[3]<<32)|(temp_bytes[2]<<16)|(temp_bytes[1]<<8)|(temp_bytes[0]));
        indice_values0=indice_values0+1;
        indice_32bytes=0;
      }
      
      lenght=lenght+1;

  }

  ESP_LOGI(PETTAG, "Message length: %i", lenght);

  //----------------------------------------------------------------------------------------------------------------
  //-------------- turn off light ----------------------------------------------------------------------------------
  lightOFF();

  //----------------------------------------------------------------------------------------------------------------
  //-------------- process data, 1Der, norma, index-----------------------------------------------------------------
  if (lenght==256) // si no son 800, error de medida no computar, 256
  {
    // calculo del gradiente
    float gradient[201];
    for(int j=0; j<64; j++) {
        int j_left = j - 1;
        int j_right = j + 1;
        if (j_left < 0) {
            j_left = 0; // use your own boundary handler
            j_right = 1;
        }
        if (j_right >= sizeof(values0)){
            j_right = sizeof(values0) - 1;
            j_left = j_right - 1;
        }
        // gradient value at position j
        double dist_grad = (values0[j_right] - values0[j_left]) / 2.0;
        gradient[j]=dist_grad;
    }
    
    // normalizer
    float norm[201];
    for(int j=0; j<64; j++) {
      norm[j] = gradient[j] / 200;
    }
    float PET=norm[51]/norm[59];


    ESP_LOGI(PETTAG, "Pet index: %f", PET);

    memcpy(index, &PET, sizeof(PET));

    if (PET>1.1) //0.9
    {
        ESP_LOGI(PETTAG, "PET detected\n");
        return true;
    }
    else
    {
        ESP_LOGI(PETTAG, "No PET detected\n");
        return false;
    }
    
  }else
  {
    ESP_LOGW(PETTAG, "No measure\n");  
    return false;
  }
  
}
