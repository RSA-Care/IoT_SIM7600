#if !defined(SIM7600G_h)
#define SIM7600G_h

#include <Arduino.h>
#include <HardwareSerial.h>
#include "OLED/SSD1306.h"
#include "Data/DataHandler.h"

// Your GPRS credentials, if any
const char apn[] = "internet";
const char gprsUser[] = "";
const char gprsPass[] = "";

typedef struct
{
  String latitude = "0.00";
  String longitude = "0.00";
} gpsReading;

void SIM7600Gbegin();
void beginGPS();
gpsReading getGPS();
void publish(String payload);

#endif // SIM7600G_h