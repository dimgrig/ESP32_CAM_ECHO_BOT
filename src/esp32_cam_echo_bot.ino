#include "secure.h"

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





/*
#define HOST "dg-telegram-bot-2.herokuapp.com"
#define SSL_PORT 443


typedef bool (*MoreDataAvailable)();
typedef byte (*GetNextByte)();

bool _debug = true;

#define HANDLE_MESSAGES 1
struct telegramMessage {
  String text;
  String chat_id;
  String chat_title;
  String from_id;
  String from_name;
  String date;
  String type;
  float longitude;
  float latitude;
  int update_id;
};
telegramMessage messages[HANDLE_MESSAGES];
int last_message_received = 1;
*/


bool isMoreDataAvailable();
byte getNextByte();

void readFile(fs::FS &fs, String path, String chat_id);
String writeFile(fs::FS &fs);



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
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());




  camera_init();
  SD_init();













/*


  if (!client.connected()) {
      Serial.println(F("[BOT]Connecting to server"));
    if (!client.connect(HOST, SSL_PORT)) {

        Serial.println(F("[BOT]Conection error"));
    } else {

      Serial.println(F("[BOT]connected to server"));
    }
  }


  //String command = "bot" + "611081761:AAFbNKvK1EcOeGacpb2BjicNlWh7tMNVkaI" + "/GetUpdates";
  client.println("GET /bot611081761:AAFbNKvK1EcOeGacpb2BjicNlWh7tMNVkaI/GetUpdates?offset=" + 
    String(last_message_received) + " HTTP/1.1");
  client.println("Host: dg-telegram-bot-2.herokuapp.com");
  client.println("User-Agent: ESP");
  client.println("Accept: * / *");
  client.println("Cache-Control: no-cache");
  client.println("");
  client.println("");

  String mess = "";
  long now;
  bool avail;
  char c;
  int ch_count = 0;

  now = millis();
  avail = false;
  int waitForResponse = 10000;
  const int maxMessageLength = 2000;
  while (millis() - now < waitForResponse) {
    while (client.available()) {
      char c = client.read();
      // Serial.write(c);
      if (ch_count < maxMessageLength) {
        mess = mess + c;
        ch_count++;
      }
      avail = true;
    }
    if (avail) {
        Serial.println();
        Serial.println(mess);
        Serial.println();
      break;
    }
  }

  */
  

/*
  Serial.println("GET /bot611081761:AAFbNKvK1EcOeGacpb2BjicNlWh7tMNVkaI/GetUpdates HTTP/1.1");
  Serial.println("Host: dg-telegram-bot-2.herokuapp.com");
  Serial.println("User-Agent: ESP");
  Serial.println("Accept: * / *"); !!!!!!!!
  Serial.println("Cache-Control: no-cache");
  Serial.println("");
  Serial.println("");
*/

/*
  {"ok":true,"result":[{"update_id":1372570,
"message":{"message_id":1168,"from":{"id":199220133,"is_bot":false,"first_name":
"Dmitriy","last_name":"Grigoriev","language_code":"ru"},"chat":{"id":199220133,"
first_name":"Dmitriy","last_name":"Grigoriev","type":"private"},"date":157323807
9,"text":"Hhhghnmjhbb"}}]}
*/



/*
    DynamicJsonBuffer jsonBuffer;
    String response = mess.substring(mess.indexOf('{'));
    //Serial.println(response);
    JsonObject &root = jsonBuffer.parseObject(response);
    Serial.println("JSON");
    root.printTo(Serial);
    if (root.success()) {
      if (root.containsKey("result")) {
        int resultArrayLength = root["result"].size();
        if (resultArrayLength > 0) {
          int newMessageIndex = 0;
          // Step through all results
          for (int i = 0; i < resultArrayLength; i++) {
            JsonObject &result = root["result"][i];
            if (processResult(result, newMessageIndex)) {
              newMessageIndex++;
            }
          }
          // We will keep the client open because there may be a response to be
          // given
        } else {
          if (_debug)
            Serial.println(F("[Bot]no new messages"));
        }
      } else {
        if (_debug)
          Serial.println(F("[Bot]Response contained no 'result'"));
      }
    } else { // Parsing failed
      if (response.length() < 2) { // Too short a message. Maybe connection issue
        if (_debug)
          Serial.println(F("[Bot]Parsing error: Message too short"));
      } else {
        // Buffer may not be big enough, increase buffer or reduce max number of
        // messages
        if (_debug)
          Serial.println(F("[Bot]Failed to parse update, the message could be too "
                           "big for the buffer"));
      }
    }

Serial.println(messages[0].text);
*/

/*
if (messages[0].text == "Hhhghnmjhbb") {
  Serial.println("Photo");
  String path = writeFile(SD_MMC);
  Serial.println(path);
  readFile(SD_MMC, path); //"/picture1.jpg"
}
*/






  Serial.print("Done");
}

