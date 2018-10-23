/* Onboarding Project - Team Fire
 * 
 * Updated to change sensor wiring to I2C and add GPS
 */

#include <Wire.h>
#include <SPI.h>
#include "SdFat.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

SdFat sd;
SdFile file;
File myFile;

#include <SoftwareSerial.h>

#include <TinyGPS.h>

TinyGPS gps;

// Use one of these to connect your GPS
// ------------------------------------
#define gpsPort Serial1

const int SD_CS = 9;
//const int BMP_CS = 10;
const double altCalibrate = 1014.5;

int times = 0;

Adafruit_BMP280 bme; // (BMP_CS);

void writeToFile(char filename[], float writeLine) {
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = sd.open(filename, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
//    Serial.print("Writing to ");
//    Serial.print(filename);
    myFile.println(writeLine);
    // close the file:
    myFile.close();
//    Serial.println(". Done.");
  } else {
    // if the file didn't open, print an error:
//    Serial.print("error opening ");
//    Serial.println(filename);
  }
}

void writeToFile(char filename[], char writeLine[]) {
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = sd.open(filename, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
//    Serial.print("Writing to ");
//    Serial.print(filename);
    myFile.println(writeLine);
    // close the file:
    myFile.close();
//    Serial.println(". Done.");
  } else {
    // if the file didn't open, print an error:
//    Serial.print("error opening ");
//    Serial.println(filename);
  }
}

void readFile(char filename[]) {
  // open the file for reading:
  myFile = sd.open(filename);
  if (myFile) {
    Serial.print(filename);
    Serial.println(":");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();

    Serial.println();
  } else {
    // if the file didn't open, print an error:
    Serial.print("error opening ");
    Serial.println(filename);
  }
}

float readBMP(char readWhat[]) {
  if(strcmp(readWhat, "temp") == 0) {
    return bme.readTemperature();
  } else if(strcmp(readWhat, "pres") == 0) {
    return bme.readPressure();
  } else if(strcmp(readWhat, "alti") == 0) {
    return bme.readAltitude();
  } else {
//    Serial.println("parameter not found!");
  }

  return 0;
}



void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  SPI.setMOSI(11);
  SPI.setMISO(12);
  SPI.setSCK(14);
  
  // Initialize SD card
//  Serial.print("Initializing SD card...");
  if (!sd.begin(SD_CS, SPI_QUARTER_SPEED)) {
//    Serial.println("initialization failed!");
    return;
  }
//  Serial.println("initialization done.");

  // Initialize bmp
  if (!bme.begin()) {  
//    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  } else {
//    Serial.println("BMP280 sensor detected.");
  }

  // Starts GPS mwahahaha
  gpsPort.begin(4800);
}

void loop() {
  // GPS nonsense
  float flat, flon;
  unsigned long age, chars = 0;
  unsigned short sentences = 0, failed = 0;
  static const double LONDON_LAT = 51.508131, LONDON_LON = -0.128002;
  
  // nothing happens after setup

  char tmpFile[] = "temptest.txt";
  char presFile[] = "presTest.txt";
  char altFile[] = "BMPalt.txt";
  char gpsFile[] = "gpsTest.txt";

  char what[] = "temp";
  float temp = readBMP(what);
  strcpy(what, "pres");
  float pres = readBMP(what);
  strcpy(what, "alti");
  float alt = readBMP(what);

  gps.f_get_position(&flat, &flon, &age);
  int year;
  byte month, day, hour, minute, second, hundredths;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
  char sz[32];
  sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d ",
          month, day, year, hour, minute, second);
  
  writeToFile(tmpFile, temp); // Temperature
//  Serial.print("Temperature: ");
//  Serial.println(temp);
  writeToFile(presFile, pres);  // Pressure
//  Serial.print("Pressure: ");
//  Serial.println(pres);
  writeToFile(altFile, alt); // Altitude (approximated)
//  Serial.print("BMP Altitude: ");
//  Serial.println(alt);
  writeToFile(gpsFile, flat);
//  Serial.print("Flat: ");
//  Serial.println(flat);
  writeToFile(gpsFile, flon);
//  Serial.print("Flon: ");
//  Serial.println(flon);
  writeToFile(gpsFile, gps.f_altitude());
//  Serial.print("GPS Altitude:");
//  Serial.println(gps.f_altitude());
  writeToFile(gpsFile, sz);
//  Serial.print("Date and time: ");
//  Serial.println(sz);
  delay(2000);
}
