/*!
 * @file wifiServer.cpp
 *
 * https://m1cr0lab-esp32.github.io/remote-control-with-websocket/wifi-setup/
 */
#include "wifiServer.h"

// Globals
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

uint32_t client_id = 1;

Logger logger;

WifiServer::WifiServer(){
}

String uint8ToString(uint8_t val){
  char cval[32];
  dtostrf(val, 6, 0, cval);
  return String(cval);
}

String uint32ToString(uint32_t val){
  char cval[32];
  dtostrf(val, 6, 0, cval);
  return String(cval);
}

void SD_file_delete(AsyncWebServerRequest *request){
  int paramsNr = request->params();
  for(int i=0;i<paramsNr;i++){

    AsyncWebParameter* p = request->getParam(i);
 
     Serial.print("Param name: ");
     Serial.println(p->name());
 
     Serial.print("Param value: ");
     Serial.println(p->value());

    char igcf[40];
    p->value().toCharArray(igcf,40);
//    logger.deleteFile(SD, igcf);
#ifdef LILYGO_S3_E_PAPER_V_1_0
    logger.deleteFile(SD, igcf);
#else
    logger.deleteFile(SD_MMC, igcf);
#endif 
  }
}

void SD_file_download(AsyncWebServerRequest *request){

  int paramsNr = request->params();
  for(int i=0;i<paramsNr;i++){

    AsyncWebParameter* p = request->getParam(i);
 
     Serial.print("Param name: ");
     Serial.println(p->name());
 
     Serial.print("Param value: ");
     Serial.println(p->value());
 
    if(p->name()=="igc"){
#ifdef LILYGO_S3_E_PAPER_V_1_0
      File download = SD.open(p->value());
      if (download) {
        request->send(SD, p->value(), "text/text", true);
        return;
      } 
#else
      File download = SD_MMC.open(p->value());
      if (download) {
        request->send(SD_MMC, p->value(), "text/text", true);
        return;
      } 
#endif
    }

    char subpath[64];
    strcpy(subpath,IGCFOLDER);
    strcat(subpath,"/");
    if(p->name()=="year"){
      char yyyy[5];
      p->value().toCharArray(yyyy,5,0);
      strcat(subpath, yyyy);
    }
    if(p->name()=="month"){
      strcat(subpath,"/");
      char mm[3];
      p->value().toCharArray(mm,3,0);
      strcat(subpath, mm);
    }
    if(p->name()=="day"){
      strcat(subpath,"/");
      char dd[3];
      p->value().toCharArray(dd,3,0);
      strcat(subpath, dd);
    }

  }
}

void export_settings(AsyncWebServerRequest *request){
//  TODO 
  request->send(SPIFFS, SETTINGS_FileName, String(), true);
}

String processor(const String& var)
{
  String stringReturn = "";

  if(var == "PLACEHOLDER"){
    return String("DUCCIO");
  }
  else if(var == "VERSION"){
    return VERSION;
  }
  else if(var == "DEVELOPERINFO"){
    return DEVELOPER;
  // }else if (var == "NEEDTORESTART"){
  //   return String(status.needToRestart);
  }else if(var == "BUILD_TIMESTAMP"){
    return compile_date;
  }else if(var == "IGCPATH"){
    return IGCFOLDER;
  }else if(var == "IGCFILELIST"){
        // TODO list all igc files and create link to download
    char igc_dir[32];
    stringReturn = "";
    bool getfiles = false;

//    if(String(status.folder_path) == "none"){
      strcpy(igc_dir,IGCFOLDER);
//    }else{
//      strcpy(igc_dir,status.folder_path);
//    }

    stringReturn += "<table><thead><tr><th>SD Folder: </th><th id='folder_path'></th></tr></thead><tbody>";

#ifdef LILYGO_S3_E_PAPER_V_1_0
    logger.listFiles(SD,igc_dir,false);
#else
    logger.listFiles(SD_MMC,igc_dir,false);
#endif
    char* d = strtok(logger.igclist, ";");

    while (d != NULL ) {
        //Serial.println (d);
        if((!String(d).startsWith("/._") )){//&& !String(d).startsWith("/test"))){
          stringReturn += "<tr><td><button class='button bsil'>";

          if(String(d).indexOf(".igc") > 0){
            stringReturn += "<a href='/download?igc="+String(d)+"' download class='igc_link'>";
          }else{
            stringReturn += "<a href='?folder="+String(d)+"' class='igc_link'>";
          }
          stringReturn += String(d)+"</a>";
          stringReturn += "</button></td>";
          if(String(d).indexOf(".igc") > 0){
            stringReturn += "<td><button title='Delete' class='button bred' type='button'>";
            stringReturn += "<a href='/deleteigc?igc="+String(d)+"' target='_self'>";
            stringReturn += " X </a></button></td>";
          }else{
            stringReturn += "<td></td>";
          }
          stringReturn += "</tr>";
        }
        d = strtok(NULL, ";");
    }
    stringReturn += "</tbody></table>";
    return stringReturn;
  }else if (var == "VARIO_CURVE"){
    return  "";//status.jsonSettings["vario_curve"];
  }
 
  return "";
}

