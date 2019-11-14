#include "secure.h" // ssid, password and BOTtoken

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

#include "FS.h" 
#include "SD_MMC.h" 


#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "fd_forward.h"
#include "fr_forward.h"
           
#include "soc/soc.h"           
#include "soc/rtc_cntl_reg.h"  
#include "dl_lib.h"
#include "driver/rtc_io.h"
#include <EEPROM.h>            


#define EEPROM_SIZE 1


#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

int pictureNumber = 0;



bool isMoreDataAvailable();
byte getNextByte();

void readFile(fs::FS &fs, String path, String chat_id);
String writeFile(fs::FS &fs);
void sendFile(fs::FS &fs, String chat_id);

File file;

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int Bot_mtbs = 5000; //mean time between scan messages
long Bot_lasttime;   //last time messages' scan has been done


void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
  Serial.begin(115200);

  // Set WiFi to station mode and disconnect from an AP if it was Previously
  // connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Attempt to connect to Wifi network:
  Serial.print("[ESP]Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("[ESP]WiFi connected");
  Serial.print("[ESP]IP address: ");
  Serial.println(WiFi.localIP());


  camera_init();
  SD_init();

  Serial.print("[ESP] setup done");
}

void loop() {

    if (millis() > Bot_lasttime + Bot_mtbs)  {
      int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

      while(numNewMessages) {
        Serial.println("[BOT]got response");
        for (int i=0; i<numNewMessages; i++) {
          if ((bot.messages[i].text == "image") || (bot.messages[i].text == "Image")) {
            Serial.println("[ESP]Start send image");
            
            //with SD card
            //String path = writeFile(SD_MMC);
            //Serial.println([SD]path);
            //readFile(SD_MMC, path, bot.messages[i].chat_id);

            //without SD card
            sendFile(SD_MMC, bot.messages[i].chat_id);
          } else {
            bot.sendMessage(bot.messages[i].chat_id, bot.messages[i].text, "");  
          }
          
        }
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      }

      Bot_lasttime = millis();
    }

}


//Read a file in SD card
void readFile(fs::FS &fs, String path, String chat_id){

    Serial.printf("[SD]Reading file: %s\n", path);

    file = fs.open(path);
    if(!file){
        Serial.println("[SD]Failed to open file for reading");
        return;
    }

    Serial.print("[SD]Read from file: ");
    //Content type for PNG image/png
    String sent = bot.sendPhotoByBinary(chat_id, "image/jpeg", file.size(),
        isMoreDataAvailable,
        getNextByte);

    if (sent) {
      Serial.println("[BOT]Was successfully sent");
    } else {
      Serial.println("[BOT]Was not sent");
    }
}


String writeFile(fs::FS &fs){
  camera_fb_t * fb = NULL;
  
  // Сделать снимок с помощью камеры
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("[ESP]Camera capture failed");
    return "";
  }
  // Инициализация EEPROM
  EEPROM.begin(EEPROM_SIZE);
  pictureNumber = EEPROM.read(0) + 1;

  // Путь сохранения на флешке
  ////String str = "/picture" + String(pictureNumber) +".jpg\n";
  ////char char_array[str.length() + 1];
  ////str.toCharArray(char_array, str.length() + 1);

  String path = "/picture" + String(pictureNumber) +".jpg";

  //fs::FS &fs = SD_MMC; 
  Serial.printf("[SD]Picture file name: %s\n", path.c_str()); 
  
  File file = fs.open(path.c_str(), FILE_WRITE); 
  if(!file){
    Serial.println("[SD]Failed to open file in writing mode");
  } 
  else {
    file.write(fb->buf, fb->len); 
    Serial.printf("[SD]Saved file to path: %s\n", path.c_str());
    EEPROM.write(0, pictureNumber);
    EEPROM.commit();
  }
  file.close();
  esp_camera_fb_return(fb); 
  return path;
}


//Read a file in SD card
void sendFile(fs::FS &fs, String chat_id){

  camera_fb_t * fb = NULL;

  // Сделать снимок с помощью камеры
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("[ESP]Camera capture failed");
  } else {
    Serial.printf("[BOT]Sending file");

    String path = "/picture_to_bot.jpg";

    file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("[SD]Failed to open file in writing mode");
    } 
    else {
      file.write(fb->buf, fb->len); 
      Serial.printf("[SD]Saved file to path: %s\n", path.c_str());
      file.close();

      file = fs.open(path);
      if(!file){
          Serial.println("[SD]Failed to open file for reading");
      } else {
        //Content type for PNG image/png
        String sent = bot.sendPhotoByBinary(chat_id, "image/jpeg", file.size(),
            isMoreDataAvailable,
            getNextByte);

        if (sent) {
          Serial.println("[BOT]Was successfully sent");
        } else {
          Serial.println("[BOT]Was not sent");
        }

      file.close(); 
      }
    }
  }

  esp_camera_fb_return(fb);
}


bool isMoreDataAvailable(){
  return file.available();
}

byte getNextByte(){
  return file.read();
}


void camera_init(){
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
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA; 
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("[CAM]Camera init failed with error 0x%x", err);
    return;
  }
}

void SD_init() {
 if(!SD_MMC.begin()){
      Serial.println("[SD]Card Mount Failed");
      return;
  }
  uint8_t cardType = SD_MMC.cardType();

  if(cardType == CARD_NONE){
      Serial.println("[SD]No SD_MMC card attached");
      return;
  }

  Serial.print("[SD]SD_MMC Card Type: ");
  if(cardType == CARD_MMC){
      Serial.println("MMC");
  } else if(cardType == CARD_SD){
      Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
      Serial.println("SDHC");
  } else {
      Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
  Serial.printf("[SD]SD_MMC Card Size: %lluMB\n", cardSize);
}