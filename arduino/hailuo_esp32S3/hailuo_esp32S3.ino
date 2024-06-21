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
#include "usbmsc.h"
#include "Audio.h"

#define home  0

Audio audio;
HTTPClient http_client;

const char *ntpServer1 = "pool.ntp.org";
const char *ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;
const char *time_zone = "CST-8";  // TimeZone rule for Europe/Rome including daylight adjustment rules (optional)


// 1. Replace with your network credentials
#if home
  const char* ssid = "XSC-2.4";
  const char* password = "84940782";
#else
  const char* ssid = "RelianceTech";
  const char* password = "RelianceTech2019";
#endif

//按键0定义
#define BUTTON_PIN  0

uint64_t recording = 0;
char recording_file_path[50] = {0};
char recording_temp_file_path[] = "/voice/tmp_record.wav";
File record_file_handle, record_file_handle_temp;


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
  Serial.setDebugOutput(true);
  while(1)
  {
    if (!SD_MMC_Init())
    {
      Serial.println("SD begin failed");
      delay(10000);
    }else{
      Serial.println("SD Card init ok");
      createDir(SD_MMC, "/voice");
      break;
    }
  }
  I2S_Init(I2S_NUM_0, I2S_MODE_RX, I2S_BITS_PER_SAMPLE_16BIT, 1024);
  audio.setPinout(PIN_I2S_BCLK_HORN, PIN_I2S_WS_HORN, PIN_I2S_DOUT_HORN);
  audio.setVolume(21); // default 0...21
  Serial.printf("i2s prot: %d\n", audio.getI2sPort());
  // I2S_Init(I2S_NUM_1, I2S_MODE_TX, I2S_BITS_PER_SAMPLE_16BIT, 1024);
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

char* audio_buff = NULL;

void loop() {
  if(digitalRead(BUTTON_PIN)==LOW){
    delay(20);
    if(digitalRead(BUTTON_PIN)==LOW){
      if(recording == 0)
      {
        recording = millis();  //标记开始录音并生成文件名
        time_t now = time(nullptr);
        struct tm timeinfo;
        localtime_r(&now, &timeinfo);
        sprintf(recording_file_path, "/voice/%d-%02d-%02d_%02d-%02d-%02d_record.wav", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        deleteFile(SD, recording_temp_file_path);
        record_file_handle_temp = SD.open(recording_temp_file_path, FILE_WRITE);
        Serial.printf("start recording to %s\n", recording_file_path);
      }else{
        recording = 0;  //标记结束录音
        free(audio_buff);
        audio_buff = NULL;
        //临时文件转移到正式文件并加上wav头
        record_file_handle_temp.close();
        record_file_handle_temp = SD.open(recording_temp_file_path, FILE_READ);
        Serial.printf("%s size: %d\n", recording_temp_file_path, record_file_handle_temp.size());
        record_file_handle = SD.open(recording_file_path, FILE_WRITE);
        char header[44];
        CreateWavHeader(header, record_file_handle_temp.size());
        record_file_handle.write((const uint8_t*)header, sizeof(header));
        Serial.printf("write header ok, start copy data\n");
        uint8_t* buff = (uint8_t*)malloc(1024);
        while(record_file_handle_temp.available()){
          size_t sz = record_file_handle_temp.read(buff, sizeof(buff));
          record_file_handle.write(buff, sz);
          // Serial.printf("write data %d ok, left: %d\n", sz, record_file_handle_temp.available());
        }
        free(buff);
        buff = NULL;
        record_file_handle_temp.close();
        record_file_handle.close();
        
        remove(recording_temp_file_path);
        Serial.printf("end recording to %s\n", recording_file_path);
        Serial.printf("time: %d ms\n", millis() - recording);

        record_file_handle = SD.open(recording_file_path, FILE_READ, false);
        #if home
          if(http_post_audio_stream("http://192.168.1.42:5000/phone_msg?model=hailuo&response_format=json", &record_file_handle, record_file_handle.size(), recording_file_path))
        #else
          if(http_post_audio_stream("http://192.168.0.5:5000/phone_msg?model=hailuo&response_format=json", &record_file_handle, record_file_handle.size(), recording_file_path))
        #endif
        {
          Serial.printf("post result: %s\n", recording_file_path);
        }
        record_file_handle.close();
        recording = 0;  //标记结束录音
      }
    }
  }
}