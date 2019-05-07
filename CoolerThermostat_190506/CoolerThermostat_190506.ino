/*
 * David's Data Logger V. 190506
 * System only reads one temp sensor.
 * System currently has no sleep mode to save power. 
 * Temp control is tuned for a good starting-point setup. Cooler will find good tuning over time.
 * set a minimum adjustment as well as a maximum adjustment
 */

#include <Wire.h>                // used for RTC
#include <DS3231.h>              // used for RTC
#include <OneWire.h>             // used to read temperature sensor
#include <DallasTemperature.h>   // used to read temperature sensor
#include <SD.h>                  // used by SD card reader

//SD CARD ------------
File myFile;

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

//SENSOR ---------------
// Data wire is connected to GPIO9
OneWire oneWire(9);
DallasTemperature tempSensor(&oneWire);//setup to read sensor

// FAN ------------
int fanPinGND = 2;
int fanPinVCC = 3;
int fanPin = 4;

// RTC -------------
int RTCVCCpin = 8;

// SD card reader------------
int SDreaderVCCpin = 7;

//Thermostat variables
unsigned long currentTime; //variable for most recent read of arduino run time millis()
unsigned long tstatTimeWindow = 6000; //time in ms. must be long enough for thermostat action to be seen. It should be 10-15 minutes.  
unsigned long tstatTimestamp = 0; //set to current time at end of a given window

float thisWindowTemp; //used to determine thermostat action. This is the determined by a series of measurements taken over the course of a minute and averaged.
float prevWindowTemp; //used to determine overshoots. 

float setPoint = 5; //set to 5C
float setPointRange = 0; // +-0.5C

int maxAdjustment = 500; //maximum number of fanRunTime milliseconds allowed to be added/subtracted in a single window
int minAdjustment = 25;
int currentAdjustment = 50; //holds the calculated number of fanRunTime milliseconds to be added/subtracted for a given window


//TIMING VARIABLES-----------------------
unsigned long logTimeStamp = 0;

unsigned long fanTimeWindow = 1000; //100 seconds for test (100000ms)
unsigned long fanTimeStamp = 0;
unsigned long fanRunTime = 500;//number of milliseconds the fan is allowed to run in a 100,000 millisecond timeframe. This value is adjusted by the thermostat loop over time.
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx+++++++++++xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx+xxx line added by my 3 year old son;

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
void readDS3231time(byte *second,
byte *minute,
byte *hour,
byte *dayOfWeek,
byte *dayOfMonth,
byte *month,
byte *year)

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

void LogDataFunction(){

//Write timestamp and temp data to SD Card

 myFile = SD.open("dataLog.txt", FILE_WRITE);
 
  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to dataLog.txt...");
    
byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;

  // retrieve data from DS3231
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month,
  &year);
  
  // send it to the SD Card:
 
  myFile.print(year, DEC);
  myFile.print(" ");
  myFile.print(month, DEC);
  myFile.print(" ");
  myFile.print(dayOfMonth, DEC);
  myFile.print(" ");
  myFile.print(hour, DEC);
  myFile.print(" ");
   if (minute<10)
  {
    myFile.print("0");
  }
  myFile.print(minute, DEC);
  myFile.print(" ");
if (second<10)
  {
    myFile.print("0");
  }
  myFile.print(second, DEC);
  
//Serial.print("Requesting temperatures...");
  tempSensor.requestTemperatures(); // Send the command to get temperature
  //Serial.println("DONE");
  
  myFile.print(" ");
  myFile.print(tempSensor.getTempCByIndex(0)); 

  myFile.print(" ");
  myFile.print(currentAdjustment);

  myFile.print(" ");
  myFile.println(fanRunTime);
  
  // close the file:
    myFile.close();
    Serial.println("done.");
    
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }


}

void downloadDataFunction(){

// re-open the file for reading:
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

}

