/* Onboarding Project - Team Fire
 * 
 * Updates
 * *Successfully transmits via RockBlock
 * *Transmits BMP and GPS data in <= 50 bytes
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
#define gpsPort Serial1
TinyGPS gps;

// Iridium SBD
#define IridiumSerial Serial3
IridiumSBD modem(IridiumSerial);

// BMP
Adafruit_BMP280 bme;

const int SD_CS = 9; // Chip select for SD

int seconds = 80;  // Seconds since last transmission; initialized to send 40 seconds after first initialization

void writeToFile(char filename[], float writeLine, int prec) {
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = sd.open(filename, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
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
        month, day, year, (hour-7), minute, second);
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
  float bmp_alt = bme.readAltitude();

  // Getting data from GPS
  float flat, flon;
  unsigned long age = 0;
  gps.f_get_position(&flat, &flon, &age);
  float gps_alt = gps.f_altitude();
  char tmp[] = "****";
  writeToFile(master, tmp);
  print_date(gps, master);

  writeToFile(master, temp, 2); // Temperature
  writeToFile(master, pres, 2);  // Pressure
  writeToFile(master, bmp_alt, 2); // BMP Altitude (approximated)
  
  writeToFile(master, flat, 10);  // GPS Latitude
  writeToFile(master, flon, 10);  // GPS Longitude
  writeToFile(master, gps.f_altitude(), 2); // GPS Altitude

  // Vital method for GPS; do not remove!
  smartdelay(1000);

  // Data to be sent: temp, pressure, BMP altitude, GPS latitude, GPS longitude, GPS altitude
  char buff[20] = "";
  char toSend[50] = "";
  dtostrf(temp, 5, 1, buff);
  strcat(toSend, buff);
  strcat(toSend, ",");
  dtostrf(pres, 6, 0, buff);
  strcat(toSend, buff);
  strcat(toSend, ",");
  dtostrf(bmp_alt, 6, 0, buff);
  strcat(toSend, buff);
  strcat(toSend, ",");
  dtostrf(flat, 10, 5, buff);
  strcat(toSend, buff);
  strcat(toSend, ",");
  dtostrf(flon, 10, 5, buff);
  strcat(toSend, buff);
  strcat(toSend, ",");
  dtostrf(gps_alt, 7, 1, buff);
  strcat(toSend, buff);

  // Sending through RockBlock
  int signalQuality;
  int err = modem.getSignalQuality(signalQuality);
  if (err == ISBD_SUCCESS) {
    // Only sends if at least 2 minutes has passed since last transmission, and signal quality is 3 and above
    // or if at least 4 minutes has passed since last transmission and signal quality is not 0
    if ((signalQuality > 2 || (signalQuality > 0 && seconds >= 240)) && seconds >= 120) {
      err = modem.sendSBDText(toSend);
      if (err == ISBD_SUCCESS) {
        seconds = 0;
      }
    }
  }
  
  delay(19000); // plus 1000 from smart delay = 20 seconds
  
  seconds += 20;
}
