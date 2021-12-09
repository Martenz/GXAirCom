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
    blLevel = 200;

    return true;
};

void ScreenTFT::end(void){
  tft->fillScreen(TFT_BLACK);
  bInit = false;
}

void ScreenTFT::run(void){
  // Test
  if (!bInit){
    doInitScreenTFT();
    drawHeader();
  }else{
    //bl->on();
    //doInitScreenTFT();
    updateScreenTFT();
  }
}

void ScreenTFT::doInitScreenTFT(void){
    BackLight *newbl = new BackLight(TFT_BL);
    bl = newbl;
    bl->begin();
    bl->on();
    bl->adjust(blLevel);
    TFT_eSPI *newtft = new TFT_eSPI(TFT_WIDTH,TFT_HEIGHT);    
    tft = newtft;
    tft->setDriver(DRV,FREQ);
    tft->setPins(TFT_MOSI, TFT_MISO, TFT_SCLK, TFT_CS, TFT_DC);
    tft->setRotation(0);
    tft->init();
    tft->initDMA();

    bInit = true;
}

void ScreenTFT::drawHeader(void){
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

    tft->drawRoundRect(5,X_Logo_height+30,230,40,20,TFT_WHITE);
    tft->drawRoundRect(5,X_Logo_height+75,230,40,20,TFT_WHITE);
    tft->drawRoundRect(5,X_Logo_height+120,230,40,20,TFT_WHITE);

}

void ScreenTFT::updateScreenTFT(void){
//    tft->fillScreen(TFT_RED);
//    tft->fillRect(0,X_Logo_height+30,240,240 - (X_Logo_height+30),TFT_BLACK);
    if (setting.myDevId){
      strcpy(myid,"");
      setting.myDevId.toCharArray(myid,20,0);
      writeMssg(X_Logo_height+30,"ID",myid);
    }
    if(setting.vario.volume){
      uint8_t vvol = setting.vario.volume;
      strcpy(vol,"");
      itoa(vvol,vol,10);
      writeMssg(X_Logo_height+75,"Vol",vol);
    }
}

void ScreenTFT::writeMssg(uint8_t y, char* label, char* msg){
    tft->fillRect(20,y+5,200,35,TFT_BLACK);
    tft->setTextColor(TFT_GREEN);
    tft->setTextSize(2);
    tft->setCursor(25,y+15);
    tft->print(label);
    tft->print(":");
    tft->print(msg);
    //log_i("Label: %s   Msg: %s",label,msg);
}