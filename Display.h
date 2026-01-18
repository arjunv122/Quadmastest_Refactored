#ifndef DISPLAY_H
#define DISPLAY_H

#include "Config.h"
#include "logo.h"

// Function declarations
void renderLogo();
void renderWelcomePage();
void renderBatteryLevelAndQuadPie();
void renderSensorCounter();
void renderAutoCalibrationPage();
int renderRecheckInputMessage(struct ElectricalConductivity elCdvty);
void displayEC(ElectricalConductivity elCdvty, int invalidInput);
float* displayTemp(float* temperatures);
bool checkBatteryAndRenderPrompt();
void checkCharging();

#endif