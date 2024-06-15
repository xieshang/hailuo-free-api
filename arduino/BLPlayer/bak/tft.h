#ifndef __TFT_H__
#define __TFT_H__


#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>

#define TFT_LINE_NUM 9


void tft_init();
void tft_close();
void tft_print(uint16_t colom, uint16_t row, char* str, char clean_line, char fresh);
void disp_refresh();
#endif
