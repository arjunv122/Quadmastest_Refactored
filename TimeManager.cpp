#include "TimeManager.h"
#include <time.h>

String iso(const DateTime& dt) { 
  return String(dt.timestamp()); 
}

void logUTCandIST(const char* tag) {
  DateTime u = rtc.now();
  DateTime i = DateTime(u.unixtime() + IST_OFFSET);
  Serial.printf("[%s]\n  RTC UTC: %s (%ld)\n  RTC IST: %s\n",
                tag, u.timestamp().c_str(), (long)u.unixtime(), i.timestamp().c_str());
}

bool approxEq(time_t a, time_t b, long tol) { 
  return labs((long)a - (long)b) <= tol; 
}

String two(int v) { 
  char b[3]; 
  snprintf(b, sizeof(b), "%02d", v); 
  return String(b); 
}

String fmt12h(const DateTime& dt) {
  int hr12 = dt.hour() % 12; 
  if (hr12 == 0) hr12 = 12;
  char buf[16];
  snprintf(buf, sizeof(buf), "%02d:%02d %s", hr12, dt.minute(), dt.hour() < 12 ? "AM" : "PM");
  return String(buf);
}

bool isRTCTimeOlderThanCompileTime(DateTime rtcTime, DateTime compileTime) { 
  Serial.println(F("RTC Time "));
  Serial.print(rtcTime.timestamp());
  Serial.println(F("Compile Time "));  
  Serial.print(compileTime.timestamp());
  return (rtcTime < compileTime);
}

void syncRTCFromNTP_UTC_once() {
  if (ntpSynced) { 
    Serial.println("NTP already synced (skipped)"); 
    return; 
  }

  configTime(0, 0, "pool.ntp.org", "time.google.com", "time.nist.gov");
  struct tm tmUTC;
  
  if (!getLocalTime(&tmUTC, 8000)) {
    Serial.println("NTP failed ❌"); 
    return;
  }

  time_t ntp = time(nullptr);
  DateTime before = rtc.now();

  Serial.printf("NTP (UTC): %s", asctime(gmtime(&ntp)));
  Serial.printf("RTC before (UTC): %s (%ld)\n", before.timestamp().c_str(), (long)before.unixtime());

  if (! approxEq(before.unixtime(), ntp)) {
    rtc.adjust(DateTime((uint32_t)ntp));
    delay(50);
  }

  DateTime after = rtc.now();
  Serial.printf("RTC after  (UTC): %s (%ld)\n", after.timestamp().c_str(), (long)after.unixtime());

  if (approxEq(after.unixtime(), ntp)) {
    Serial.println("SYNC PASS ✅");
    ntpSynced = true;
  } else {
    Serial.println("SYNC FAIL ❌");
  }
  logUTCandIST("Post NTP");
}

void rtcSelfTest() {
  Serial.println("=== RTC SELF-TEST ===");
  time_t known = 1731400000;
  rtc.adjust(DateTime((uint32_t)known));
  delay(50);
  DateTime r1 = rtc.now();
  Serial.printf("Set RTC to known UTC: %s (%ld)\n", r1.timestamp().c_str(), (long)r1.unixtime());

  if (!approxEq(r1.unixtime(), known)) {
    Serial.println("Seed FAIL ❌"); 
    return;
  } else {
    Serial.println("Seed PASS ✅");
  }

  time_t fakeNTP = known + 123;
  rtc.adjust(DateTime((uint32_t)fakeNTP));
  delay(50);
  DateTime r2 = rtc. now();
  Serial.printf("Adjust to fake NTP:    %s (%ld)\n", r2.timestamp().c_str(), (long)r2.unixtime());

  if (!approxEq(r2.unixtime(), fakeNTP)) {
    Serial.println("Adjust FAIL ❌"); 
    return;
  } else {
    Serial.println("Adjust PASS ✅");
  }

  DateTime ist = DateTime(r2.unixtime() + IST_OFFSET);
  Serial.printf("IST display: %s\n", ist.timestamp().c_str());
  Serial.println("RTC SELF-TEST COMPLETE ✅");
}

bool rtcLooksValid(const DateTime& t) {
  return (t.year() >= 2024 && t.year() <= 2099);
}

void setupRTC_Basic() {
  Wire.begin();
  if (!rtc.begin()) {
    Serial.println("RTC not found!");
    return;
  }

  DateTime t = rtc.now();
  Serial.print("RTC on boot (UTC): ");
  Serial.println(t.timestamp());

  bool needSeed = ! rtcLooksValid(t);

  if (needSeed) {
    DateTime compileTime(F(__DATE__), F(__TIME__));
    Serial.println("Seeding RTC from compile time (last resort)...");
    rtc.adjust(compileTime);
    delay(50);
    t = rtc.now();
    Serial.print("RTC seeded (UTC): ");
    Serial.println(t.timestamp());
  } else {
    Serial.println("RTC looks valid ✅");
  }
}

void printNowBothUTCandIST(const char* tag) {
  DateTime utc = rtc.now();
  DateTime ist = DateTime(utc.unixtime() + IST_OFFSET);
  Serial.print(tag); 
  Serial.println();
  Serial.print("  UTC: "); 
  Serial.println(utc.timestamp());
  Serial.print("  IST: "); 
  Serial.println(ist.timestamp());
}