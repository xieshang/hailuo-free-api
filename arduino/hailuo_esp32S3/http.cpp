#include "HTTPClient.h"
#include "Arduino.h"
#include "sdcard.h"


int http_post_audio_buff(String url, char* audio_data, uint32_t len, char* outfilename)
{
  HTTPClient http;

  http.setTimeout(30000);
  http.addHeader("Content-Type", "audio/wave");
  http.addHeader("Content-Length", String(len, DEC));
  http.addHeader("Authorization", "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJleHAiOjE3MjExMTUwNzMsInVzZXIiOnsiaWQiOiIyNDc0NTE1OTQ5NTc3NTQzNzMiLCJuYW1lIjoi5bCP6J665bi9NDM3MyIsImF2YXRhciI6Imh0dHBzOi8vY2RuLnlpbmdzaGktYWkuY29tL3Byb2QvdXNlcl9hdmF0YXIvMTcwNjI2NzU5ODg3NTg5OTk1My0xNzMxOTQ1NzA2Njg5NjU4OTZvdmVyc2l6ZS5wbmciLCJkZXZpY2VJRCI6IjI0NzQ1MTU5NDcxODY3OTA0NiIsImlzQW5vbnltb3VzIjpmYWxzZX19.40UhMhuGo9kr79fOL7lflCiDd10z2DyRIUELwNjjfb4");
  if (!http.begin(url))
  {
    Serial.println("\nfailed to begin http\n");
    return 0;
  }

  int http_code;


  http_code = http.sendRequest("POST", (uint8_t*)audio_data, len);

  Serial.println(http_code);
//   http.writeToStream(&Serial);
  //当前日期和时间作为文件名

  time_t now = time(nullptr);
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);
  sprintf(outfilename, "/voice/%d-%02d-%02d_%02d-%02d-%02d_bot.wav", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  Serial.printf("outfilename: %s\n", outfilename);
  File file = SD_MMC.open(outfilename, FILE_WRITE);
  http.writeToStream(&file);
  file.close();
  http.end();

  if (http_code > 0)
  {

    Serial.print("HTTP Response code: ");
    Serial.println(http_code);

    String response = http.getString();

    // Serial.println(response);
    return 1;
    
  }
  else
  {
    Serial.print('\n');
    Serial.print("Error code: ");
    Serial.println(http_code);
    return -1;
  }









  // EthernetClient client;
  // byte server[] = { 192, 168, 1, 100 };
    
  // if (client.connect(server, 5000)) {
  //   // 发送HTTP POST请求
  //   client.println("POST /phone_msg?model=hailuo&response_format=json HTTP/1.1");
  //   client.println("Host: 192.168.1.42");
  //   client.println("Content-Type: audio/wave");  // 根据实际音频类型设置Content-Type
  //   String content_length = "Content-Length: " + String(len);
  //   client.println(content_length);  // 每个分片的长度

  //   // 发送音频文件分片
  //   client.println(audio_data);  // 发送分片数据

  //   client.stop();
  //   return 1;
  // }
  return 1;
}