void loop() {



    if (millis() > Bot_lasttime + Bot_mtbs)  {
      int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

      while(numNewMessages) {
        Serial.println("got response");
        for (int i=0; i<numNewMessages; i++) {
          if ((bot.messages[i].text == "image") || (bot.messages[i].text == "Image")) {
            Serial.println("Photo");
            String path = writeFile(SD_MMC);
            Serial.println(path);
            readFile(SD_MMC, path, bot.messages[i].chat_id);
          } else {
            bot.sendMessage(bot.messages[i].chat_id, bot.messages[i].text, "");  
          }
          
        }
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      }

      Bot_lasttime = millis();
    }

}

/*
bool processResult(JsonObject &result, int messageIndex) {
  int update_id = result["update_id"];
  // Check have we already dealt with this message (this shouldn't happen!)
  if (last_message_received != update_id) {
    last_message_received = update_id;
    messages[messageIndex].update_id = update_id;

    messages[messageIndex].text = F("");
    messages[messageIndex].from_id = F("");
    messages[messageIndex].from_name = F("");
    messages[messageIndex].longitude = 0;
    messages[messageIndex].latitude = 0;

    if (result.containsKey("message")) {
      JsonObject &message = result["message"];
      messages[messageIndex].type = F("message");
      messages[messageIndex].from_id = message["from"]["id"].as<String>();
      messages[messageIndex].from_name =
          message["from"]["first_name"].as<String>();

      messages[messageIndex].date = message["date"].as<String>();
      messages[messageIndex].chat_id = message["chat"]["id"].as<String>();
      messages[messageIndex].chat_title = message["chat"]["title"].as<String>();

      if (message.containsKey("text")) {
        messages[messageIndex].text = message["text"].as<String>();

      } else if (message.containsKey("location")) {
        messages[messageIndex].longitude =
            message["location"]["longitude"].as<float>();
        messages[messageIndex].latitude =
            message["location"]["latitude"].as<float>();
      }
    } else if (result.containsKey("channel_post")) {
      JsonObject &message = result["channel_post"];
      messages[messageIndex].type = F("channel_post");

      messages[messageIndex].text = message["text"].as<String>();
      messages[messageIndex].date = message["date"].as<String>();
      messages[messageIndex].chat_id = message["chat"]["id"].as<String>();
      messages[messageIndex].chat_title = message["chat"]["title"].as<String>();

    } else if (result.containsKey("callback_query")) {
      JsonObject &message = result["callback_query"];
      messages[messageIndex].type = F("callback_query");
      messages[messageIndex].from_id = message["from"]["id"].as<String>();
      messages[messageIndex].from_name =
          message["from"]["first_name"].as<String>();

      messages[messageIndex].text = message["data"].as<String>();
      messages[messageIndex].date = message["date"].as<String>();
      messages[messageIndex].chat_id =
          message["message"]["chat"]["id"].as<String>();
      messages[messageIndex].chat_title = F("");
    } else if (result.containsKey("edited_message")) {
      JsonObject &message = result["edited_message"];
      messages[messageIndex].type = F("edited_message");
      messages[messageIndex].from_id = message["from"]["id"].as<String>();
      messages[messageIndex].from_name =
          message["from"]["first_name"].as<String>();

      messages[messageIndex].date = message["date"].as<String>();
      messages[messageIndex].chat_id = message["chat"]["id"].as<String>();
      messages[messageIndex].chat_title = message["chat"]["title"].as<String>();

      if (message.containsKey("text")) {
        messages[messageIndex].text = message["text"].as<String>();

      } else if (message.containsKey("location")) {
        messages[messageIndex].longitude =
            message["location"]["longitude"].as<float>();
        messages[messageIndex].latitude =
            message["location"]["latitude"].as<float>();
      }
    }

    return true;
  }
  return false;
}
*/


//Read a file in SD card
void readFile(fs::FS &fs, String path, String chat_id){

    Serial.printf("Reading file: %s\n", path);

    file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    //Content type for PNG image/png
    String sent = bot.sendPhotoByBinary(chat_id, "image/jpeg", file.size(),
        isMoreDataAvailable,
        getNextByte);

    if (sent) {
      Serial.println("was successfully sent");
    } else {
      Serial.println("was not sent");
    }

    //while(file.available()){
    //   Serial.write(file.read());
    //}
}


String writeFile(fs::FS &fs){
  camera_fb_t * fb = NULL;
  
  // Сделать снимок с помощью камеры
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
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
  Serial.printf("Picture file name: %s\n", path.c_str()); 
  
  File file = fs.open(path.c_str(), FILE_WRITE); 
  if(!file){
    Serial.println("Failed to open file in writing mode");
  } 
  else {
    file.write(fb->buf, fb->len); 
    Serial.printf("Saved file to path: %s\n", path.c_str());
    EEPROM.write(0, pictureNumber);
    EEPROM.commit();
  }
  file.close();
  esp_camera_fb_return(fb); 
  Serial.printf("Saved");

  return path;
}


