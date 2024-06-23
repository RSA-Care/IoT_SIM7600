#include <Arduino.h>
#include "SIM7600G/SIM7600G.h"

void setup()
{
  Serial.begin(115200);
  beginSIM7600G();
}

void loop()
{
  gpsReading gps = getGPS();

  delay(20000);
}