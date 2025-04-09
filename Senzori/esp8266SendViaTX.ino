/*

Ovaj kod salje podatke sa IMU i GNSS na Serial, u vidu json stringa. Namenjen je za **ESP 8266** i NEO M9N 00b GNSS

Ovo je primer podatka koji se salje:
{"latitude":37.7749,"longitude":-122.4194,"altitude":10.5,"speed":5.2,"timestamp":"2023-10-05 14:30:45","imu":{"accelerometer":{"x":1234,"y":5678,"z":9101},"gyroscope":{"x":2345,"y":6789,"z":1011},"temperature":25.3}}

Testiracu ga 10.3. ili 11.3. i okacicu snimak negde.

Potencijalne izmene, u zavisnosti od toga koje komponente koristimo:
-DODATI PODATKE SA STRUJNOG SENZORA
-dodati podatke sa dosta ultrasonic senzora (bice potrebno jos oloca vrv)
-dodati podatke sa 2d lidara

*/
#include <TinyGPS++.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <MPU6050_light.h>
#include <SoftwareSerial.h>

#define GPS_BAUDRATE 38400
#define JSON_BAUDRATE 9600
#define GPS_RX_PIN D5    // GPS TX → ESP8266 D5 (GPIO14)
#define JSON_TX_PIN D6   // JSON output via D6 (GPIO12)

TinyGPSPlus gps;
MPU6050 mpu(Wire);
SoftwareSerial gpsSerial(GPS_RX_PIN, 255); // RX-only for GPS
SoftwareSerial jsonSerial(JSON_TX_PIN, 255); // TX-only for JSON

void setup() {
  Serial.begin(115200); // Debug
  gpsSerial.begin(GPS_BAUDRATE);
  jsonSerial.begin(JSON_BAUDRATE);
  
  Wire.begin(D2, D1); // I2C: SDA=D2 (GPIO4), SCL=D1 (GPIO5)
  mpu.begin();
  mpu.calcGyroOffsets(); // Auto-calibrate IMU
}

void loop() {
  // Read GPS data
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      processData();
    }
  }

  // GPS timeout check
  if (millis() > 5000 && gps.charsProcessed() < 10) {
    jsonSerial.println("{\"error\":\"No GPS\"}");
  }
}

void processData() {
  StaticJsonDocument<1024> doc;

  // GPS Data
  if (gps.location.isValid()) {
    doc["latitude"] = gps.location.lat();
    doc["longitude"] = gps.location.lng();
    doc["altitude"] = gps.altitude.meters();
    doc["speed"] = gps.speed.kmph();
  }

  // Timestamp
  if (gps.date.isValid() && gps.time.isValid()) {
    char timestamp[25];
    snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02dT%02d:%02d:%02dZ",
             gps.date.year(), gps.date.month(), gps.date.day(),
             gps.time.hour(), gps.time.minute(), gps.time.second());
    doc["timestamp"] = timestamp;
  }

  // IMU Data (calibrated)
  JsonObject imu = doc.createNestedObject("imu");
  imu["accelerometer"]["x"] = mpu.getAccX();
  imu["accelerometer"]["y"] = mpu.getAccY();
  imu["accelerometer"]["z"] = mpu.getAccZ();
  imu["gyroscope"]["x"] = mpu.getGyroX();
  imu["gyroscope"]["y"] = mpu.getGyroY();
  imu["gyroscope"]["z"] = mpu.getGyroZ();

  // Output JSON
  serializeJson(doc, jsonSerial);
  jsonSerial.println();
}
