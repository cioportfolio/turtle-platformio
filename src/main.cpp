#include <Arduino.h>
#include "common.hpp"
#include "sd.hpp"
#include "web.hpp"
#include "control.hpp"

void initSerial() {
  Serial.begin(115200);
  // Wait for USB Serial
  while (!Serial && (millis()<3000)) {
    yield();
  }
}

void setup() {
  initSerial();
  Serial.println("Serial done. Pause...");
  delay(5000);
  Serial.println("Start motors...");
  initControl();
  Serial.println("Pause...");
  delay(5000);
  Serial.println("Start SD...");
  initSD();
  Serial.println("Pause...");
  delay(5000);
  Serial.println("Start web.");
  initWeb();
}

//------------------------------------------------------------------------------
void loop() {
    processWeb();
    yield();
}