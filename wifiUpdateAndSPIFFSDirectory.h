#ifndef WIFI_UPDATE_SPIFFS_H
#define WIFI_UPDATE_SPIFFS_H

#ifdef ESP8266
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  #include <ESP8266mDNS.h>
#else
  #include <WiFi.h>
  #include <WebServer.h>
  #include "SPIFFS.h"
  #include <WiFiManager.h>
  #include <DNSServer.h>  // ← ADD THIS
#endif

#define ServerVersion "2.2"
extern String webpage;
extern bool SPIFFS_present;
extern uint32_t uniqueChipID;
extern String addIP;

#include "FS.h"
#include <SPI.h>

#ifdef ESP8266
  extern ESP8266WebServer server;
#else
  extern WebServer server;
#endif

// Function Declarations
void File_Action(String action_type);
void SPIFFS_dir();
void SendHTML_Header();
void SendHTML_Content();
void SendHTML_Stop();
void ReportSPIFFSNotPresent();
void ReportFileNotPresent(String target);
String file_size(int bytes);
void append_page_header();
void append_page_footer();
String Wifisetup(uint32_t uniqueChipID);

// Trend and Visualization
void Trend_Page();
void List_Files_JSON();
void Get_File_Content();

// WiFi Management (NEW)
void WiFi_Reset_Page();
void WiFi_Reset_Confirm();
void WiFi_Info_Page();

// ============================================================
// NEW:  Captive Portal Handlers
// ============================================================
void handleNotFound();
void handleCaptivePortal();
void handleHotspotDetect();

#endif