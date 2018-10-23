/* Onboarding Project - Team Fire
 * 
 * Updated to change sensor wiring to I2C and add GPS
 * Cleaned
 */

#include <Wire.h>
#include <SPI.h>
#include "SdFat.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <TinyGPS.h>

// SD Card
SdFat sd;
SdFile file;
File myFile;

// GPS
#define gpsPort Serial1
TinyGPS gps;

// BMP
Adafruit_BMP280 bme;

const int SD_CS = 9; // Chip select for SD
const double altCalibrate = 1014.5; // Change this to current pressure at sea level

void writeToFile(char filename[], float writeLine) {
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = sd.open(filename, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    myFile.println(writeLine);
    myFile.close();
  }
}

void writeToFile(char filename[], char writeLine[]) {
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = sd.open(filename, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    myFile.println(writeLine);
    // close the file:
    myFile.close();
  }
}

// NOTE: this function probably won't be used (since we can't output to serial)
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
  sd.begin(SD_CS, SPI_QUARTER_SPEED);

  // Initialize bmp
  bme.begin();

  // Starts GPS mwahahaha
  gpsPort.begin(4800);
}

void loop() {
  // GPS nonsense
  float flat, flon;
  unsigned long age, chars = 0;
  unsigned short sentences = 0, failed = 0;
  static const double LONDON_LAT = 51.508131, LONDON_LON = -0.128002;
  int year;
  byte month, day, hour, minute, second, hundredths;
  char sz[32];

  // Filenames (remember can't be longer than 8 chars for FAT)
  char tmpFile[] = "temptest.txt";
  char presFile[] = "presTest.txt";
  char altFile[] = "BMPalt.txt";
  char gpsFile[] = "gpsTest.txt";

  // Getting data from BMP
  float temp = bme.readTemperature();
  float pres = bme.readPressure();
  float alt = bme.readAltitude();

  // Getting data from GPS
  gps.f_get_position(&flat, &flon, &age);
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
  sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d ", month, day, year, hour, minute, second);

  // Writing data to files
  writeToFile(tmpFile, temp); // Temperature
  writeToFile(presFile, pres);  // Pressure
  writeToFile(altFile, alt); // BMP Altitude (approximated)
  writeToFile(gpsFile, flat); // Latitude
  writeToFile(gpsFile, flon); // Longitude
  writeToFile(gpsFile, gps.f_altitude()); // GPS Altitude
  writeToFile(gpsFile, sz); // GPS date and time

//  Print statements for all variables written to files
//  Serial.print("Temperature: ");
//  Serial.println(temp);
//  Serial.print("Pressure: ");
//  Serial.println(pres);
//  Serial.print("BMP Altitude: ");
//  Serial.println(alt);
//  Serial.print("Flat: ");
//  Serial.println(flat);
//  Serial.print("Flon: ");
//  Serial.println(flon);
//  Serial.print("GPS Altitude:");
//  Serial.println(gps.f_altitude());
//  Serial.print("Date and time: ");
//  Serial.println(sz);

  delay(2000);
}
