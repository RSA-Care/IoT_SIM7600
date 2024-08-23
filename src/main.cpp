#include <Arduino.h>
#include "SIM7600G/SIM7600G.h"
#include "OLED/SSD1306.h"
#include "DHT/DHT22.h"
#include "Data/DataHandler.h"

unsigned long mainStartTime = 0;

void setup()
{
  mainStartTime = millis();
  Serial.begin(115200);
  oledBegin();
  SIM7600Gbegin();
  SPIFFSBegin();
  dhtBegin();

  clearScreen();
  header(getData("topic.txt"), false);
}

void loop()
{
  gpsReading gps = getGPS();
  dhtReading dht = getDHT();

  // display on oled screen
  gpsDisplay(String(gps.latitude), String(gps.longitude));
  dhtDisplay(String(dht.temperature), String(dht.humidity));

  if (millis() - mainStartTime > 5000)
  {
    String payload = gps.latitude + "," + gps.longitude + "," + String(dht.temperature) + "," + String(dht.humidity);
    publish(payload);
    mainStartTime = millis();
  }
}