#include <Arduino.h>
#include <vector>
#include "SIM7600G.h"

HardwareSerial SerialAT(2);

String broker_ip = "34.30.152.206";
String client_id = "customize_id";
bool gps_state = false;

unsigned long startTime = 0;

String sendAT(String command)
{
  SerialAT.print(command + "\r");

  String response = "";
  bool ok_status = false;
  bool error_status = false;
  while (!ok_status && !error_status)
  {
    while (SerialAT.available())
    {
      String temp = SerialAT.readString();
      response += temp;
      Serial.println(temp);
      if (temp.indexOf("OK") != -1)
      {
        ok_status = true;
      }
      else if (temp.indexOf("ERROR") != -1)
      {
        error_status = true;
      }
    }
    delay(500);
  }

  return response;
}

void beginGPS()
{
  String check = sendAT("AT+CGPS?");
  if (check.indexOf("+CGPS: 0,1") != -1)
  {
    Serial.println("GPS Disabled");
    Serial.println("Starting GPS...");
    sendAT("AT+CGPS=1");
  }
  else if (check.indexOf("+CGPS: 1,1") != -1)
  {
    Serial.println("GPS Enabled");
    gps_state = true;
  }
}

void beginSIM7600G()
{
  startTime = millis();
  SerialAT.begin(115200);
  Serial.println("Initializing SIM7600G...");

  bool ready = false;
  bool sim_ready = true;

  while (!ready)
  {
    while (SerialAT.available())
    {
      String response = SerialAT.readString();
      Serial.println(response);
      if (response.indexOf("+CME ERROR: SIM not inserted") != -1)
      {
        sim_ready = false;
      }
      if (response.indexOf("RDY") != -1)
      {
        ready = true;
      }
    }
    delay(500);
  }

  if (!sim_ready)
  {
    return;
  }

  sendAT("ATI");

  beginGPS();

  unsigned long endTime = millis() - startTime;
  float duration = endTime / 1000;
  Serial.println("Duration : " + String(duration) + " Second");
  Serial.println("=== End of SIM7600 Initialization ===");
}

std::vector<String> splitString(const String &input, char delimiter)
{
  std::vector<String> tokens;
  String currentToken = "";

  for (int i = 0; i < input.length(); i++)
  {
    char c = input[i];
    if (c == delimiter)
    {
      if (!currentToken.isEmpty())
      {
        tokens.push_back(currentToken);
      }
      currentToken = "";
    }
    else
    {
      currentToken += c;
    }
  }

  if (!currentToken.isEmpty())
  {
    tokens.push_back(currentToken);
  }

  return tokens;
}

gpsReading getGPS()
{
  gpsReading gps;
  if (!gps_state)
  {
    beginGPS();
  }

  /* AT+CGSPINFO - response
  AT+CGPSINFO

  +CGPSINFO: 0614.354283,S,10651.371415,E,230624,104328.0,2.9,0.0,

  OK
  */
  String gps_data = sendAT("AT+CGPSINFO");
  std::vector<String> data;
  bool complete = false;

  std::vector<String> _gps_data = splitString(gps_data, ':');
  for (String _data : _gps_data)
  {
    data = splitString(_data, ',');
  }

  if (data.empty())
  {
    return gps;
  }

  for (String dat : data)
  {
    Serial.println(dat);
  }

  float lat = data[0].toFloat();
  float lon = data[2].toFloat();

  Serial.println("Latitude  : " + String(lat));
  Serial.println("Longitude : " + String(lon));

  return gps;
}

void MQTTStart()
{
  String start = sendAT("AT+CMQTTSTART");
  if (start.indexOf("+CMQTTSTART: 0") == -1)
  {
    Serial.println("MQTT failed to start");
    return;
  }

  String client_acquired = sendAT("AT+CMQTTACCQ=0,\"" + client_id + "\",0,4");
  if (client_acquired.indexOf("OK") == -1)
  {
    Serial.println("Failed to acquire client");
    return;
  }
  Serial.println("Client id: " + client_id);

  String connection = sendAT("AT+CMQTTCONNECT=0,\"tcp://" + broker_ip + "\",120,1");
  if (connection.indexOf("OK") == -1)
  {
    Serial.println("Failed to connect to broker");
  }

  return;
}

void publish(String payload)
{
}