// ----------------------------------------------------------------------------
// WebSocket initialization
// ----------------------------------------------------------------------------

void WifiServer::wifiNotifyClients(void){

    const uint16_t size = JSON_OBJECT_SIZE(150);
    StaticJsonDocument<size> json;

    json.clear();
    json["dev_id"] = setting.myDevId;
    json["run_time"] = String(status.gps.Time);

    char datetime[20];
    if (status.gps.Fix){
      sprintf(datetime,"%s %s", status.gps.Date, status.gps.Time);
    }else{
      strcpy(datetime,"GPS Locating...");
    }

    json["gps_time"] = datetime ? datetime : "";
    json["pilot_name"] = setting.PilotName ? setting.PilotName : "";
    json["wifi_psw"] = setting.wifi.password;
    json["wifi_timer"] = setting.wifi.tWifiStop;
    json["b_mv"] = status.battery.voltage;
    json["b_perc"] = status.battery.percent;
    json["gps_fix"] = status.gps.Fix;
    json["gps_sat"] = status.gps.NumSat ? status.gps.NumSat : 0;
    json["temp_c"] = status.vario.temp ? status.vario.temp : 0;
    json["spk_vol"] = setting.vario.volume;
    json["beep_when_f"] = setting.vario.BeepOnlyWhenFlying;
    json["sd_size"] = setting.sd_size;
    json["pres_hpa"] = status.vario.pressure ? status.vario.pressure : 0;
    json["gps_alt"] = status.gps.alt ? status.gps.alt : 0;
    json["baro_alt"] = status.vario.alt ? status.vario.alt : 0;
    json["ble_on"] = 1;//status.jsonSettings["ble_on"];
    json["wifi_countdown"] = 60;
    json["fly_min_v"] = 6;
    json["fly_min_t"] = 15;
    json["fly_stop_t"] = 60;
    json["gps_hz"] = setting.gps.Baud; //status.jsonSettings["gps_hz"];
    json["epaper"] = "GxEPD213bn"; //status.jsonSettings["epaper"];
    json["t_refresh"] = 300;//status.jsonSettings["t_refresh"];
    json["auto_switch_page"] = 0; //status.jsonSettings["auto_switch_page"];
    json["min_sat_av"] = 5; //status.jsonSettings["min_sat_av"];
    json["rotation"] = setting.displayRotation; // status.jsonSettings["rotation"];
    json["utc_offset"] = 0; //status.jsonSettings["utc_offset"];
    json["sink_on"] = setting.vario.sinkingThreshold;//status.jsonSettings["sink_on"];
    json["lift_on"] = setting.vario.climbingThreshold;//status.jsonSettings["lift_on"];
    json["kalman_e_mea"] = setting.vario.sigmaA;//status.jsonSettings["kalman_e_mea"];
    json["kalman_e_est"] = setting.vario.sigmaA;//status.jsonSettings["kalman_e_est"];
    json["kalman_q"] = setting.vario.sigmaP;//status.jsonSettings["kalman_q"];
    json["vario_avg_ms"] = 250;//status.jsonSettings["vario_avg_ms"];
    json["vario_avg_ms_b"] = 50; //status.jsonSettings["vario_avg_ms_b"];

    json["thermal_detect"] = 0; //status.jsonSettings["thermal_detect"];
    json["thermal_avg"] = 2000; //status.jsonSettings["thermal_avg"];

    json["fanet_rx"] = status.fanetRx;
    json["fanet_tx"] = status.fanetTx;
    json["leg_rx"] = status.legRx;
    json["leg_tx"] = status.legTx;

//    json["vario_curve"] = status.jsonSettings["vario_curve"];

    char data[2048];
    size_t len = serializeJson(json, data);

    if (ws.hasClient(client_id) && ws.availableForWrite(client_id)) {
      ws.text(client_id, data);
    }
  //  ws.textAll(data, len);

}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
      const uint16_t size = JSON_OBJECT_SIZE(100);
      StaticJsonDocument<size> jsonData;
      DeserializationError err = deserializeJson(jsonData, data);
      if (err) {
            Serial.print(F("deserializeJson() failed with code "));
            Serial.println(err.c_str());
            return;
      }
      char value_input[128]; 

      if (jsonData.containsKey("update")){
        if (jsonData["update"] == true) {

          if (jsonData.containsKey("pilot_name")) setting.PilotName = jsonData["pilot_name"].as<String>();
          if (jsonData.containsKey("utc_offset")) setting.utc_offset = jsonData["utc_offset"].as<int8_t>();
          if (jsonData.containsKey("volume")) setting.vario.volume = jsonData["volume"].as<uint8_t>();
          if (jsonData.containsKey("rotation")) setting.displayRotation = jsonData["rotation"].as<uint8_t>();
          //if (jsonData.containsKey("t_refresh")) ...
          if (jsonData.containsKey("beep_when_f")) setting.vario.BeepOnlyWhenFlying = jsonData["beep_when_f"].as<bool>();
          if (jsonData.containsKey("wifi_timer")) setting.wifi.tWifiStop = jsonData["wifi_timer"].as<uint32_t>();        
          //if (jsonData.containsKey("fly_min_v")) ...
          //if (jsonData.containsKey("fly_min_t")) ...
          //if (jsonData.containsKey("fly_stop_t")) ...
          //if (jsonData.containsKey("min_sat_av")) ...
          //if (jsonData.containsKey("ble_on")) ...

        }
      }

      // // check test speaker input
      // if (jsonData.containsKey("speaker")){
      //   strcpy(value_input,jsonData["speaker"]);
      //     if (strcmp(value_input, "test") == 0) {
      //         status.test_speaker = true;
      //         Serial.println("Testing speaker, you should hear a beep!?");              
      //     }else{
      //       Serial.println("Something went wrong!");
      //     }
      // }

      // check for restart
      if (jsonData.containsKey("restart")){
        if(jsonData["restart"] == true){
          ESP.restart();
        }
      }

      // check restart GPS TODO
      if (jsonData.containsKey("restart_gps")){
        if(jsonData["restart_gps"] == true){
//          status.restart_GPS = true;
            command.ConfigGPS = true;
        }
      }

      // check change GPS Hz TODO
      // if (jsonData.containsKey("gps_hz")){
      //   status.gps_hz = jsonData["gps_hz"];
      //   status.jsonSettings["gps_hz"] = jsonData["gps_hz"];
      //   Serial.print("Gps frq set to: Hz ");
      //   Serial.println(status.gps_hz);
      //   status.resetGpsHz = true;
      // }

      // // check vario test input TODO
      // if (jsonData.containsKey("test_vario")){
      //   status.test_vario = jsonData["test_vario"];
      // }
      // if (jsonData.containsKey("tv_vval")){
      //   status.tv_vval = jsonData["tv_vval"]; 
      // }
      // if (jsonData.containsKey("tv_frq")){
      //   status.tv_frq = jsonData["tv_frq"]; 
      // }
      // if (jsonData.containsKey("tv_ton")){
      //   status.tv_ton = jsonData["tv_ton"]; 
      // }
      // if (jsonData.containsKey("tv_toff")){
      //   status.tv_toff = jsonData["tv_toff"]; 
      // }

      // // check sink lift input
      // if(jsonData.containsKey("sink_on")){
      //   status.vario_sink_on = jsonData["sink_on"];
      //   status.jsonSettings["sink_on"] = jsonData["sink_on"];
      // }
      // if(jsonData.containsKey("lift_on")){
      //   status.vario_lift_on = jsonData["lift_on"];
      //   status.jsonSettings["lift_on"] = jsonData["lift_on"];
      // }

      // // check Kalman and average input
      // if(jsonData.containsKey("kalman_e_mea")){
      //   status.kalman_e_mea = jsonData["kalman_e_mea"];
      //   status.jsonSettings["kalman_e_mea"] = jsonData["kalman_e_mea"];
      //   status.update_kalman = true;
      // }
      // if(jsonData.containsKey("kalman_e_est")){
      //   status.kalman_e_est = jsonData["kalman_e_est"];
      //   status.jsonSettings["kalman_e_est"] = jsonData["kalman_e_est"];
      //   status.update_kalman = true;
      // }
      // if(jsonData.containsKey("kalman_q")){
      //   status.kalman_q = jsonData["kalman_q"];
      //   status.jsonSettings["kalman_q"] = jsonData["kalman_q"];
      //   status.update_kalman = true;
      // }
      // if(jsonData.containsKey("vario_avg_ms")){
      //   status.vario_avg_ms = jsonData["vario_avg_ms"];
      //   status.jsonSettings["vario_avg_ms"] = jsonData["vario_avg_ms"];
      // }
      // if(jsonData.containsKey("vario_avg_ms_b")){
      //   status.vario_avg_ms_b = jsonData["vario_avg_ms_b"];
      //   status.jsonSettings["vario_avg_ms_b"] = jsonData["vario_avg_ms_b"];
      // }

      // // check vario curve input
      // if (jsonData.containsKey("vario_curve")){
      //   status.jsonSettings.remove("vario_curve");
      //   JsonArray vario_curve = status.jsonSettings.createNestedArray("vario_curve");

      //   for (int i=0;i<sizeof(jsonData["vario_curve"]);i++){
      //       float vval = jsonData["vario_curve"][i]["vval"];
      //       uint32_t frq = jsonData["vario_curve"][i]["frq"];
      //       uint32_t ton = jsonData["vario_curve"][i]["ton"];
      //       uint32_t toff = jsonData["vario_curve"][i]["toff"];

      //       JsonObject vario_curve_x = vario_curve.createNestedObject();
      //       vario_curve_x["vval"] = vval;
      //       vario_curve_x["frq"] = frq;
      //       vario_curve_x["ton"] = ton;
      //       vario_curve_x["toff"] = toff;
      //   }
      // }

