/*

Ovaj kod salje podatke sa IMU i GNSS na Serial, u vidu json stringa. Namenjen je za **ESP 32** i NEO M6 GNSS

Ovo je primer podatka koji se salje:
{"latitude":37.7749,"longitude":-122.4194,"altitude":10.5,"speed":5.2,"timestamp":"2023-10-05 14:30:45","imu":{"accelerometer":{"x":1234,"y":5678,"z":9101},"gyroscope":{"x":2345,"y":6789,"z":1011},"temperature":25.3}}

Testiracu ga 10.3. ili 11.3. i okacicu snimak negde.

Potencijalne izmene, u zavisnosti od toga koje komponente koristimo:
-dodati podatke sa dosta ultrasonic senzora (bice potrebno jos oloca vrv)
-dodati podatke sa 2d lidara

*/


#include <TinyGPS++.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <MPU6050.h>

#define GPS_BAUDRATE 9600

TinyGPSPlus gps;
MPU6050 mpu;

void setup() {
  Serial.begin(9600);
  Serial2.begin(GPS_BAUDRATE);  // Serial2 for GPS UART

  // Initialize the MPU6050
  Wire.begin();
  mpu.initialize();

  // Check if the MPU6050 is connected
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed. Check your wiring!");
    while (1);
  }

  Serial.println(F("ESP32 - GPS and MPU6050 IMU Simulation"));
}

void loop() {
  
  while (Serial2.available() > 0) {
    gps.encode(Serial2.read());
  }

  
  StaticJsonDocument<512> doc;

  
  if (gps.location.isUpdated()) {
    doc["latitude"] = gps.location.isValid() ? gps.location.lat() : 0.0;
    doc["longitude"] = gps.location.isValid() ? gps.location.lng() : 0.0;
    doc["altitude"] = gps.altitude.isValid() ? gps.altitude.meters() : 0.0;
    doc["speed"] = gps.speed.isValid() ? gps.speed.kmph() : 0.0;

    if (gps.date.isValid() && gps.time.isValid()) {
      char timestamp[20];
      snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02d %02d:%02d:%02d",
               gps.date.year(), gps.date.month(), gps.date.day(),
               gps.time.hour(), gps.time.minute(), gps.time.second());
      doc["timestamp"] = timestamp;
    } else {
      doc["timestamp"] = "INVALID";
    }
  }

  // Read MPU6050 data
  int16_t ax, ay, az;  // Accelerometer raw data
  int16_t gx, gy, gz;  // Gyroscope raw data
  int16_t temp;        // Temperature raw data

  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);  // Read accelerometer and gyroscope data
  temp = mpu.getTemperature();                   // Read temperature data

  // Add MPU6050 data to JSON
  doc["imu"]["accelerometer"]["x"] = ax;
  doc["imu"]["accelerometer"]["y"] = ay;
  doc["imu"]["accelerometer"]["z"] = az;

  doc["imu"]["gyroscope"]["x"] = gx;
  doc["imu"]["gyroscope"]["y"] = gy;
  doc["imu"]["gyroscope"]["z"] = gz;

  doc["imu"]["temperature"] = temp / 340.0 + 36.53;  // Convert raw temperature to Celsius

  // Serialize JSON and print it
  String jsonString;
  serializeJson(doc, jsonString);
  Serial.println(jsonString);

  // If no GPS data received after 5 seconds, warn user
  if (millis() > 5000 && gps.charsProcessed() < 10) {
    Serial.println(F("No GPS data received: check wiring"));
  }

  delay(1000);  // Wait 1 second before the next iteration
}
