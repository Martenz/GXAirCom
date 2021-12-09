/*!
 * @file ScreenTFT.h
 *
 *
 */

extern struct SettingsData setting;
extern struct statusData status;


#ifndef __SCREENTFT_H__
#define __SCREENTFT_H__

#include <Arduino.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include <bl.h>
#include <icons.h>
#include "main.h"

#define TFT_WIDTH           240
#define TFT_HEIGHT          240
#define TFT_MISO            (gpio_num_t)0
#define TFT_MOSI            (gpio_num_t)19
#define TFT_SCLK            (gpio_num_t)18
#define TFT_CS              (gpio_num_t)5
#define TFT_DC              (gpio_num_t)27
#define TFT_RST             (gpio_num_t)-1
#define TFT_BL              (gpio_num_t)12

#define DRV 0x7789 // 0x9A01 
#define FREQ 27000000 // 40000000  

class ScreenTFT {
public:
  ScreenTFT(); //constructor
  TFT_eSPI *tft = NULL;
  BackLight *bl = NULL;
  bool begin();
  void end(void);
  void run(void); //has to be called cyclic

private:
  bool bInit;
  uint8_t blLevel;
  char vol[5];
  char myid[10];
  void doInitScreenTFT(void);
  void drawHeader(void);
  void updateScreenTFT(void);
  void writeMssg(uint8_t y, char* label, char* msg);
};

#endif

