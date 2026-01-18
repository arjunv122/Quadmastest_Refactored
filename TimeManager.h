#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include "Config.h"

// Function declarations
String iso(const DateTime& dt);
void logUTCandIST(const char* tag);
bool approxEq(time_t a, time_t b, long tol=2);
String two(int v);
String fmt12h(const DateTime& dt);
bool isRTCTimeOlderThanCompileTime(DateTime rtcTime, DateTime compileTime);
void syncRTCFromNTP_UTC_once();
void rtcSelfTest();
void setupRTC_Basic();
void printNowBothUTCandIST(const char* tag);
bool rtcLooksValid(const DateTime& t);

#endif