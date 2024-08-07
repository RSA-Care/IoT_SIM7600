#include <Arduino.h>
#include "SIM7600G.h"

HardwareSerial SerialAT(2);

String broker_ip = "34.30.152.206";
bool gps_state = false;
bool ready = false;
bool sim_ready = true;

unsigned long startTime = 0;

String sendAT(String command, String expected = "")
{
  SerialAT.print(command + "\r");

  String response = "";
  bool ok_status = false;
  bool error_status = false;
  bool accept_response = false;
  startTime = millis();
  while (SerialAT.available() || (!ok_status && !error_status) && millis() - startTime < 120000)
  {
    String temp = SerialAT.readStringUntil('\n');
    temp.trim();
    Serial.println(temp);
    if (expected.length() > 0 && temp.indexOf(expected) != -1)
    {
      response = temp;
      ok_status = true;
    }

    if (temp.indexOf(command) != -1)
    {
      accept_response = true;
    }

    if ((temp.indexOf("OK") != -1 && accept_response))
    {
      ok_status = true;
    }
    else if (temp.indexOf("ERROR") != -1 && accept_response)
    {
      error_status = true;
    }
    else if (temp.indexOf("RDY") != -1)
    {
      response = "";
      ok_status = true;
    }
    else
    {
      response += temp;
    }

    response += "\n";
  }
  delay(500);

  if (millis() - startTime > 120000)
  {
    SerialAT.print("AT+CFUN=6\r");
    ready = false;
    while (!ready)
    {
      while (SerialAT.available())
      {
        String temp = SerialAT.readStringUntil('\n');
        temp.trim();
        Serial.println(temp);
        if (temp.indexOf("RDY") != -1)
        {
          response = "";
          ready = true;
        }
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

void SIM7600Gbegin()
{
  Serial.println("Starting SIM7600");
  startTime = millis();
  Serial.println("Start time : " + String(startTime));
  SerialAT.begin(115200);

  while (!ready && (millis() - startTime) < 60000)
  {
    while (SerialAT.available())
    {
      String response = SerialAT.readString();
      println(response);
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

  if (!sim_ready || !ready)
  {
    return;
  }

  clearScreen();
  println("GPS TRACKER");
  println("FOR DOWN SYNDROME");
  for (int i = 0; i < 21; i++)
  {
    print("=");
  }
  println("SIM7600G");

  sendAT("ATI");

  sendAT("AT+CMEE=2");

  beginGPS();

  unsigned long endTime = millis() - startTime;
  float duration = endTime / 1000;
  println("Duration : " + String(duration) + " Second");
  Serial.println("=== End of SIM7600 Initialization ===");
}

String splitString(String input, char delimiter, int index = 0)
{
  input.trim();
  String result;
  int prev_delimiter_index = 0;
  int temp_index = 0;

  for (int i = 0; i < input.length(); i++)
  {
    if (input[i] == delimiter)
    {
      if (temp_index == index)
      {
        if (prev_delimiter_index == 0)
        {
          result = input.substring(prev_delimiter_index, i);
        }
        else
        {
          result = input.substring(prev_delimiter_index + 1, i);
        }
      }
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

  float lat = splitString(_data, ',').toFloat();
  float lon = splitString(_data, ',', 2).toFloat();

  gps.latitude = String(lat);
  gps.longitude = String(lon);

  return gps;
}

void MQTTStart()
{
  unsigned long seed = millis();
  const char charSet[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

  Serial.println("Starting Randomize Topic.");

  String client_id;
  for (int i = 0; i < 10; i++)
  {
    int randomIndex = (seed >> i) % (sizeof(charSet) - 2);
    client_id += charSet[randomIndex];
  }

  String start = sendAT("AT+CMQTTSTART", "+CMQTTSTART: 0");

  if (start.indexOf("+CMQTTSTART: 23") != -1)
  {
    Serial.println("MQTT Opened");
    String client_acquired = sendAT("AT+CMQTTACCQ=0,\"" + client_id + "\",0,4");
    Serial.println("Client id: " + client_id);

    String connection = sendAT("AT+CMQTTCONNECT=0,\"tcp://" + broker_ip + "\",120,1");
    return;
  }

  return;
}

void publish(String payload)
{
  // String topic = "test";
  String topic = getData("/topic.txt");
  if (topic.length() == 0)
  {
    randomizeMQTTTopic();
    topic = getData("/topic.txt");
  }

  String response;
  bool set_topic = false;
  bool set_payload = false;
  bool publish = false;
  bool error = false;
  SerialAT.print("AT+CMQTTTOPIC=0," + String(topic.length()) + "\r");
  delay(500);
  while (!set_topic && !error)
  {
    while (SerialAT.available())
    {
      String temp = SerialAT.readStringUntil('\n');
      Serial.println(temp);
      if (temp.indexOf("+CMQTTTOPIC: 0,11") != -1)
      {
        Serial.println("ERROR: no connection\nStarting MQTT on SIM7600...");
        MQTTStart();
        error = true;
      }
      if (temp.indexOf("OK") != -1)
      {
        set_topic = true;
      }
      else if (temp.indexOf("ERROR") != -1)
      {
        error = true;
      }
      SerialAT.print(topic + "\r");
    }

    delay(500);
  }

  if (error)
  {
    return;
  }

  SerialAT.print("AT+CMQTTPAYLOAD=0," + String(payload.length()) + "\r");
  delay(500);
  while (!set_payload && !error)
  {
    while (SerialAT.available())
    {
      String temp = SerialAT.readStringUntil('\n');
      Serial.println(temp);
      if (temp.indexOf("OK") != -1)
      {
        set_payload = true;
      }
      SerialAT.print(payload + "\r");
    }

    delay(500);
  }

  if (error)
  {
    return;
  }

  sendAT("AT+CMQTTPUB=0,0,120,1", "+CMQTTPUB: 0,0");
}

SIM7600 getDeviceInfo()
{
  SIM7600 device;

  String signal_data = sendAT("AT+CSQ");
  String raw_signal_data = splitString(signal_data, ':');
  raw_signal_data.replace(" ", "");
  String rssi = splitString(raw_signal_data, ',');
  String ber = splitString(raw_signal_data, ',', 1);

  device.signalStrength = rssi.toInt();
  device.errorRate = ber.toInt();

  return device;
}