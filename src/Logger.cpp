/*!
 * @file Logger.cpp
 *
 *
 */

#include "Logger.h"

Logger::Logger()
{
}

bool Logger::begin()
{
  lInit = false;
  lStop = true;
  ltest = false;

  igcDate[0] = 0;

  char headers[500];

  strcpy(igcPath, IGCFOLDER);
  strcat(igcPath, "/test.igc");

  log_i("IGC - Initialization done.");
  //   log_i("Card: %s", status.sdSize);
  delay(1);

  strcpy(headers, igcHeaders());
#ifdef LILYGO_S3_E_PAPER_V_1_0
  writeFile(SD, (const char *)igcPath, headers);
#else
  writeFile(SD_MMC, (const char *)igcPath, headers);
#endif
  //  strcpy(gcodeRow, shaOut);
  // Close Hash sha256
  char gfinalrow[79];
  strcpy(gfinalrow, "G");
  strcat(gfinalrow, gcodeRow);
  strcat(gfinalrow, "\r");

#ifdef LILYGO_S3_E_PAPER_V_1_0
  appendFile(SD, (const char *)igcPath, gfinalrow);
#else
  appendFile(SD_MMC, (const char *)igcPath, gfinalrow);
#endif

  char igc_dir[32];
  strcpy(igc_dir, IGCFOLDER);
  // Serial.println("###IGC FOLDER ###");
  // Serial.println(igc_dir);
  // Serial.println("SD Logger Init OK.");

  log_i("Igc folder: %s", (String)igc_dir);
  delay(1);

  return true;
};

void Logger::end(void)
{
  // close logger
  lInit = false;
  // stop track close igc
  if (!lStop)
  {
    lStop = true;
    doStopLogger();
  }
};