void setup() {

  Serial.begin(9600);

  Wire.begin();

//Set up Pins

pinMode(RTCVCCpin, OUTPUT);
pinMode(SDreaderVCCpin, OUTPUT);
pinMode(fanPinGND, OUTPUT);
pinMode(fanPinVCC, OUTPUT);  
pinMode(fanPin, OUTPUT);  

digitalWrite(RTCVCCpin, HIGH);
digitalWrite(SDreaderVCCpin, HIGH);
digitalWrite(fanPinGND, LOW);  
digitalWrite(fanPinVCC, HIGH);
digitalWrite(fanPin,LOW);

delay(500); //let power stapalize for peripherals

 //Setting RTC: DS3231 seconds, minutes, hours, day of week 01=sunday, date, month, year
 //setDS3231time(00,38,20,1,06,01,19);//uncomment this line and set the numbers then upload to set clock. Finally, comment then upload again.

tempSensor.begin();


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

    myFile.println("Yr Mo Dy Hr Mn Sc Tmp Adj Run");
  // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening dataLog.txt");
  }

downloadDataFunction();

}


void loop() {

delay(500);//half second delay to keep thermocouple reads stable
currentTime = millis(); //update currentTime variable


//PERIODICALLY RECORDS ALL DATA TO SD CARD
if (currentTime-logTimeStamp>60000){
    LogDataFunction();
    delay(500);
    logTimeStamp = currentTime;
  }

//Run the fan at the current duty-cycle setting:

  if ((currentTime-fanTimeStamp)>(fanTimeWindow) && (currentTime-fanTimeStamp)<(fanTimeWindow+fanRunTime)){
    digitalWrite(fanPin,HIGH);
    }

  else if ((currentTime-fanTimeStamp)>(fanTimeWindow) && (currentTime-fanTimeStamp)>(fanTimeWindow+fanRunTime)){
    fanTimeStamp = currentTime;
    digitalWrite(fanPin,LOW);
    }


//Run the thermostat adjustment loop:

if (currentTime-tstatTimestamp>tstatTimeWindow){
  tstatTimestamp = currentTime;

prevWindowTemp = thisWindowTemp; //update the prevWindowTemp before reading the current temp into thisWidowTemp

//read the temperature into thisWindowTemp:
for(int i=0; i<4; i++){
thisWindowTemp = thisWindowTemp+(tempSensor.getTempCByIndex(0));
delay(1000);
}

thisWindowTemp = thisWindowTemp/5;



//If i overshot setPoint Window Range by warming up:
if (thisWindowTemp > (setPoint+setPointRange) && (setPoint-setPointRange) > prevWindowTemp){
    currentAdjustment = currentAdjustment/2;
    if (currentAdjustment < minAdjustment){
      currentAdjustment = minAdjustment;}
    fanRunTime=fanRunTime+currentAdjustment;
    }

//If i overshot setPoint Window Range by cooling down:
  else if (thisWindowTemp < (setPoint-setPointRange) && (setPoint+setPointRange) < prevWindowTemp){
    currentAdjustment = currentAdjustment/2;
    if (currentAdjustment < minAdjustment){
      currentAdjustment = minAdjustment;}
    if (fanRunTime < currentAdjustment){
      fanRunTime = 0;}
    else fanRunTime=(fanRunTime-currentAdjustment);
    }
    
//If the temp is STILL higher than the setPointWindowRange since last time:
  else if (thisWindowTemp > (setPoint+setPointRange) && prevWindowTemp > (setPoint+setPointRange)){
    currentAdjustment=(2*currentAdjustment);
    if (currentAdjustment > maxAdjustment){
    currentAdjustment = maxAdjustment;}
    fanRunTime=(fanRunTime+currentAdjustment);
    }
    
//If the temp is STILL lower than the setPointWindowRange since last time:
  else if (thisWindowTemp < (setPoint-setPointRange) && prevWindowTemp < (setPoint-setPointRange)){
    currentAdjustment=(2*currentAdjustment);
    if (currentAdjustment > maxAdjustment){
    currentAdjustment = maxAdjustment;}
    if (fanRunTime < currentAdjustment){
      fanRunTime = 0;}
    else {fanRunTime=(fanRunTime-currentAdjustment);
      }
    }
  }
}
