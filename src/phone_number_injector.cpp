#include <EEPROM.h>
#include <Arduino.h>

// Put allowed numbers in this array. all reports will be sent to the first number in the array.
// For example:  String phoneNumbers[] = {"+989123456789", "+989876543210"}
String phoneNumbers[] = {};

// N: length of phoneNumbers array
const int N = sizeof(phoneNumbers) / sizeof(phoneNumbers[0]);

// #### function prototypes ####

void writeToEEPROM(int addrOffset, const String &strToWrite);
String readFromEEPROM(int addrOffset);

// #### end of function prototypes ####

void setup()
{
  Serial.begin(9600);

  for (int i = 0; i < N; i++)
  {
    writeToEEPROM(i * 13, phoneNumbers[i]);
  }
  Serial.print(N);
  Serial.println(" phone numbers was written");

  delay(5000);

  for (int i = 0; i < N; i++)
  {
    String s = readFromEEPROM(i * 13);
    Serial.println(s);
  }
}

void loop()
{
}

/*******************************************************************************
 * writeToEEPROM function:
 * Store registered phone numbers in EEPROM
 ******************************************************************************/
void writeToEEPROM(int addrOffset, const String &strToWrite)
{
  byte len = 13; // strToWrite.length();
  for (int i = 0; i < len; i++)
  {
    EEPROM.write(addrOffset + i, strToWrite[i]);
  }
}

/*******************************************************************************
 * readFromEEPROM function:
 * Store phone numbers in EEPROM
 ******************************************************************************/
String readFromEEPROM(int addrOffset)
{
  int len = 13;
  char data[len + 1];
  for (int i = 0; i < len; i++)
  {
    data[i] = EEPROM.read(addrOffset + i);
  }
  data[len] = '\0';
  return String(data);
}