//       // check Thermal detect input
//       if (jsonData.containsKey("thermal_detect")){
//         bool tdb = jsonData["thermal_detect"];
//         status.jsonSettings["thermal_detect"] = tdb;
// //        status.thermal_detect = tdb;
//         Serial.print("Thermal detect: '");
//         Serial.print(tdb);
//         Serial.println("'");
//       }
//       if(jsonData.containsKey("thermal_avg")){
// //        status.thermal_avg = jsonData["thermal_avg"];
//         status.jsonSettings["thermal_avg"] = jsonData["thermal_avg"];
//       }

//      delay(100);

      // check if update input to save to SPIFFS setting
      if (jsonData.containsKey("update")){
          if (jsonData["update"] == true) {
              // Serial.println("Updating json SPIFFS setting.");         
              //   if (!SPIFFS.begin(true)) {
              //     Serial.println("An Error has occurred while mounting SPIFFS");
              //     return;
              //   }     
              //   // Open file for writing
              //   File file = SPIFFS.open(SETTINGS_FileName, FILE_WRITE);
              //   if (!file) {
              //     Serial.println(F("Failed to create file"));
              //     return;
              //   }
              //   // Serialize JSON to file
              //   if (serializeJson(status.jsonSettings, file) == 0) {
              //     Serial.println(F("Failed to write to file"));
              //   }              
              //   // Close the file
              //   file.close();
              //   Serial.print("Settings saved to ");
              //   Serial.println(SETTINGS_FileName); 

              // Overwrite preferences with modified setting
              log_i("write config-to file");
              write_configFile(&setting);

          }else{
            Serial.println("Something went wrong!");
          }
      }

    }
}

