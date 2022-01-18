/*
Sensors designed for Emily Wedel to measure temperature of fires in watersheds at Konza.

This set-up uses a adafruit Adalogger and ds3231 featherwing to log data and track time.
K-type thermocouple will be measured with a MCP9600 universal thermocouple amplifier that uses I2C communication.
This sketch also flashes a green light after data has been logged as an indicator that the data is being logged properly.
 */
/**************************************************
      define the pin for monitoring the battery
**************************************************/

#define VBATPIN A7

/**************************************************
      set time of sample interval
**************************************************/

unsigned int sample_interval = 1 * 1000;

/**************************************************
      RTC set-up
**************************************************/

#include "RTClib.h"
RTC_DS3231 rtc;

/**************************************************
      SD Card set-up 
**************************************************/

#include <SPI.h>
#include <SD.h>

const int chipSelect = 4;
int sd_pin = 7;
int grnLED = 8;
int redLED = 13;

/**************************************************
      Thermocouple amplifier set-up 
**************************************************/

#include <Wire.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_I2CRegister.h>
#include "Adafruit_MCP9600.h"

#define I2C_ADDRESS (0x67)

Adafruit_MCP9600 mcp;

void setup()
{
    Serial.begin(115200);

    pinMode(sd_pin,INPUT);
    pinMode(grnLED,OUTPUT);
    pinMode(redLED,OUTPUT);
    digitalWrite(redLED,LOW);

/**************************************************
      RTC Initialization and set time
**************************************************/

    if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

//    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));


/**************************************************
      MCP9600 Initilization and set parameters
**************************************************/

    Serial.println("MCP9600 HW test");

    /* Initialise the driver with I2C_ADDRESS and the default I2C bus. */
    if (! mcp.begin(I2C_ADDRESS)) {
        Serial.println("Sensor not found. Check wiring!");
       
    }

  Serial.println("Found MCP9600!");

  mcp.setADCresolution(MCP9600_ADCRESOLUTION_18);
  Serial.print("ADC resolution set to ");
  switch (mcp.getADCresolution()) {
    case MCP9600_ADCRESOLUTION_18:   Serial.print("18"); break;
    case MCP9600_ADCRESOLUTION_16:   Serial.print("16"); break;
    case MCP9600_ADCRESOLUTION_14:   Serial.print("14"); break;
    case MCP9600_ADCRESOLUTION_12:   Serial.print("12"); break;
  }
  Serial.println(" bits");

  mcp.setThermocoupleType(MCP9600_TYPE_K);
  Serial.print("Thermocouple type set to ");
  switch (mcp.getThermocoupleType()) {
    case MCP9600_TYPE_K:  Serial.print("K"); break;
    case MCP9600_TYPE_J:  Serial.print("J"); break;
    case MCP9600_TYPE_T:  Serial.print("T"); break;
    case MCP9600_TYPE_N:  Serial.print("N"); break;
    case MCP9600_TYPE_S:  Serial.print("S"); break;
    case MCP9600_TYPE_E:  Serial.print("E"); break;
    case MCP9600_TYPE_B:  Serial.print("B"); break;
    case MCP9600_TYPE_R:  Serial.print("R"); break;
  }

  Serial.println(" type");

  mcp.setFilterCoefficient(3);
  Serial.print("Filter coefficient value set to: ");
  Serial.println(mcp.getFilterCoefficient());

  mcp.enable(true);

  Serial.println(F("------------------------------"));

/**************************************************
      SD Card Initialization and print file headers
**************************************************/
  while(digitalRead(sd_pin) == 0){
    Serial.println("No SD Card Inserted!!!");
    digitalWrite(grnLED,HIGH);
    delay(500);
    digitalWrite(grnLED,LOW);
    delay(500);
  }
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");
  digitalWrite(redLED,HIGH);
  delay(20);
  digitalWrite(redLED,LOW);
  

  File dataFile = SD.open("datalog.csv", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println("date, time, v_bat, hot_j, cold_j, adc, rtc_temp");
    dataFile.close();
    // print to the serial port too:
    Serial.println("date, time, v_bat, hot_j, cold_j, adc, rtc_temp");
      digitalWrite(redLED,HIGH);
      delay(20);
      digitalWrite(redLED,LOW);

  }

}

void loop()
{
  String timeString = "";
  String dataString = "";


/**************************************************
      RTC get time
**************************************************/

      DateTime now = rtc.now();

    
    timeString += now.month(), DEC;
    timeString += '/';
    timeString += now.day(), DEC;
    timeString += '/';    
    timeString += now.year(), DEC;
    timeString += ",";
    timeString += now.hour(), DEC;
    timeString += ':';
    timeString += now.minute(), DEC;
    timeString += ':';
    timeString += now.second(), DEC;
    timeString += ",";

/**************************************************
      get the battery voltage
**************************************************/

float vbat = analogRead(VBATPIN)*2*3.3/1024;
dataString += vbat; dataString += ",";

/**************************************************
      MCP9600 get temperature data 
**************************************************/

  dataString += mcp.readThermocouple(); dataString += ",";
  dataString += mcp.readAmbient(); dataString += ",";
  dataString += mcp.readADC() * 2; dataString += ",";
  dataString += rtc.getTemperature();

/**************************************************
      SD Card print time and data
**************************************************/
  
File dataFile = SD.open("datalog.csv", FILE_WRITE);

  if (dataFile) {
    dataFile.print(timeString);
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.print(timeString);
    Serial.println(dataString);
      digitalWrite(redLED,HIGH);
      delay(20);
      digitalWrite(redLED,LOW);

  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.csv");
  }


/**************************************************
      delay for time of sample interval
**************************************************/
 
  delay(sample_interval);
}
