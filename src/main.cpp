#include <Arduino.h>
#include "SIM7600G/SIM7600G.h"
#include "OLED/SSD1306.h"
#include "DHT/DHT22.h"
#include "Data/DataHandler.h"

void setup()
{
  Serial.begin(115200);
  oledBegin();
  SIM7600Gbegin();
  SPIFFSBegin();
  dhtBegin();

  clearScreen();
  SIM7600 gprs = getDeviceInfo();
  header(String(gprs.signalStrength), getData("topic.txt"));
}

void loop()
{
  gpsReading gps = getGPS();
  dhtReading dht = getDHT();

  String payload = gps.longitude + "," + gps.latitude + "," + String(dht.temperature) + "," + String(dht.humidity);
  publish(payload);

  // display on oled screen
  gpsDisplay(String(gps.latitude), String(gps.longitude));
  dhtDisplay(String(dht.temperature), String(dht.humidity));

  delay(10000);
}