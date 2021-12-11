/*!
 * @file Logger.cpp
 *
 *
 */

#include "Logger.h"

Logger::Logger(){

}

bool Logger::begin(){
  lInit = false;
  lStop = true;
  ltest = false;

  strcpy(igcPath, IGCFOLDER);
  strcat(igcPath,"/test.igc");

  log_i("IGC - Initialization done.");
  uint8_t cardType = SD_MMC.cardType();

  if(cardType == CARD_NONE){
      log_i("No SD_MMC card attached");
      return false;
  }

  log_i("SD_MMC Card Type: ");
  if(cardType == CARD_MMC){
      log_i("MMC");
  } else if(cardType == CARD_SD){
      log_i("SDSC");
  } else if(cardType == CARD_SDHC){
      log_i("SDHC");
  } else {
      log_i("UNKNOWN");
  }

  uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
  log_i("SD_MMC Card Size: %10dMB", cardSize);

  createDir(SD_MMC, IGCFOLDER);

  char* testigc = igcHeaders();
  writeFile(SD_MMC, igcPath, testigc);
   char igc_dir[32];
   strcpy(igc_dir,IGCFOLDER);
   //strcat(igc_dir,"/");
   Serial.println("### IGC FOLDER ###");
   Serial.println(igc_dir);
  listFiles(SD_MMC,igc_dir);

  //sha256
  mbedtls_md_context_t ctx;
  mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

  return true;
};

void Logger::end(void){
  // close logger
  lInit = false;
    // stop track close igc
    if (!lStop){
      lStop = true;
      doStopLogger();
    }
};

void Logger::run(void){
  if (status.flying){
    gotflytime = millis();
  }
  // if Flying i.e. also got gps fix and not initialized
  // test TODO CHANGE HERE to start recording when flying
//  if (status.GPS_Date && !lInit){
  if ((status.flying || ltest) && !lInit && status.GPS_Date){
    lInit = true;
    lStop = false;
    // start new igc track with headers
    int8_t fnum = 0;
    char fnumc[3];
    while (1){
      char trackFile[64];
      strcpy(trackFile,"/gxAirLogs/");
      strcat(trackFile,status.GPS_Date);
      strcat(trackFile,"_");
      strcat(trackFile,itoa(fnum,fnumc,10));
      // TODO this will be withouth extension and will be added when closing the igc 
      //      to have .igc files downloadable only when closed properly with security line
      strcat(trackFile,".igc"); 
      if (SD_MMC.exists(trackFile)){
        log_i("File already exists: %s",trackFile);
        fnum+=1;
      }else{
        log_i("File to write: %s",trackFile);
        strcpy(igcPath,trackFile);
        doInitLogger(igcPath);
        // write only one log for 30 seconds (ignoring flying status)
        ltest = false;
        break;
      }
    }
  }
    // if initilaized and flying/not but gps fix and sat > 3
  if(lInit && ( status.flying || (status.GPS_Fix && status.GPS_NumSat > 3)) ){
    updateLogger();
  }

  // if not flying (or i.e. gps fix lost) for more than 30s
  if ( millis() - gotflytime > 30000 ){
    lInit = false;
    // stop track close igc
    if (!lStop){
      lStop = true;
      doStopLogger();
    }
  }

}