bool isMoreDataAvailable(){
  return file.available();
}

byte getNextByte(){
  return file.read();
}

/*
String sendPhotoByBinary(
    String chat_id, String contentType, int fileSize,
    MoreDataAvailable moreDataAvailableCallback,
    GetNextByte getNextByteCallback) {

  if (_debug)
    Serial.println("SEND Photo");

  String response = sendMultipartFormDataToTelegram(
      "sendPhoto", "photo", "img.jpg", contentType, chat_id, fileSize,
      moreDataAvailableCallback, getNextByteCallback);

  if (_debug)
    Serial.println(response);

  return response;
}
*/

/*

String sendMultipartFormDataToTelegram(
    String command, String binaryProperyName, String fileName,
    String contentType, String chat_id, int fileSize,
    MoreDataAvailable moreDataAvailableCallback,
    GetNextByte getNextByteCallback) {

  String body = "";
  String headers = "";
  long now;
  bool responseReceived;
  String boundry = F("------------------------b8f610217e83e29b");

  // Connect with api.telegram.org if not already connected
  if (!client.connected()) {
    if (_debug)
      Serial.println(F("[BOT Client]Connecting to server"));
    if (!client.connect(HOST, SSL_PORT)) {
      if (_debug)
        Serial.println(F("[BOT Client]Conection error"));
    }
  }
  if (client.connected()) {

    String start_request = "";
    String end_request = "";

    start_request = start_request + "--" + boundry + "\r\n";
    start_request = start_request +
                    "content-disposition: form-data; name=\"chat_id\"" + "\r\n";
    start_request = start_request + "\r\n";
    start_request = start_request + chat_id + "\r\n";

    start_request = start_request + "--" + boundry + "\r\n";
    start_request = start_request + "content-disposition: form-data; name=\"" +
                    binaryProperyName + "\"; filename=\"" + fileName + "\"" +
                    "\r\n";
    start_request = start_request + "Content-Type: " + contentType + "\r\n";
    start_request = start_request + "\r\n";

    end_request = end_request + "\r\n";
    end_request = end_request + "--" + boundry + "--" + "\r\n";

    client.print("POST /bot611081761:AAFbNKvK1EcOeGacpb2BjicNlWh7tMNVkaI/" + command);
    client.println(F(" HTTP/1.1"));
    // Host header
    client.print(F("Host: "));
    client.println(HOST);
    client.println(F("User-Agent: arduino/1.0"));
    client.println(F("Accept: * / *"));

    int contentLength =
        fileSize + start_request.length() + end_request.length();
    if (_debug)
      Serial.println("Content-Length: " + String(contentLength));
    client.print("Content-Length: ");
    client.println(String(contentLength));
    client.println("Content-Type: multipart/form-data; boundary=" + boundry);
    client.println("");

    client.print(start_request);

    if (_debug)
      Serial.print(start_request);

    byte buffer[512];
    int count = 0;
    char ch;
    while (moreDataAvailableCallback()) {
      buffer[count] = getNextByteCallback();
      // client->write(ch);
      // Serial.write(ch);
      count++;
      if (count == 512) {
        // yield();
        if (_debug) {
          Serial.println(F("Sending full buffer"));
        }
        client.write((const uint8_t *)buffer, 512);
        count = 0;
      }
    }

    if (count > 0) {
      if (_debug) {
        Serial.println(F("Sending remaining buffer"));
      }
      client.write((const uint8_t *)buffer, count);
    }

    client.print(end_request);
    if (_debug)
      Serial.print(end_request);

    count = 0;
    int ch_count = 0;
    char c;
    now = millis();
    bool finishedHeaders = false;
    bool currentLineIsBlank = true;

    int waitForResponse = 10000;
    const int maxMessageLength = 2000;


    while (millis() - now < waitForResponse) {
      while (client.available()) {
        char c = client.read();
        responseReceived = true;

        if (!finishedHeaders) {
          if (currentLineIsBlank && c == '\n') {
            finishedHeaders = true;
          } else {
            headers = headers + c;
          }
        } else {
          if (ch_count < maxMessageLength) {
            body = body + c;
            ch_count++;
          }
        }

        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }

      if (responseReceived) {
        if (_debug) {
          Serial.println();
          Serial.println(body);
          Serial.println();
        }
        break;
      }
    }
  }

  return body;
}
*/


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
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}

void SD_init() {
 if(!SD_MMC.begin()){
      Serial.println("Card Mount Failed");
      return;
  }
  uint8_t cardType = SD_MMC.cardType();

  if(cardType == CARD_NONE){
      Serial.println("No SD_MMC card attached");
      return;
  }

  Serial.print("SD_MMC Card Type: ");
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
  Serial.printf("SD_MMC Card Size: %lluMB\n", cardSize);
}