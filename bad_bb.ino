#include <driver/i2s.h>
#include "driver/gpio.h"
#include "Arduino.h"
#include "WiFi.h"
#include <WiFiClient.h>
#include <WiFiServer.h>

//IPAddress receiverIP(192, 168, 55, 7); // IP address of the receiver ESP32
IPAddress receiverIP(192, 168, 55, 226);
const int receiverPort = 8888;
WiFiClient client;

// Connections to INMP441 I2S microphone
#define I2S_WS 27
#define I2S_SD 32
#define I2S_SCK 33
 

#define I2S_DOUT  22
#define I2S_BCLK  26
#define I2S_LRC   25

// Use I2S Processor 0
#define I2S_PORT_0 I2S_NUM_0
#define I2S_PORT_1 I2S_NUM_1

// Define input buffer length
#define bufferLen 64//256//64 //1024
#define bufferCnt 8//8 //8 //64
int16_t sBuffer[bufferLen];
 
String ssid =    "brr";
String password = "Rohit@123";
/*
void i2s_install_0() {
  // Set up I2S Processor configuration
  const i2s_config_t i2s_config0 = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 44100,
    .bits_per_sample = i2s_bits_per_sample_t(16),
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, //I2S_CHANNEL_FMT_RIGHT_LEFT
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S), 
    .intr_alloc_flags = 0,
    .dma_buf_count = bufferCnt,
    .dma_buf_len = bufferLen,
    .use_apll = false
  };
 
  i2s_driver_install(I2S_PORT_0, &i2s_config0, 0, NULL);
}
 
void i2s_setpin_0() {
  // Set I2S pin configuration
  const i2s_pin_config_t pin_config0 = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = -1,
    .data_in_num = I2S_SD
  };
 
  i2s_set_pin(I2S_PORT_0, &pin_config0);
}
*/

/*void i2s_install_1(){
  i2s_config_t i2s_config1 = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 44100,
    .bits_per_sample = i2s_bits_per_sample_t(16),
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
    .tx_desc_auto_clear = false,
    .dma_buf_count = bufferCnt,
    .dma_buf_len = bufferLen,
    .use_apll = false,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1  // Interrupt level 1, default 0
    };
    i2s_driver_install(I2S_PORT_1, &i2s_config1, 0, NULL);

}*/
void i2s_install_1(){
  i2s_config_t i2s_config1 = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 8000,//44100,
    .bits_per_sample = i2s_bits_per_sample_t(16),
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,//I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // Interrupt level 1, default 0
    .dma_buf_count = bufferCnt,
    .dma_buf_len = bufferLen,
    .use_apll = false,
    .tx_desc_auto_clear = false
  };
  i2s_driver_install(I2S_PORT_1, &i2s_config1, 0, NULL);
}

void i2s_setpin_1(){
  i2s_pin_config_t pin_config1 = {
    .bck_io_num = I2S_BCLK,
    .ws_io_num = I2S_LRC,
    .data_out_num = I2S_DOUT,
    .data_in_num = I2S_PIN_NO_CHANGE
    };
    i2s_set_pin(I2S_PORT_1, &pin_config1);
}

TaskHandle_t Task1;
TaskHandle_t Task2;

// LED pins


void setup() {
    Serial.begin(9600); 
   // Setup WiFi in Station mode
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
  
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  
    // WiFi Connected, print IP to serial monitor
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("");

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  //xTaskCreatePinnedToCore(Task1code,"Task1",10000,NULL,1,&Task1,0);                        
  //delay(500); 

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(Task2code,"Task2",10000,NULL,1,&Task2,0);          
    delay(500); 


  //i2s_install_0();
  //i2s_setpin_0();
  //i2s_start(I2S_PORT_0);
  //delay(500);
  i2s_install_1();
  i2s_setpin_1();
  i2s_start(I2S_PORT_1);


   
}
/*
//Task1code: get audio from microphone nd send to socket
void Task1code( void * pvParameters ){
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());
  
  for(;;){
    if (!client.connect(receiverIP, receiverPort)) {
      Serial.println("Sender: Connection failed");
      delay(1000);
      continue;
    }
    size_t bytesIn = 0;
    esp_err_t result = i2s_read(I2S_PORT_0, &sBuffer, bufferLen, &bytesIn, portMAX_DELAY);
    if (result == ESP_OK)
    {
      

      Serial.println("Sender: Sending data...");
      client.write((uint8_t *)sBuffer, bytesIn * sizeof(int16_t));//client.write((int16_t *)sBuffer, sizeof(sBuffer));
      Serial.println("Sender: Data sent");
    }
  } 
}
*/

//Task2code: receive audio via socket nd send to speaker
void Task2code( void * pvParameters ){
  
  uint32_t v32bit;
  size_t BytesWritten;
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());

  WiFiServer server(receiverPort);
  WiFiClient client;

  server.begin();
  Serial.println("Receiver: Server started");
  
  //uint8_t xBuffer[bufferLen * sizeof(int16_t)];//int16_t xBuffer[bufferLen];
  uint8_t xBuffer[sizeof(int16_t) * bufferLen];


  for(;;){
    client = server.available();
    if (client) {
      //Serial.println("Receiver: Client connected");
      
      size_t bytesRead = client.readBytes(xBuffer, sizeof(xBuffer));
      
      // Interpret the uint8_t buffer as int16_t array
      int16_t* intData = reinterpret_cast<int16_t*>(xBuffer);
      // Process or use the received int16_t values here
      
      for (size_t i = 0; i < bytesRead / sizeof(int16_t); i++) {
        intData[i] *= 10;

        // Make sure the value does not exceed the maximum and minimum limits
        if (intData[i] > INT16_MAX) {
          intData[i] = INT16_MAX;
        } else if (intData[i] < INT16_MIN) {
          intData[i] = INT16_MIN;
        }
        Serial.print(intData[i]);
        
        Serial.print(" ");
        v32bit = (intData[i] << 16) | intData[i];
        v32bit=intData[i];
        esp_err_t result_n = i2s_write(I2S_PORT_1, &v32bit, 4, &BytesWritten, portMAX_DELAY);
      }
     /*if (client) {
      Serial.println("Receiver: Client connected");
      size_t bytesRead = client.read((int16_t *)xBuffer, sizeof(xBuffer));
      for (size_t i = 0; i < bytesRead; i += sizeof(int16_t)) {
        int16_t value = *reinterpret_cast<int16_t*>(xBuffer + i);
        // Process or use the received int16_t value here
        v32bit = (value << 16) | value;
        esp_err_t result_n = i2s_write(I2S_PORT_1, &v32bit, 4, &BytesWritten, portMAX_DELAY);
      }*/
      /*for (int i=0;i<bufferLen;i++){
        v32bit = ( xBuffer[i]<< 16) | xBuffer[i];
        esp_err_t result_n = i2s_write(I2S_PORT_1,&v32bit,4,&BytesWritten,portMAX_DELAY);
      }*/
      Serial.println(" ");
      client.stop();
      //Serial.println("Receiver: Client disconnected");
     }
   
  }
}

void loop() {
  
}