void onEvent(AsyncWebSocket       *server,
             AsyncWebSocketClient *client,
             AwsEventType          type,
             void                 *arg,
             uint8_t              *data,
             size_t                len) {
              
    switch (type) {
        case WS_EVT_CONNECT:
            log_i("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
            client_id = client->id();
            status.wifiSTA.state = CONNECTED;
            break;
        case WS_EVT_DISCONNECT:
            log_i("WebSocket client #%u disconnected\n", client->id());
            client_id = 0;
            status.wifiSTA.state = DISCONNECTED;
            break;
        case WS_EVT_DATA:
            handleWebSocketMessage(arg, data, len);
            break;
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            log_i("WS_EVT_ERROR: %s", data);
            break;
    }
}

// https://m1cr0lab-esp32.github.io/remote-control-with-websocket/websocket-setup/

void initWebSocket() {
    ws.onEvent(onEvent);
    server.addHandler(&ws);
}

void WifiServer::wifiCleanClients(void){
    ws.cleanupClients();
}

// ----------------------------------------------------------------------------
// WiFi Server initialization
// ----------------------------------------------------------------------------

void WifiServer::end(void){
  // to close softAP wifi
  WiFi.softAPdisconnect (true);
  WiFi.disconnect();

  // to close websocket
  server.end();

  WiFi.mode(WIFI_MODE_NULL);

  // force redraw of display
  //status.e_refresh = true;

  log_i("Wifi Server Off.");
//  Serial.println("Wifi Server Off.");
}

static void handle_update_progress_cb(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  uint32_t free_space = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
  if (!index){
    //Serial.print("Total bytes:    "); Serial.println(SPIFFS.totalBytes());
    //Serial.print("Used bytes:     "); Serial.println(SPIFFS.usedBytes());
    //Serial.println(filename);
    //Serial.println("Update");
    //log_i("stopping standard-task");
    //vTaskDelete(xHandleStandard); //delete standard-task    
    WebUpdateRunning = true;
    delay(500); //wait 1 second until tasks are stopped
    Serial.println("webupdate starting");      
    //Update.runAsync(true);
    if (filename.startsWith("spiffs")){
//      if (!Update.begin(0x30000,U_SPIFFS)) {
        if (!Update.begin(UPDATE_SIZE_UNKNOWN,U_SPIFFS)) {
          Update.printError(Serial);
        }
    }else{
      if (!Update.begin(free_space,U_FLASH)) {
        Update.printError(Serial);
      }
    }
  }
  //log_i("l=%d",len);
  if (Update.write(data, len) != len) {
    Update.printError(Serial);
  }

  if (final) {
    if (!Update.end(true)){
      Update.printError(Serial);
    } else {
      Serial.println("Update complete");      
      delay(1000);
      ESP.restart();
    }
  }
}

void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
  Serial.println(logmessage);

  if (!index) {
    logmessage = "Upload Start: " + String(filename);
    // open the file on first call and store the file handle in the request object
    request->_tempFile = SPIFFS.open(SETTINGS_FileName, "w");
    Serial.println(logmessage);
  }

  if (len) {
    // stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);
    logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
    Serial.println(logmessage);
  }

  if (final) {
    logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
    // close the file handle as the upload is now done
    request->_tempFile.close();
    Serial.println(logmessage);
    Serial.println("Overriding /setting.json");
    request->redirect("/setting");
  }
}

