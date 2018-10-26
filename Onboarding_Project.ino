/* Onboarding Project - Team Fire
 * 
 * Updates
 * *Successfully transmits via RockBlock
 * *Transmits BMP altitude and GPS data
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

// Iridium SBD
#define IridiumSerial Serial3
IridiumSBD modem(IridiumSerial);

// BMP
Adafruit_BMP280 bme;

const int SD_CS = 9; // Chip select for SD

int seconds = 180;  // Seconds since last transmission; initialized to send upon first connection

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

void writeToFile(char filename[], int writeLine) {
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

static void smartdelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (Serial1.available()) {
      gps.encode(Serial1.read());
    }
  } while (millis() - start < ms);
}

static void print_date(TinyGPS &gps, char filename[]) {
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
  if (age == TinyGPS::GPS_INVALID_AGE) {
    char tmp[] = "GPS NO SIGNAL";
    writeToFile(filename, tmp);
  } else {
    char sz[32] = "";
    sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d ",
        month, day, year, hour, minute, second);
    writeToFile(filename, sz);
  }
  smartdelay(0);
}

void setup() {
  char master[] = "master.txt"; // filename
  
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

  // Initialize RockBlock modem
  int err;
  err = modem.begin();
  if (err != ISBD_SUCCESS) {
    char tmp[] = "Begin failed: error";
    writeToFile(master, tmp);
    writeToFile(master, err);
    if (err == ISBD_NO_MODEM_DETECTED) {
      strcpy(tmp, "No modem detected: check wiring.");
      writeToFile(master, tmp);
    }
  } else {
    char tmp[] = "Modem initialized.";
    writeToFile(master, tmp);
  }
}

void loop() {
  char master[] = "master.txt"; // Filename
  
  // Getting data from BMP
  float temp = bme.readTemperature();
  float pres = bme.readPressure();
  float alt = bme.readAltitude();

  // Getting data from GPS
  float flat, flon;
  unsigned long age = 0;
  gps.f_get_position(&flat, &flon, &age);
  char tmp[] = "****";
  writeToFile(master, tmp);
  print_date(gps, master);

  writeToFile(master, temp, 2); // Temperature
  writeToFile(master, pres, 2);  // Pressure
  writeToFile(master, alt, 2); // BMP Altitude (approximated)
  
  writeToFile(master, flat, 10);  // GPS Latitude
  writeToFile(master, flon, 10);  // GPS Longitude
  writeToFile(master, gps.f_altitude(), 2); // GPS Altitude

  // Vital method for GPS; do not remove!
  smartdelay(1000);

  // Data to be sent
  char buff[20] = "";
  char toSend[42] = "";
  dtostrf(alt, 9, 2, buff);
  strcat(toSend, buff);
  strcat(toSend, ",");
  dtostrf(flat, 15, 10, buff);
  strcat(toSend, buff);
  strcat(toSend, ",");
  dtostrf(flon, 15, 10, buff);
  strcat(toSend, buff);

  Serial.print("data to send: ");
  Serial.println(toSend);

  int signalQuality;
  int err = modem.getSignalQuality(signalQuality);
  if (err == ISBD_SUCCESS) {
    Serial.print("Signal quality: ");
    Serial.println(signalQuality);
    Serial.print("seconds: ");
    Serial.println(seconds);
    if ((signalQuality > 2 || (signalQuality > 0 && seconds >= 300)) && seconds >= 180) {
      Serial.println("Trying to send.");
      err = modem.sendSBDText(toSend);
      if (err != ISBD_SUCCESS) {
        char tmp[] = "Failure to send: poor signal";
        writeToFile(master, tmp);
      } else {
        char tmp[] = "Hey, it worked!";
        writeToFile(master, tmp);
        seconds = 0;
      }
    } else {
      char tmp[] = "signalQuality not greater than 0; max time not yet exceeded";
      writeToFile(master, tmp);
    }
  } else {
    char tmp[] = "Failure: can't get signal quality";
    writeToFile(master, tmp);  
  }
  
  delay(20000);
  
  seconds += 20;
}
