/* Onboarding Project - Team Fire
 * 
 * Writes data but data from BMP is inaccurate
 */

#include <Wire.h>
#include <SPI.h>
#include "SdFat.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

SdFat sd;
SdFile file;
File myFile;

const int SD_CS = 9;
const int BMP_CS = 10;
const double altCalibrate = 1013.25;

Adafruit_BMP280 bme(BMP_CS);

void writeToFile(char filename[], char writeLine[]) {
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = sd.open(filename, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to ");
    Serial.print(filename);
    myFile.println(writeLine);
    // close the file:
    myFile.close();
    Serial.println(". Done.");
  } else {
    // if the file didn't open, print an error:
    Serial.print("error opening ");
    Serial.println(filename);
  }
}

//void writeToFile(String filename, String writeLine) {
//  // open the file. note that only one file can be open at a time,
//  // so you have to close this one before opening another.
//  myFile = sd.open(filename, FILE_WRITE);
//
//  // if the file opened okay, write to it:
//  if (myFile) {
//    Serial.print("Writing to " + filename);
//    myFile.print(writeLine);
//    // close the file:
//    myFile.close();
//    Serial.println(". Done.");
//  } else {
//    // if the file didn't open, print an error:
//    Serial.println("error opening " + filename);
//  }
//}

//void writeTemperature(String filename) {
//  // open the file. note that only one file can be open at a time,
//  // so you have to close this one before opening another.
//  myFile = sd.open(filename, FILE_WRITE);
//
//  // if the file opened okay, write to it:
//  if (myFile) {
//    Serial.print("Writing to " + filename);
//    myFile.print("Temperature = ");
//    myFile.print(bme.readTemperature());
//    myFile.println(" *C");
//    // close the file:
//    myFile.close();
//    Serial.println(". Done.");
//  } else {
//    // if the file didn't open, print an error:
//    Serial.println("error opening " + filename);
//  }
//}
//
//void writePressure(String filename) {
//  // open the file. note that only one file can be open at a time,
//  // so you have to close this one before opening another.
//  myFile = sd.open(filename, FILE_WRITE);
//
//  // if the file opened okay, write to it:
//  if (myFile) {
//    Serial.print("Writing to " + filename);
//    myFile.print("Pressure = ");
//    myFile.print(bme.readPressure());
//    myFile.println(" Pa");
//    // close the file:
//    myFile.close();
//    Serial.println(". Done.");
//  } else {
//    // if the file didn't open, print an error:
//    Serial.println("error opening " + filename);
//  }
//}

//void writeAltitude(String filename) {
//  // open the file. note that only one file can be open at a time,
//  // so you have to close this one before opening another.
//  myFile = sd.open(filename, FILE_WRITE);
//
//  // if the file opened okay, write to it:
//  if (myFile) {
//    Serial.print("Writing to " + filename);
//    myFile.print("Approx altitude = ");
//    myFile.print(bme.readAltitude(altCalibrate));
//    myFile.println(" Pa");
//    // close the file:
//    myFile.close();
//    Serial.println(". Done.");
//  } else {
//    // if the file didn't open, print an error:
//    Serial.println("error opening " + filename);
//  }
//}

void readFile(char filename[]) {
  // re-open the file for reading:
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
  SPI.setMOSI(11);
  SPI.setMISO(12);
  SPI.setSCK(13);
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  if (!bme.begin()) {  
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  } else {
    Serial.println("BMP280 sensor detected.");
  }

  Serial.print("Initializing SD card...");
  if (!sd.begin(SD_CS, SPI_QUARTER_SPEED)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  Serial.println();

  char tmpStr[25];
  strcat(tmpStr, "Temperature = ");
  Serial.println(tmpStr);
  char tmp[6];
  Serial.println(bme.readTemperature());
  strcat(tmpStr, dtostrf(bme.readTemperature(), 6, 2, tmp));
  strcat(tmpStr, " *C");
  Serial.println(tmpStr);

  char presStr[25];
  strcat(presStr, "Pressure = ");
  char pres[9];
  Serial.println(bme.readPressure());
  strcat(presStr, dtostrf(bme.readPressure(), 9, 2, pres));
  strcat(presStr, " Pa");
  Serial.println(presStr);

  char altStr[25];
  strcat(altStr, "Altitude = ");
  char alt[9];
  Serial.println(bme.readPressure());
  strcat(altStr, dtostrf(bme.readPressure(), 9, 2, alt));
  strcat(altStr, " m");
  Serial.println(altStr);

  char tmpFile[] = "temptest.txt";
  char presFile[] = "pressureTest.txt";
  char altFile[] = "altTest.txt";
  
  writeToFile(tmpFile, tmpStr); // Temperature
  writeToFile(presFile, presStr);  // Pressure
  writeToFile(altFile, altStr); // Altitude (approximated)

  readFile(tmpFile);
  readFile(presFile);
  readFile(altFile);
}

void loop() {
  // nothing happens after setup
  
}