// Callback: send 404 if requested file does not exist
void onPageNotFound(AsyncWebServerRequest *request) {
  IPAddress remote_ip = request->client()->remoteIP();
  log_e("[%s] HTTP GET request of %s",remote_ip.toString().c_str(),request->url().c_str());
  request->send(404, "text/plain", "Not found");
}

bool WifiServer::begin(uint8_t type){

    const char* mypassword =  "12345678";

    log_i("switch WIFI ACCESS-POINT ON");
//    Serial.println("switch WIFI ACCESS-POINT ON");
    WiFi.disconnect(true,true);
    WiFi.mode(WIFI_OFF);
    WiFi.persistent(false);
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE,INADDR_NONE,INADDR_NONE);
    WiFi.mode(WIFI_MODE_AP);
    //WiFi.softAPConfig(local_IP, gateway, subnet);

    if(!SPIFFS.begin()){
     Serial.println("An Error has occurred while mounting SPIFFS");
     return false;
    }
    strcpy(myssid,"TzI-Wifi-");
    char mychipid[10];
    sprintf(mychipid, "%s", setting.myDevId.c_str()); 
    strcat(myssid,mychipid);
    delay(1);
    log_i("Wifi ssid: %s", myssid);

    WiFi.setHostname(myssid);

    // if (status.jsonSettings.containsKey("wifi_psw")){
    //   WiFi.softAP(myssid, status.jsonSettings["wifi_psw"]);
    // }else{
      WiFi.softAP(myssid, mypassword);
    // }

    initWebSocket();
  
  // Serial.print("IP: ");
  // Serial.println(WiFi.softAPIP());
  // WiFi.softAPIP().toString().toCharArray(status.wifiAPip,12,0);
 
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    //request->send(SPIFFS, "/index.html", "text/html", false, processor);
    request->send(SPIFFS, "/index.html", "text/html",false,processor);
  });

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/data.html", "text/html",false,processor);
  });

  server.on("/restart",HTTP_GET, [](AsyncWebServerRequest *request){
    ESP.restart();
  });

  server.on("/logger", HTTP_GET, [](AsyncWebServerRequest *request){
    
    //strcpy(status.folder_path,IGCFOLDER);
    int paramsNr = request->params();
    Serial.println(paramsNr);
    if (paramsNr){
      for(int i=0;i<paramsNr;i++){

        AsyncWebParameter* p = request->getParam(i);
    
        Serial.print("Param name: ");
        Serial.println(p->name());
    
        Serial.print("Param value: ");
        Serial.println(p->value());

        if (p->name()=="folder"){
//          strcpy(status.folder_path,p->value().c_str());
        }
      }

    }
    request->send(SPIFFS, "/logs.html", "text/html",false,processor);
  });

  server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request){
    SD_file_download(request);
  });

  server.on("/export-setting", HTTP_GET, [](AsyncWebServerRequest *request){
    export_settings(request);
  });

  // run handleUpload function when any file is uploaded
  server.on("/import-setting", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200);
      }, handleUpload);

  server.on("/deleteigc", HTTP_GET, [](AsyncWebServerRequest *request){
    SD_file_delete(request);
    request->redirect("/logger");    
  });

  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/settings.html", "text/html",false,processor);
  });

  server.on("/vario_curve", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/vario_curve.html", "text/html",false,processor);
  });

  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/update.html", "text/html",false,processor);
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, request->url(), "text/css");
  });

  server.on("/images/home.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, request->url(), "image/png");
  });


  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request){
      request->send(200);
    }, handle_update_progress_cb);

  // Handle requests for pages that do not exist
  server.onNotFound(onPageNotFound);
  
  server.serveStatic("/", SPIFFS, "/");
  server.begin();

  return true;
}