char * Logger::igcHeaders(){
  static char headers[500]; 
  strcpy(headers,IGC_ROW1);
  #ifdef APPNAME
    strcat(headers,APPNAME);
  #endif
  strcat(headers,"\r");
  strcat(headers,IGC_ROW2);
  strcat(headers,"\r");
  strcat(headers,IGC_ROW3);
  if (status.GPS_Date) strcat(headers,status.GPS_Date);
  strcat(headers,"\r");
  strcat(headers,IGC_ROW4);
  if (setting.PilotName) {
    char pname[100];
    setting.PilotName.toCharArray(pname,100);
    strcat(headers, pname);
  } 
  strcat(headers,"\r");
  strcat(headers,IGC_ROW5);
  strcat(headers,"\r");
  strcat(headers,IGC_ROW6);
  strcat(headers,"\r");
  strcat(headers,IGC_ROW7);
  strcat(headers,"\r");
  strcat(headers,IGC_ROW8);
  strcat(headers,"\r");
  strcat(headers,IGC_ROW9);
  strcat(headers,"\r");
  strcat(headers,IGC_ROW10);
  #ifdef VERSION
    strcat(headers,VERSION);
  #endif
  strcat(headers,"\r");
  strcat(headers,IGC_ROW11);
  strcat(headers,"\r");
  strcat(headers,IGC_ROW12);
  strcat(headers,"\r");
  strcat(headers,IGC_ROW13);
  strcat(headers,"\r");
  strcat(headers,IGC_ROW14);
  strcat(headers,"\r");
  strcat(headers,IGC_ROW15);
  strcat(headers,"\r");
  strcat(headers,IGC_ROW16);
  strcat(headers,"\r");
  strcat(headers,IGC_ROW17);
  strcat(headers,"\r");
  return headers;
}

void Logger::doInitLogger(const char * trackFile){
  
  //const char* headers = "...headers...\r";
  log_i("%s",trackFile);
  char* newigc = igcHeaders();
  g_time = 0;
  g_latlon = 0;
  g_baroalt = 0;
  g_gpsalt = 0;
  writeFile(SD_MMC, trackFile, newigc);
};

void Logger::updateLogger(void){

//   To cut to the chase a real IGC file could be as below (although most software will require additional header records):
// B1101355206343N00006198WA0058700558
// B1101455206259N00006295WA0059300556
// ...
// And to explain the basic record format, using commas to indicate the actual fields:
// B,110135,5206343N,00006198W,A,00587,00558
//
// B: record type is a basic tracklog record
// 110135: <time> tracklog entry was recorded at 11:01:35 i.e. just after 11am
// 5206343N: <lat> i.e. 52 degrees 06.343 minutes North
// 00006198E: <long> i.e. 000 degrees 06.198 minutes West
// A: <alt valid flag> confirming this record has a valid altitude value
// 00587: <altitude from pressure sensor>
// 00558: <altitude from GPS>

  static char row[256];

  strcpy(row,"B");
  strcat(row,status.GPS_Time);

  static char lat_d[10]; //withouth leading zero added directly to row
  static char lon_d[10]; //withouth leading zero added directly to row
  static char lat_m[10];
  static char lon_m[10];
  int ilat_d = (int)status.GPS_Lat;
  int ilat_m = (int)( round((status.GPS_Lat - ilat_d)*60.*1000) );
  int ilon_d = (int)status.GPS_Lon;
  int ilon_m = (int)( round((status.GPS_Lon - ilon_d)*60.*1000) );
  // latitude from eg. 45.xxx to 45,0.xxx*60*1000N 
  // Degrees
  if ( ilat_d < 10 ) strcat(row,"0");
  itoa(ilat_d,lat_d,10);
  strcat(row,lat_d);
  // Minutes
  if ( ilat_m < 10000) strcat(row,"0");
  if ( ilat_m < 1000) strcat(row,"0");
  if ( ilat_m < 100) strcat(row,"0");
  if ( ilat_m < 10) strcat(row,"0");
  itoa(ilat_m,lat_m,10);
  strcat(row,lat_m);
  strcat(row,"N");
  // longitude from eg. 8.xxx to 8,0.xxx*60*1000E 
  // Degrees
  if ( ilon_d < 100 ) strcat(row,"0");
  if ( ilon_d < 10 ) strcat(row,"0");
  itoa(ilon_d,lon_d,10);
  strcat(row,lon_d);
  // Minutes
  if ( ilon_m < 10000) strcat(row,"0");
  if ( ilon_m < 1000) strcat(row,"0");
  if ( ilon_m < 100) strcat(row,"0");
  if ( ilon_m < 10) strcat(row,"0");
  itoa(ilon_m,lon_m,10);
  strcat(row,lon_m);
  strcat(row,"E");

  // Altitude
  strcat(row,"A");
  // Altitude from vario
  char altvario[5];
  int ialtvario = (int)(round(status.GPS_alt));
  if (ialtvario < 10000) strcat(row,"0");
  if (ialtvario < 1000) strcat(row,"0");
  if (ialtvario < 100) strcat(row,"0");
  if (ialtvario < 10) strcat(row,"0");
  itoa(ialtvario,altvario,10);
  strcat(row,altvario);
  // ALtitude from GPS
  char altgps[5];
  int igpsvario = (int)(round(status.varioAlt));
  if (igpsvario < 10000) strcat(row,"0");
  if (igpsvario < 1000) strcat(row,"0");
  if (igpsvario < 100) strcat(row,"0");
  if (igpsvario < 10) strcat(row,"0");
  itoa(igpsvario,altgps,10);
  strcat(row,altgps);

  strcat(row,"\r");

  appendFile(SD_MMC, igcPath, row);

  // G update
  g_time += (int)status.GPS_Time[strlen(status.GPS_Time)-1];
  g_latlon += (int)lat_m[strlen(lat_m)-1];
  g_baroalt += (int)altvario[strlen(altvario)-1];
  g_gpsalt += (int)altgps[strlen(altgps)-1];

}

