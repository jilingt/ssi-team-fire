/* Onboarding Project - Team Fire
 * 
 * Updated to initialize and de-initialize each sensor
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
const double altCalibrate = 1014.5;

int times = 0;

Adafruit_BMP280 bme; // (BMP_CS);

void writeToFile(char filename[], float writeLine) {
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
    Serial.println("parameter not found!");
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
  SPI.setSCK(13);
  
  // Initialize SD card
  Serial.print("Initializing SD card...");
  if (!sd.begin(SD_CS, SPI_QUARTER_SPEED)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  // Initialize bmp
  if (!bme.begin()) {  
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  } else {
    Serial.println("BMP280 sensor detected.");
  }
}

void loop() {
  // nothing happens after setup

  char tmpFile[] = "temptest.txt";
  char presFile[] = "presTest.txt";
  char altFile[] = "altTest.txt";

  char what[] = "temp";
  float temp = readBMP(what);
  strcpy(what, "pres");
  float pres = readBMP(what);
  strcpy(what, "alti");
  float alt = readBMP(what);
  
  writeToFile(tmpFile, temp); // Temperature
  writeToFile(presFile, pres);  // Pressure
  writeToFile(altFile, alt); // Altitude (approximated)

  delay(2000);
}
