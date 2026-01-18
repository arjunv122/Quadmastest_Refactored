#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include "FS.h"
#include "SPIFFS.h"
#include "SPI.h"
#include "TFT_22_ILI9225.h"
#include "EEPROM.h"
#include "RTClib.h"
#include "BfButton.h"
#include "AsyncDelay.h"
#include <BfButtonManager.h>
#include <MFRC522.h>
#include <MCP3XXX.h>
#include "ADS1X15.h"
#include <Wire.h>
#include <WebServer.h>
// Pin Definitions
#define TFT_RST 2
#define TFT_RS 15
#define TFT_CLK 18
#define TFT_SDI 23
#define TFT_CS 5
#define TFT_LED 0
#define TFT_BRIGHTNESS 200
#define TIMEOUT 30000
#define POWER_LATCH_PIN 32
#define POWER_BUTTON_PIN 35
#define AUTOCAL_PIN 36
#define PREVIOUS_PIN 39
#define NEXT_PIN 34

#define TEMP_PIN1 1
#define TEMP_PIN2 0
#define TEMP_PIN3 3
#define TEMP_PIN4 2
#define BATTERY_PIN 4
#define CHARGER_VOLT 33
#define BUZZER_PIN 4

#define SS_PIN 16
#define RST_PIN 17
#define SPI_MOSI 23
#define SPI_MISO 19
#define SPI_SCK 18
// Time zone offset for IST (India Standard Time)
#define IST_OFFSET_SECONDS 19800  // 5 hours 30 minutes (5*3600 + 30*60)
// Constants
const int SPIFFS_CAPACITY_LIMIT_PERCENT = 80;
const int BAT_MIN_RAW = 455;
const int BAT_MAX_RAW = 535;
const long IST_OFFSET = 5*3600 + 30*60;
const long TOL = 2;
extern const char VersionString[];

// Structs
struct ElectricalConductivity {
  float ecWithoutComp[4] = {0.0}; 
  float ecWithComp[4] = {0.0};
};

struct scValueCuttoffMap {
  float minimumValue[15] = {5.6, 5.7,5.8,5.9,6.0,6.1,6.2,6.3,6.4,6.5,6.6,6.7,6.8,6.9,7.0};
  int percentage[15]= {20,19,18,17,16,15,14,13,12,11,10,9,8,7,6};
};

// Global Objects
extern TFT_22_ILI9225 tft;
extern RTC_DS1307 rtc;
extern MFRC522 rfid;
extern MCP3008 mcp;
extern ADS1115 ADS;
extern AsyncDelay delay_1m;
extern BfButton power;
extern SPIClass RFID_SPI;

// Global Variables
extern int testsCounter;
extern int battPercentage;
extern struct ElectricalConductivity elCdvty;
extern struct scValueCuttoffMap SCmap;
extern float temperatures[4];
extern String positives[2][4];
extern String rfidNumber;
extern String currentDate;
extern String currentTime;
extern uint32_t uniqueChipID;
extern bool autoCalFlag;
extern float kvalueLow[4];
extern float kvalueHigh[4];
extern float kvalue[4];
extern bool invalid[4];
extern int invalidInput;
extern bool calibrationA;
extern bool calibrationB;
extern bool wifiFlag;
extern bool routineCompleteFlag;
extern String resultFL, resultFR, resultBL, resultBR;
extern bool ntpSynced;
extern bool lastPrintHadTests;

extern bool solAFlag;
extern float temperatureLow[4];
extern float temperatureHigh[4];
extern byte nuidPICC[4];
extern String tagContent;
extern WebServer server;
#endif