void Logger::doStopLogger(void){
  // add igc security line
  // close igc file and rename to .igc to be downloadable
  static char row[256];
  strcpy(row,"G");

  //TODO create the algorithm to write security based on data

  log_i("Closing igc file with security line G");
  appendFile(SD_MMC, igcPath, row);

}

void Logger::createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

void Logger::writeFile(fs::FS &fs, const char * path, const char * message){
    log_i("Writing file: %s", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        log_i("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        log_i("File written");
    } else {
        log_i("Write failed");
    }
}

void Logger::appendFile(fs::FS &fs, const char * path, const char * message){
    log_i("Appending to file: %s", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        log_i("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
//        log_i("Message appended");
    } else {
        log_i("Append failed");
    }
}

void Logger::deleteFile(fs::FS &fs, const char * path){
    log_i("Deleting file: %s", path);
    if(fs.remove(path)){
        log_i("File deleted");
    } else {
        log_i("Delete failed");
    }
}

void Logger::listFiles(fs::FS &fs, const char * dirname){
  // TODO List all files and push to server
  log_i("Listing igc files:");
  File root = fs.open(dirname);
      if(!root){
        log_e("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        log_i("Not a directory");
        return;
    }

    File file = root.openNextFile();
    
    strcpy(igclist,"");
    while(file){
        if(!file.isDirectory()){    
          if (  strstr(file.name(), ".igc") ){       
            log_i("  FILE: %s", file.name());
            log_i("  SIZE: %10d", file.size());
            strcat(igclist,file.name());
            strcat(igclist,";");
          }
        }
        file = root.openNextFile();
    }

    return;
}

//download igcfiles form server
//https://github.com/G6EJD/ESP32-8266-File-Download/blob/master/ESP_File_Download_v01.ino

// void Logger::initHash(void){
//   char *payload = "initPrivateKey";

//   const size_t payloadLength = strlen(payload);
   
//   mbedtls_md_init(&ctx);
//   mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
//   mbedtls_md_starts(&ctx);
//   mbedtls_md_update(&ctx, (const unsigned char *) payload, payloadLength);
//   Serial.println(payload);
// }

// void Logger::updateHash(char* payload){
//   Serial.println(payload);
//   const size_t payloadLength = strlen(payload);
//   mbedtls_md_update(&ctx, (const unsigned char *) payload, payloadLength);  
// }

// void Logger::closeHash(void){
//   mbedtls_md_finish(&ctx, shaResult);
//   mbedtls_md_free(&ctx);  

//   //Serial.print("Hash: ");
//   strcpy(gcodeRow,"G");
   
//   for(int i= 0; i< sizeof(shaResult); i++)
//   {
//   char str[3];
//   sprintf(str, "%02x", (int)shaResult[i]);
//   //Serial.print(str);
//   strcat(gcodeRow,str);
//   }
  
//   Serial.println("");
//   Serial.print("Hash char: ");
//   Serial.println(gcodeRow);
// }
