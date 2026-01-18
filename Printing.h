#ifndef PRINTING_H
#define PRINTING_H

#include "Config.h"
#include <BluetoothSerial.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>

// External declarations
extern BluetoothSerial SerialBT;
extern BLEScan *pBLEScan;
extern bool connected;
extern bool lastPrintHadTests;  // ✅ ADD THIS

// Function declarations
bool tpsetup();
void tpPrint(ElectricalConductivity elCdvty);
void tpPrint2();
void print5setup();
void printLast5Tests();
bool connectToPrinter();
void bleSetup();
void toMACAddress(const String &macStr, uint8_t *macArray);
void testPrintLast5Tests();

#endif