void Logger::run(void)
{
  if (status.flying)
  {
    gotflytime = millis();
  }
  // if Flying i.e. also got gps fix and not initialized
  // test TODO CHANGE HERE to start recording when flying
  //  if (status.GPS_Date && !lInit){
  if ((status.flying || ltest) && !lInit)
  {
    lInit = true;
    lStop = false;
    // start new igc track with headers
    int8_t fnum = 0;
    char fnumc[3];
    while (1)
    {
      char trackFile[64];
      strcpy(trackFile, status.gps.Date);
      strcat(trackFile, "_");
      strcat(trackFile, itoa(fnum, fnumc, 10));
      // TODO this will be withouth extension and will be added when closing the igc
      //      to have .igc files downloadable only when closed properly with security line
      strcat(trackFile, ".igc");
#ifdef LILYGO_S3_E_PAPER_V_1_0
      if (SD.exists(trackFile))
      {
#else
      if (SD_MMC.exists(trackFile))
      {
#endif
        log_i("File already exists: %s", trackFile);
        fnum += 1;
      }
      else
      {
        log_i("File to write: %s", trackFile);
        strcpy(igcPath, trackFile);
        doInitLogger(igcPath);
        // write only one log for 30 seconds (ignoring flying status)
        ltest = false;
        break;
      }
    }
  }
  // if initilaized and flying/not but gps fix and sat > min sat available
  //  if(lInit && ( status.flying || (status.GPS_Fix && status.GPS_sat > MIN_SAT_AVAILABLE)) ){
  if (lInit && (status.flying || (status.gps.NumSat > MIN_SAT_AVAILABLE)))
  {
    //      status.logging = true;
    updateLogger();
  }

  // if not flying (or i.e. gps fix lost) for more than 30s
  if (millis() - gotflytime > 60 * 1000)
  {
    lInit = false;
    // stop track close igc
    if (!lStop)
    {
      lStop = true;
      doStopLogger();
    }
  }
}

//  char * Logger::timeGPSchar(){
//    static char fullTime[8]={0};
//  //  strcpy(fullTime,"");

//    // if got a correct date i.e. with year
//    if (status.gps.Time){
//       status.gps.Time.toCharArray(fullTime,8);
//    }else{
//      strcpy(fullTime,"000000");
//    }

//    return fullTime;
//  }

//  char * Logger::pathDateGPSchar(const char* subpath){
//    // create a global variable for logger igc file name based on GPS datetime
//    //set today date
//    static char fullDate[36];
//    static char full_p[64];

//    strcpy(fullDate,"");
//    // if got a correct date i.e. with year
//    if (status.gps.Date.year()>0){
//      char day[3];
//      itoa(status.gps.Date.day(), day, 10);
//      if (status.gps.Date.day()<10) strcat(fullDate,"0");
//      strcat(fullDate, day);

//      char month[3];
//      itoa(status.gps.Date.month(), month, 10);
//      if (status.gps.Date.month()<10) strcat(fullDate,"0");
//      strcat(fullDate, month);

//      char year[3];
//      char fyear[5];
//      itoa(status.gps.Date.year() - 2000, year, 10);
//      itoa(status.gps.Date.year(), fyear, 10);
//      strcat(fullDate, year);

//      strcpy(igcDate, fullDate);

//      // check and create folders
//      strcpy(full_p,subpath);
//      strcat(full_p,"/");
//      strcat(full_p,fyear);
//  #ifdef LILYGO_S3_E_PAPER_V_1_0
//      if(!SD.exists(full_p)) SD.mkdir(full_p);
//      strcat(full_p,"/");
//      strcat(full_p,month);
//      if(!SD.exists(full_p)) SD.mkdir(full_p);
//      strcat(full_p,"/");
//      strcat(full_p,day);
//      if(!SD.exists(full_p)) SD.mkdir(full_p);
//      strcat(full_p,"/");
//      strcat(full_p,fullDate);
//  #else
//      if(!SD_MMC.exists(full_p)) SD_MMC.mkdir(full_p);
//      strcat(full_p,"/");
//      strcat(full_p,month);
//      if(!SD_MMC.exists(full_p)) SD_MMC.mkdir(full_p);
//      strcat(full_p,"/");
//      strcat(full_p,day);
//      if(!SD_MMC.exists(full_p)) SD_MMC.mkdir(full_p);
//      strcat(full_p,"/");
//      strcat(full_p,fullDate);
//  #endif

//    }
//    //Serial.println(full_p);
//    return full_p;
//  }

void Logger::updateHash(const char *row)
{
  // update hash - combine previous hash with new data
  char sha_comb[160];
  strcpy(sha_comb, gcodeRow);
  strcat(sha_comb, row);
  hash256(sha_comb);
  // update gcodeRow var only here and first time!
  strcpy(gcodeRow, shaOut);
}

char *Logger::igcHeaders()
{

  // Start New Hash sha256
  hash256(SHAPRIVATEKEY);
  // write computed first sha256 to gcodeRow var to init
  strcpy(gcodeRow, shaOut);

  static char headers[500];
  static char hfrows[128];
  strcpy(headers, IGC_ROW1);
#ifdef APPNAME
  strcat(headers, APPNAME);
#endif
  strcat(headers, "\r");
  strcat(headers, IGC_ROW2);
  updateHash(IGC_ROW2);
  strcat(headers, "\r");
  strcat(headers, IGC_ROW3);
  strcpy(hfrows, IGC_ROW3);
  if (status.gps.Date)
  {
    strcat(headers, status.gps.Date);
    strcat(hfrows, status.gps.Date);
  }
  updateHash(hfrows);
  strcat(headers, "\r");
  strcat(headers, IGC_ROW4);
  strcpy(hfrows, IGC_ROW4);
  if (setting.PilotName)
  {
    strcat(headers, setting.PilotName.c_str());
    strcat(hfrows, setting.PilotName.c_str());
  }
  updateHash(hfrows);
  strcat(headers, "\r");
  strcat(headers, IGC_ROW5);
  updateHash(IGC_ROW5);
  strcat(headers, "\r");
  strcat(headers, IGC_ROW6);
  updateHash(IGC_ROW6);
  strcat(headers, "\r");
  strcat(headers, IGC_ROW7);
  strcat(headers, "\r");
  strcat(headers, IGC_ROW8);
  updateHash(IGC_ROW8);
  strcat(headers, "\r");
  strcat(headers, IGC_ROW9);
  updateHash(IGC_ROW9);
  strcat(headers, "\r");
  strcat(headers, IGC_ROW10);
  strcpy(hfrows, IGC_ROW10);
#ifdef VERSION
  strcat(headers, VERSION);
  strcat(hfrows, VERSION);
#endif
  updateHash(hfrows);
  strcat(headers, "\r");
  strcat(headers, IGC_ROW11);
  updateHash(IGC_ROW11);
  strcat(headers, "\r");
  strcat(headers, IGC_ROW12);
  updateHash(IGC_ROW12);
  strcat(headers, "\r");
  strcat(headers, IGC_ROW13);
  updateHash(IGC_ROW13);
  strcat(headers, "\r");
  strcat(headers, IGC_ROW14);
  updateHash(IGC_ROW14);
  strcat(headers, "\r");
  strcat(headers, IGC_ROW15);
  updateHash(IGC_ROW15);
  strcat(headers, "\r");
  strcat(headers, IGC_ROW16);
  strcat(headers, "\r");
  strcat(headers, IGC_ROW17);
  updateHash(IGC_ROW17);
  strcat(headers, "\r");
  return headers;
}

void Logger::doInitLogger(char *trackFile)
{

  // const char* headers = "...headers...\r";
  log_i("%s", trackFile);
  char newigc[1024];
  strcpy(newigc, igcHeaders());
#ifdef LILYGO_S3_E_PAPER_V_1_0
  writeFile(SD, trackFile, newigc);
#else
  writeFile(SD_MMC, trackFile, newigc);
#endif
  //  strcpy(gcodeRow, shaOut);
};

void Logger::updateLogger(void)
{

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

  static char row[256] = {0};

  strcpy(row, "B");
  strcat(row, status.gps.Time);

  static char lat_d[10]; // withouth leading zero added directly to row
  static char lon_d[10]; // withouth leading zero added directly to row
  static char lat_m[10];
  static char lon_m[10];
  int ilat_d = (int)status.gps.Lat;
  int ilat_m = (int)(round((status.gps.Lat - ilat_d) * 60. * 1000));
  int ilon_d = (int)status.gps.Lon;
  int ilon_m = (int)(round((status.gps.Lon - ilon_d) * 60. * 1000));
  // latitude from eg. 45.xxx to 45,0.xxx*60*1000N
  // Degrees
  if (ilat_d < 10)
    strcat(row, "0");
  itoa(ilat_d, lat_d, 10);
  strcat(row, lat_d);
  // Minutes
  if (ilat_m < 10000)
    strcat(row, "0");
  if (ilat_m < 1000)
    strcat(row, "0");
  if (ilat_m < 100)
    strcat(row, "0");
  if (ilat_m < 10)
    strcat(row, "0");
  itoa(ilat_m, lat_m, 10);
  strcat(row, lat_m);
  strcat(row, "N");
  // longitude from eg. 8.xxx to 8,0.xxx*60*1000E
  // Degrees
  if (ilon_d < 100)
    strcat(row, "0");
  if (ilon_d < 10)
    strcat(row, "0");
  itoa(ilon_d, lon_d, 10);
  strcat(row, lon_d);
  // Minutes
  if (ilon_m < 10000)
    strcat(row, "0");
  if (ilon_m < 1000)
    strcat(row, "0");
  if (ilon_m < 100)
    strcat(row, "0");
  if (ilon_m < 10)
    strcat(row, "0");
  itoa(ilon_m, lon_m, 10);
  strcat(row, lon_m);
  strcat(row, "E");

  // Altitude
  strcat(row, "A");
  // Altitude from vario
  char altvario[5];
  int ialtvario = (int)(round(status.gps.alt));
  if (ialtvario < 10000)
    strcat(row, "0");
  if (ialtvario < 1000)
    strcat(row, "0");
  if (ialtvario < 100)
    strcat(row, "0");
  if (ialtvario < 10)
    strcat(row, "0");
  itoa(ialtvario, altvario, 10);
  strcat(row, altvario);
  // ALtitude from GPS
  char altgps[5];
  int igpsvario = (int)(round(status.vario.alt));
  if (igpsvario < 10000)
    strcat(row, "0");
  if (igpsvario < 1000)
    strcat(row, "0");
  if (igpsvario < 100)
    strcat(row, "0");
  if (igpsvario < 10)
    strcat(row, "0");
  itoa(igpsvario, altgps, 10);
  strcat(row, altgps);

  // update hash - combine previous hash with new data
  //  char sha_comb[160];
  //  strcpy(sha_comb, gcodeRow);
  //  strcat(sha_comb, row);
  //  hash256(sha_comb);
  //  strcpy(gcodeRow, shaOut);
  updateHash(row);

  strcat(row, "\r");

#ifdef LILYGO_S3_E_PAPER_V_1_0
  appendFile(SD, igcPath, row);
#else
  appendFile(SD_MMC, igcPath, row);
#endif
}

void Logger::doStopLogger(void)
{

  // Close Hash sha256
  char gfinalrow[79];
  strcpy(gfinalrow, "G");
  strcat(gfinalrow, gcodeRow);
  strcat(gfinalrow, "\r");

  // add igc security line
  // close igc file and rename to .igc to be downloadable
  // TODO create the algorithm to write security based on data

  log_i("Closing igc file with security line G");
#ifdef LILYGO_S3_E_PAPER_V_1_0
  appendFile(SD, igcPath, gfinalrow);
#else
  appendFile(SD_MMC, igcPath, gfinalrow);
#endif
}

void Logger::createDir(fs::FS &fs, const char *path)
{
  // Serial.printf("Creating Dir: %s\n", path);
  log_i("Creating Dir: %s\n", path);
  if (fs.mkdir(path))
  {
    // Serial.println("Dir created");
    log_i("Dir created");
  }
  else
  {
    // Serial.println("mkdir failed");
    log_e("mkdir failed");
  }
}

void Logger::writeFile(fs::FS &fs, const char *path, const char *message)
{
  log_i("Writing file: %s", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    log_i("Failed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    log_i("File written");
  }
  else
  {
    log_i("Write failed");
  }
  file.close();
}

void Logger::appendFile(fs::FS &fs, const char *path, const char *message)
{
  log_i("Appending to file: %s", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file)
  {
    log_i("Failed to open file for appending");
    return;
  }
  if (file.print(message))
  {
    //        log_i("Message appended");
  }
  else
  {
    log_i("Append failed");
  }
  file.close();
}

void Logger::deleteFile(fs::FS &fs, const char *path)
{
  log_i("Deleting file: %s", path);
  if (fs.remove(path))
  {
    log_i("File deleted");
  }
  else
  {
    log_i("Delete failed");
  }
}

void Logger::listFiles(fs::FS &fs, const char *dirname, bool all_faf)
{
  // TODO List all files and push to server
  log_i("Listing igc files:");
  File root = fs.open(dirname);
  if (!root)
  {
    log_e("Failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    log_i("Not a directory");
    return;
  }

  File file = root.openNextFile();

  strcpy(igclist, "");
  while (file)
  {
    if (!file.isDirectory())
    {
      if (strstr(file.name(), ".igc"))
      {
        log_i("  FILE: %s", file.name());
        log_i("  SIZE: %10d", file.size());
        strcat(igclist, file.path());
        strcat(igclist, ";");
      }
    }
    else
    {
      log_i("  Folder: %s", file.name());
      strcat(igclist, file.path());
      strcat(igclist, ";");
    }
    file = root.openNextFile();
  }

  return;
}

// download igcfiles form server
// https://github.com/G6EJD/ESP32-8266-File-Download/blob/master/ESP_File_Download_v01.ino

void Logger::hash256(const char *payload)
{
  byte shaResult[32];

  mbedtls_md_context_t ctx;
  mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

  const size_t payloadLength = strlen(payload);

  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
  mbedtls_md_starts(&ctx);
  mbedtls_md_update(&ctx, (const unsigned char *)payload, payloadLength);
  mbedtls_md_finish(&ctx, shaResult);
  mbedtls_md_free(&ctx);

  // Serial.print("Hash: ");
  strcpy(shaOut, "");

  for (int i = 0; i < sizeof(shaResult); i++)
  {
    char str[3];
    sprintf(str, "%02x", (int)shaResult[i]);
    // Serial.print(str);
    strcat(shaOut, str);
  }

  return;
}