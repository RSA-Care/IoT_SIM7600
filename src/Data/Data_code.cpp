#include "DataHandler.h"

// const char *filename = "/data.json";

bool SPIFFSBegin()
{
  bool start = SPIFFS.begin();
  if (!start)
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
  }
  else
  {
    Serial.println("SPIFFS mounted successfully");
  }

  return start;
}

String getData(String filename)
{
  bool fileCheck = SPIFFS.exists(filename);
  if (!fileCheck)
  {
    Serial.println("File does not exist");
    return "";
  }

  File file = SPIFFS.open(filename, FILE_READ);

  if (!file)
  {
    Serial.println("ERROR: an error has occured when attempting to open file\nSPIFFS ERROR");
    return "";
  }

  const int bufferSize = min(MAX_BUFFER_SIZE, file.available() + 1); // Adjust buffer size based on data size
  char *buffer = new char[bufferSize];

  // Read the data
  int bytesRead = file.readBytes(buffer, bufferSize - 1);
  if (bytesRead == bufferSize - 1)
  {
    if (file.available() > 0 && file.peek() != '\n')
    {
      file.read();              // Read an extra byte to discard potential incomplete data
      buffer[bytesRead] = '\0'; // Add null terminator manually
    }
    else
    {
      buffer[bytesRead] = '\0'; // Likely complete line, add null terminator
    }
  }
  else
  {
    buffer[bytesRead] = '\0'; // Add null terminator for shorter data
  }

  // Close the file
  file.close();

  // Create a String object from the buffer (consider memory management)
  String dataString(buffer);

  // Deallocate memory used by the buffer
  delete[] buffer;

  return dataString;
}

bool saveData(String data, String filename)
{
  bool fileCheck = SPIFFS.exists(filename);
  if (!fileCheck)
  {
    Serial.println("File does not exist");
    return false;
  }

  File file = SPIFFS.open(filename, FILE_READ);

  if (!file)
  {
    Serial.println("ERROR: an error has occured when attempting to open file\nSPIFFS ERROR");
    return false;
  }

  file.println(data);

  file.close();

  return true;
}