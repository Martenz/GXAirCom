extern struct statusData status;
extern struct SettingsData setting;

#ifndef __BEEPERTZI_H__
#define __BEEPERTZI_H__

#include <Arduino.h>
#include <ArduinoJson.h>
#include <main.h>

class Buzzer {
  public:
    Buzzer(); //constructor
    bool begin(uint8_t pins);
    void end(uint8_t spk_pin);
    void run(uint8_t spk_pin); //has to be called cyclic
  
  private:
    bool bzInit;
    bool von;
    uint32_t tsound;
    StaticJsonDocument<1024> vario_curve;
    uint32_t interpolate(StaticJsonDocument<1024> curves, float vval, const char *field);
    void sing(uint8_t spk_pin, int song);
  
  };

#endif