#include <Arduino.h>
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
      String temp = SerialAT.readStringUntil('\n');
      temp.trim();
      Serial.println(temp);
      if (temp.indexOf("OK") != -1)
      {
        ok_status = true;
      }
      else if (temp.indexOf("ERROR") != -1)
      {
        error_status = true;
      }
      else
      {
        response += temp;
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
  println("SIM7600G");

  bool ready = false;
  bool sim_ready = true;

  while (!ready && (millis() - startTime) < 60000)
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

String splitString(String input, char delimiter, int index = 0)
{
  input.trim();
  String result;
  String res[input.length()];
  int prev_delimiter_index = 0;
  int temp_index = 0;

  for (int i = 0; i < input.length(); i++)
  {
    if (input[i] == delimiter)
    {
      if (temp_index == index)
      {
        result = input.substring(prev_delimiter_index, i);
      }
      // res[temp_index] = input.substring(prev_delimiter_index + 1, i);
      temp_index++;
      prev_delimiter_index = i;
    }
  }

  if (result.isEmpty())
  {
    result = input.substring(prev_delimiter_index + 1, input.length());
  }

  return result;
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
  bool complete = false;

  String _data = splitString(gps_data, ' ', 1);

  Serial.println(_data);

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