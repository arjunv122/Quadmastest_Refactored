#include "Config.h"
#include "TimeManager.h"
#include "Display.h"
#include "Sensors.h"
#include "FileSystem.h"
#include "Calibration.h"
#include "Testing.h"
#include "RFID.h"
#include "Printing.h"
#include "Utilities.h"
#include "wifiUpdateAndSPIFFSDirectory.h"
// Add this line after the includes: 
// Global variable definitions
const char VersionString[] = "V03.04.05";
TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_LED, TFT_BRIGHTNESS);
RTC_DS1307 rtc;
MFRC522 rfid(SS_PIN, RST_PIN);
MCP3008 mcp;
ADS1115 ADS(0x48);
AsyncDelay delay_1m;
BfButton power(BfButton::STANDALONE_DIGITAL, POWER_BUTTON_PIN, false, HIGH);
SPIClass RFID_SPI(VSPI);
SPIClass vspi(VSPI);

int testsCounter = 0;
int battPercentage = 0;
struct ElectricalConductivity elCdvty;
struct scValueCuttoffMap SCmap;
float temperatures[4] = {0.0};
String positives[2][4];
String rfidNumber = "";
String currentDate = "";
String currentTime = "";
uint32_t uniqueChipID = 0;
bool autoCalFlag = false;
float kvalueLow[4];
float kvalueHigh[4];
float kvalue[4];
struct ElectricalConductivity ecLow;
struct ElectricalConductivity ecHigh;
int EEPROMAddressACal = 10;
bool solAFlag = false;
float temperatureLow[4] = {0.0};    // ← KEEP ONLY THIS ONE
float temperatureHigh[4] = {0.0};   // ← KEEP ONLY THIS ONE
byte nuidPICC[4];                   // ← KEEP ONLY THIS ONE
String tagContent = "";             // ← KEEP ONLY THIS ONE
bool invalid[4] = {false, false, false, false};
int invalidInput = 0;
bool calibrationA = false;
bool calibrationB = false;
bool wifiFlag = false;
bool routineCompleteFlag = false;
String resultFL = "";
String resultFR = "";
String resultBL = "";
String resultBR = "";
bool ntpSynced = false;


// ❌ DELETE THESE DUPLICATE LINES (lines 59-63 in your error):
// bool solAFlag = false;
// float temperatureLow[4] = {0.0};
// float temperatureHigh[4] = {0.0};
// byte nuidPICC[4];
// String tagContent = "";

void setup() {
  pinMode(POWER_LATCH_PIN, OUTPUT);
  pinMode(POWER_BUTTON_PIN, INPUT);
  pinMode(AUTOCAL_PIN, INPUT);
  pinMode(NEXT_PIN, INPUT);
  pinMode(PREVIOUS_PIN, INPUT);
  pinMode(CHARGER_VOLT, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(POWER_LATCH_PIN, HIGH);

  Serial.begin(115200);
  SPI.begin();
  tft.begin();
  
  renderLogo();
  
  RFID_SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SS_PIN);
  rfid.PCD_Init();
  EEPROM.begin(512);
  
  digitalWrite(CHARGER_VOLT, LOW);
  delay_1m.start(TIMEOUT, AsyncDelay::MILLIS);
  power.onPress(pressHandler);
  power.onPressFor(pressHandler, 3000);

  autoCalBegin();

  // SPIFFS initialization
  if (!SPIFFS.begin(false)) {
    Serial.println("SPIFFS Mount Failed ❌ - Attempting format...");
    if (SPIFFS.format()) {
      Serial.println("SPIFFS formatted successfully ✅");
      if (SPIFFS.begin(false)) {
        Serial.println("SPIFFS Mounted Successfully after format ✅");
      } else {
        Serial.println("SPIFFS Mount Failed even after format ❌");
      }
    } else {
      Serial.println("SPIFFS Format Failed ❌");
    }
  } else {
    Serial.println("SPIFFS Mounted Successfully ✅");
  }

  // RTC initialization
  Wire.begin();
  if (! rtc.begin()) {
    Serial.println("RTC not found!");
  } else {
    DateTime bootUTC = rtc.now();
    Serial.print("RTC on boot (UTC): ");
    Serial.println(bootUTC.timestamp());
    logUTCandIST("Boot");

    bool invalid = (bootUTC.year() < 2024 || bootUTC.year() > 2099);
    if (invalid) {
      DateTime compileTime(F(__DATE__), F(__TIME__));
      Serial.println("Seeding RTC from compile time (last resort)...");
      rtc.adjust(compileTime);
      delay(50);
      bootUTC = rtc.now();
      Serial.print("RTC seeded (UTC): ");
      Serial.println(bootUTC.timestamp());
    } else {
      Serial.println("RTC looks valid ✅");
    }

    DateTime ist = DateTime(bootUTC.unixtime() + IST_OFFSET);
    Serial.print("RTC IST (display): ");
    Serial.println(ist.timestamp());
  }

  mcp.begin(27, 13, 12, 14);
  
  xTaskCreatePinnedToCore(
    powerButtonLoop,
    "powerButtonLoop",
    2000,
    NULL,
    0,
    NULL,
    ARDUINO_RUNNING_CORE
  );

  renderWelcomePage();
  resetTestCounter(); // TESTING ONLY; COMMENT FOR PRODUCTION
  Serial.println("EXITING SETUP");
}

void loop() {
 
  server.handleClient();
}

void powerButtonLoop(void* pvParameters) {
  while (1) {
    if (delay_1m.isExpired() && ! wifiFlag && routineCompleteFlag) {
      tft.clear();
      Serial.println("Time Out");
      Serial.println(F("POWER OFF"));
      tft.clear();
      digitalWrite(POWER_LATCH_PIN, LOW);
    }
    power.read();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void pressHandler(BfButton *btnPower, BfButton::press_pattern_t pattern) {
  switch (pattern) {
    case BfButton::SINGLE_PRESS:
      Serial. println(F("POWER ON"));
      ESP.restart();
      break;
    case BfButton::LONG_PRESS:
      Serial. println(F("POWER OFF"));
      tft.setBacklight(false);
      tft.setDisplay(false);
      tft.clear();
      digitalWrite(POWER_LATCH_PIN, LOW);
      break;
  }
}