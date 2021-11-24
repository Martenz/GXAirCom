/*!
 * @file ScreenTFT.cpp
 *
 *
 */

#include "ScreenTFT.h"

ScreenTFT::ScreenTFT(){
}

bool ScreenTFT::begin(){

    bInit = false;
    blLevel = 250;
    bl = new BackLight(TFT_BL);

    bl->begin();
    bl->on();
    bl->adjust(blLevel);
    tft = new TFT_eSPI(TFT_WIDTH,TFT_HEIGHT);
    
    tft->setDriver(DRV,FREQ);
    tft->setPins(TFT_MOSI, TFT_MISO, TFT_SCLK, TFT_CS, TFT_DC);
    tft->setRotation(0);
    tft->init();
    tft->initDMA();
    tft->fillScreen(TFT_BLACK);
    tft->drawXBitmap(55,10,G_Logo_bits,G_Logo_width,G_Logo_height,TFT_WHITE);
    tft->drawXBitmap(50+G_Logo_width,10,X_Logo_bits,X_Logo_width,X_Logo_height,TFT_WHITE);
    tft->drawXBitmap(60 + 69,10+2,AirCom_Logo_bits,AirCom_Logo_width,AirCom_Logo_height,TFT_WHITE);
    tft->setTextSize(2);
    tft->setTextColor(TFT_GREEN);
    tft->setCursor(10,55);
    tft->print("T-Watch");
    tft->setCursor(60 + 85,55);
    tft->print(VERSION);
    tft->drawLine(0,X_Logo_height+20,240,X_Logo_height+20,TFT_YELLOW);

    bInit = true;
};

void ScreenTFT::end(void){

}

void ScreenTFT::run(void){
  // Test
  blLevel = (blLevel + 20 )%250;
  bl->adjust(blLevel);
}