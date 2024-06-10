#include "Arduino.h"
#include "Wav.h"
#include "I2S.h"
#include "sdcard.h"
#include <WiFi.h>
#include "HTTPClient.h"
#include "cJSON.h"
#include <ArduinoJson.h>
#include "http.h"
#include "esp_sntp.h"


HTTPClient http_client;

const char *ntpServer1 = "pool.ntp.org";
const char *ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;
const char *time_zone = "CET-1CEST,M3.5.0,M10.5.0/3";  // TimeZone rule for Europe/Rome including daylight adjustment rules (optional)


// 1. Replace with your network credentials
const char* ssid = "XSC-2.4";
const char* password = "84940782";
//按键0定义
#define BUTTON_PIN  0

const int record_time = 2;  // second
const char filename[] = "/sound.wav";

const int headerSize = 44;
const int waveDataSize = record_time * 88000;
const int numCommunicationData = 8000;
const int numPartWavData = numCommunicationData/4;
// char communicationData[numCommunicationData];
// char partWavData[numPartWavData];
char waveData[waveDataSize] = {0};
// Audio audio;

uint32_t audio_record(char* waveData_t, uint32_t waveDataSize_t)
{
  waveDataSize_t = I2S_Read(waveData_t, waveDataSize_t);
  return waveDataSize_t;
}

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("No time available (yet)");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

// Callback function (gets called when time adjusts via NTP)
void timeavailable(struct timeval *t) {
  Serial.println("Got time adjustment from NTP!");
  printLocalTime();
}


void setup() {
  Serial.begin(115200);
  Serial.println("System start,wait key");
  if (!SD_MMC_Init()) Serial.println("SD begin failed");
  Serial.println("SD Card init ok");
  I2S_Init(I2S_NUM_0, I2S_MODE_RX, I2S_BITS_PER_SAMPLE_16BIT, 1024);
  I2S_Init(I2S_NUM_1, I2S_MODE_TX, I2S_BITS_PER_SAMPLE_16BIT, 1024);
  Serial.println("I2S init ok");
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  uint8_t count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    count++;
    if (count >= 75) {
      Serial.printf("\r\n-- wifi connect fail! --");
      break;
    }
    vTaskDelay(200);
  }
  Serial.printf("\r\n-- wifi connect success! --\r\n");
  Serial.println(WiFi.localIP());
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  sntp_set_time_sync_notification_cb(timeavailable);
  esp_sntp_servermode_dhcp(1);  // (optional)
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);

}

void loop() {
  if(digitalRead(BUTTON_PIN)==LOW){
    delay(20);
    if(digitalRead(BUTTON_PIN)==LOW){
      File file;
      
      file = SD_MMC.open(filename, FILE_READ);
      
      #if 0
        if(file)
        {
          Serial.printf("%s is exsit.\n", filename);
          while(file.available())
          {
            char buff[1024] = {0};
            file.readBytes(buff, sizeof(buff));
            I2S_Write(buff, sizeof(buff));
          }
          file.close();
          return;
        }
      #else
        deleteFile(SD_MMC, filename);
      #endif

      char record_path[128];
      time_t now = time(nullptr);
      struct tm timeinfo;
      localtime_r(&now, &timeinfo);
      sprintf(record_path, "/voice/%d-%02d-%02d_%02d-%02d-%02d_record.mp3", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

      file = SD_MMC.open(record_path, FILE_WRITE);
      if (!file)
      {
        Serial.printf("sd open fail: %s\n", filename);
        return;
      }
      Serial.printf("sd open ok: %s\n", filename);
      uint32_t time1 = millis();
      int record_len = 0;
      record_len = audio_record(waveData + headerSize, waveDataSize);
      CreateWavHeader((byte*)waveData, record_len);
      
      file.write((const byte*)waveData, record_len);

      // for (int j = 0; j < waveDataSize/numPartWavData; ++j) {
      //   I2S_Read(communicationData, numCommunicationData);
      //   file.write((const byte*)communicationData, numCommunicationData);
      //   Serial.println(j);
      //   // for (int i = 0; i < numCommunicationData/8; ++i) {
      //   //   partWavData[2*i] = communicationData[8*i + 2];
      //   //   partWavData[2*i + 1] = communicationData[8*i + 3];
      //   // }
      //   // file.write((const byte*)partWavData, numPartWavData);
      // }
      file.close();
      Serial.println("finish");
      Serial.printf("time: %d ms\n", millis() - time1);
      
      char filename[128] = {0};
      if(http_post_audio_buff("http://192.168.1.42:5000/phone_msg?model=hailuo&response_format=json", waveData, record_len, filename))
      {
        Serial.printf("post success: %s\n", filename);
        File file = SD_MMC.open(filename, FILE_READ);
        if (file)
        {
          // 每次读1024字节
          char buff[1024] = {0};
          while (file.available()) {
            file.readBytes(buff, sizeof(buff));
            Serial.println(buff);
            I2S_Write(buff, sizeof(buff));
          }
          file.close();
        }
      }

      Serial.println("post finish");
    }
  }
}
