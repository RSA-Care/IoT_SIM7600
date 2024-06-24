#include <Arduino.h>
#include "SIM7600G/SIM7600G.h"
#include "OLED/SSD1306.h"

void setup()
{
  Serial.begin(115200);
  oledBegin();
  beginSIM7600G();
}

void loop()
{
  gpsReading gps = getGPS();

  delay(10000);
}