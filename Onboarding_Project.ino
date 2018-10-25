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
#include <IridiumSBD.h>

// SD Card
SdFat sd;
SdFile file;
File myFile;

// GPS
TinyGPS gps;
#define gpsPort Serial1

#define IridiumSerial Serial3

// Declare the IridiumSBD object
IridiumSBD modem(IridiumSerial);

// BMP
Adafruit_BMP280 bme;

const int SD_CS = 9; // Chip select for SD


int seconds;

void writeToFile(char filename[], float writeLine, int prec) {
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = sd.open(filename, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.println(writeLine, prec);
    myFile.println(writeLine, prec);
    myFile.close();
  }
}

void writeToFile(char filename[], char writeLine[]) {
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = sd.open(filename, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.println(writeLine);
    myFile.println(writeLine);
    // close the file:
    myFile.close();
  }
}

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);

  // Initialize GPS port
  Serial1.begin(9600);

  // Initialize SD port
  SPI.setMOSI(11);
  SPI.setMISO(12);
  SPI.setSCK(14);
  
  // Initialize SD card
  sd.begin(SD_CS, SPI_QUARTER_SPEED);

  // Initialize bmp
  bme.begin();

  // Initialize RockBlock
  IridiumSerial.begin(19200);
  seconds = 0;

}

void loop() {
  
  char master[] = "master.txt";
  
  // Getting data from BMP
  float temp = bme.readTemperature();
  float pres = bme.readPressure();
  float alt = bme.readAltitude();

  // Getting data from GPS
  float flat, flon;
  unsigned long age = 0;
  gps.f_get_position(&flat, &flon, &age);
  writeToFile(master, "****");
  print_date(gps, master);

  writeToFile(master, temp, 2); // Temperature
  writeToFile(master, pres, 2);  // Pressure
  writeToFile(master, alt, 2); // BMP Altitude (approximated)
  
  writeToFile(master, flat, 10);
  writeToFile(master, flon, 10);
  writeToFile(master, gps.f_altitude(), 2);
  

  // Vital method; do not remove!
  smartdelay(1000);


  char buff[20];
  String toSend = "";
  
  dtostrf(alt, 4, 2, buff);
  toSend += buff;
  toSend += ",";
  dtostrf(flat, 4, 10, buff);
  toSend += buff;
  toSend += ",";
  dtostrf(flon, 4, 10, buff);
  toSend+= buff;

  int signalQuality;
  int err = modem.getSignalQuality(signalQuality);
  if (err == ISBD_SUCCESS)
  {
    if (signalQuality > 0 && signalQuality > (150 - seconds) / 30) 
    {
      err = modem.sendSBDText("1000,37,-122");
      if (err != ISBD_SUCCESS) {
        writeToFile(master, "Failure: poor signal");
      }
      else {
        writeToFile(master, "Hey, it worked!");
        seconds = 0;
      }
    }
  }
  else {
    writeToFile(master, "Failure: can't get signal quality");  
  }
  

  delay(20000);
  
  seconds += 20;
}


static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (Serial1.available())
      gps.encode(Serial1.read());
  } while (millis() - start < ms);
}


static void print_date(TinyGPS &gps, char filename[])
{
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
  if (age == TinyGPS::GPS_INVALID_AGE)
    writeToFile(master, "GPS NO SIGNAL");
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d ",
        month, day, year, hour, minute, second);
    writeToFile(filename, sz);
  }
  smartdelay(0);
}
