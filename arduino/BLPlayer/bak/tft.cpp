#include "tft.h"

TFT_eSPI tft = TFT_eSPI();       // Invoke custom library

uint16_t offset_y = 3;
uint16_t offset_x = 3;
uint16_t word_high = 10;
uint16_t word_width = 5;
uint16_t line_word_num = 26;
String disp_buff[TFT_LINE_NUM]; // 4 lines
bool disp_need_refresh = false;


void tft_init() {
  //Set up the display
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(0, 0);
}


void tft_close() {
  int pins[] = {32, 33, 25, 26, 27};
  for(int i = 0; i < sizeof(pins)/sizeof(pins[0]); i++) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], LOW);
  }
}


void tft_print(uint16_t colom, uint16_t row, char* str, char clean_line, char fresh) {
//   Serial.printf("x:%d y:%d str:%s\n",colom,row,str);
  String disp_str = "";
  for(uint8_t i = 0; i < line_word_num; i++) {
    if(i < colom)
    {
        if(clean_line)
        {
            disp_str += " ";
        }else{
            disp_str += disp_buff[row][i];
        }
    }else{
        if(disp_str.length() >= line_word_num) {
            // Serial.printf("line:%d word:%d\n",row,disp_buff[row].length());
            Serial.println(disp_str);
            break;
        }
        if(i >= colom + strlen(str))
        {
            if(clean_line)
            {
                disp_str += " ";
            }
        }else{
            disp_str += str[i - colom];
        }
    }
  }
  disp_buff[row] = disp_str;
  disp_need_refresh = true;
  if(fresh) {
    disp_refresh();
  }
}

void disp_refresh() {
  if(disp_need_refresh) {
    disp_need_refresh = false;
    // tft.fillScreen(TFT_BLACK);
    // tft.setTextSize(2);
    // tft.setTextColor(TFT_WHITE);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    for (uint8_t i = 0; i < TFT_LINE_NUM; i++) {
        tft.setCursor(offset_x, offset_y + i * word_high);
        tft.println(disp_buff[i]);
    }
  }
}


