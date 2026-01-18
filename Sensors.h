#ifndef SENSORS_H
#define SENSORS_H

#include "Config.h"

// Function declarations
float calculateTempWithNTC(int voltage);
float* measureTemp(float temperatures[]);
ElectricalConductivity measureECWithoutComp(float* temperatures, ElectricalConductivity elCdvty);
int newSCcuttoffs(float minimum);

#endif