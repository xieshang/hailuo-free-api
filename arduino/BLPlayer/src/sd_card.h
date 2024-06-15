#ifndef __SD_CARD_H__
#define __SD_CARD_H__

#include "FS.h"
#include "SD.h"
#include "SPI.h"

uint8_t SD_init(void);
void SD_shutdown(void);
void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
void createDir(fs::FS &fs, const char * path);
void removeDir(fs::FS &fs, const char * path);
uint8_t readFile(fs::FS &fs, const char * path, uint8_t* outbuff, int* outlen);
uint8_t writeFile(fs::FS &fs, const char * path, uint8_t* outbuff, int outlen);
void appendFile(fs::FS &fs, const char * path, const char * message);
void renameFile(fs::FS &fs, const char * path1, const char * path2);
void deleteFile(fs::FS &fs, const char * path);


#endif

