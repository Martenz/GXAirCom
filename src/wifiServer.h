/*!
 * @file wifiServer.h
 *
 *
 */

/*
extern struct SettingsData setting;
extern struct statusData status;
*/

extern struct statusData status;
extern struct SettingsData setting;
extern struct commandData command;

#ifndef __WIFISERVER_H__
#define __WIFISERVER_H__

#include <Arduino.h>
#include "WiFi.h"
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"
#include <ArduinoJson.h>
#include "fileOps.h"
#include <TimeLib.h>
#include <Update.h>
#include <Logger.h>
#include "FS.h"
#include "SD.h"

#include "main.h"

#define DEVELOPER "GxAirCom TzI"
#define SETTINGS_FileName "/settings.json"

// IPAddress local_IP(192,168,4,1);
// IPAddress gateway(0,0,0,0);
// IPAddress subnet(255,255,255,0);

extern String compile_date;

class WifiServer {
public:
  WifiServer(); //constructor
  bool begin(uint8_t type);
  void end(void);
  void wifiCleanClients(void);
  void wifiNotifyClients(void);

private:
    bool wInit;
    char myssid[32];
};


#endif