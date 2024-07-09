#include <Arduino.h>
#include "SIM7600G/SIM7600G.h"
#include "OLED/SSD1306.h"
#include "DHT/DHT22.h"
#include "Data/DataHandler.h"

void setup()
{
  Serial.begin(115200);
  SPIFFSBegin();
  oledBegin();
  dhtBegin();
  SIM7600Gbegin();
}

void loop()
{
  gpsReading gps = getGPS();
  dhtReading dht = getDHT();

  // display on oled screen
  clearScreen();
  println("Latitude : " + gps.latitude);
  println("Longitude : " + gps.longitude);
  println("Temperature : " + String(dht.temperature));
  println("Humidity : " + String(dht.humidity));

  delay(10000);
}