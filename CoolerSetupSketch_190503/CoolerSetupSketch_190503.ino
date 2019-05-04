/*
 * 
 * CoolerSetupSketch_190503 
 * by David Hartkop
 * CC-BY 2019
 * 
 * 
 * This sketch is meant to test the hardware used for the TEMPERATURE CONTROLLED VACCINE & INSULIN COOLER by David Hartkop, CC-BY 2019
 * What it does:
 * 
 * 1. Sets the Real Time Clock to whatever you type into the code below  (look below for the "SET THE REAL-TIME CLOCK HERE!" comment.)
 * 2. Tests the DFRobot DS3231 temperature sensor
 * 3. Tests writing and reading from the SD card reader
 * 4. Tests driving the PC fan.
 * 
 * 
 */
 
//Libraries
#include <Wire.h>                // used for RTC
#include <DS3231.h>              // used for RTC
#include <OneWire.h>             // used to read temperature sensors
#include <DallasTemperature.h>   // used to read temperature sensors
#include <SD.h>                  // used by SD card reader

// FAN ------------
int fanPin = 4;

// RTC -------------
#define DS3231_I2C_ADDRESS 0x68 
// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
  return( (val/10*16) + (val%10) );
}
// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return( (val/16*10) + (val%16) );
}

void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte
dayOfMonth, byte month, byte year)
{
  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.write(decToBcd(dayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(year)); // set year (0 to 99)
  Wire.endTransmission();
}
void readDS3231time(
byte *second,
byte *minute,
byte *hour,
byte *dayOfWeek,
byte *dayOfMonth,
byte *month,
byte *year
)

{
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}

//SD CARD ------------
File myFile;

//SENSORS ------------
// Data wire is connected to GPIO8
OneWire oneWire(9);
DallasTemperature tempSensor(&oneWire);

void setup() {
pinMode(fanPin, OUTPUT);  
digitalWrite(fanPin, LOW);
Serial.begin(9600);
Wire.begin();








//--------------------SET THE REAL-TIME CLOCK HERE!---------------------\\
//How to set the DS3231 RTC: (seconds, minutes, hours, day of week 01=sunday, date, month, year)
setDS3231time(00,38,20,1,06,01,19);//<-------------PUT NUMBERS HERE TO SET THE RTC
byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;









Serial.println("Cooler Setup Sketch - version 190504");
Serial.println("START OF SYSTEM TEST ----------------------");

 // retrieve data from DS3231
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month,
  &year);
Serial.print("TESTING REAL-TIME CLOCK: time[");
Serial.print(hour);
Serial.print(":");
Serial.print(minute);
Serial.print("] date[");
Serial.print(month);
Serial.print("/");
Serial.print(dayOfMonth);
Serial.print("/");
Serial.print("20");
Serial.print(year);
Serial.println("]");

// retrieve data from DS3231 temperature sensor
tempSensor.requestTemperaturesByIndex(0);

  Serial.print("TESTING TEMP. SENSOR: ");
  Serial.print(tempSensor.getTempCByIndex(0));
  Serial.println(" C");

Serial.println("TESTING SD CARD:");
//Write Recording-Start header information to SD Card
   pinMode(10, OUTPUT);//SD card pin
  if (!SD.begin(10)) {
    Serial.println("init failed!");
    return;
  }
  Serial.println("init done");
  
 myFile = SD.open("dataLog.txt", FILE_WRITE);
 
  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to dataLog.txt...");

    myFile.println("If you can read this, then your SD Card is working!");
  // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening dataLog.txt");
  }

 // Re-open the file from the Micro SD for reading:
  myFile = SD.open("dataLog.txt");
  if (myFile) {
    Serial.println("dataLog.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  } 

Serial.println("FAN TEST: Is the fan pulsing on and off?");
pinMode(fanPin, OUTPUT);  
  digitalWrite(fanPin, LOW);

Serial.println("END OF SYSTEM TEST ----------------------");
}

void loop() {

  digitalWrite(fanPin, HIGH);
  delay(200);
  digitalWrite(fanPin, LOW);
  delay(1000);

}
