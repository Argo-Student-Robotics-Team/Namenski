#include <TinyGPS++.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <MPU6050_light.h>

#define GPS_BAUDRATE 38400    // NEO-M9N default baudrate
#define GPS_RX_PIN D5         // GPS TX → ESP8266 D5 (GPIO14)

TinyGPSPlus gps;
MPU6050 mpu(Wire);
SoftwareSerial gpsSerial(GPS_RX_PIN, 255); // RX-only for GPS

void setup() {
  Serial.begin(115200);       // Serial Monitor for JSON output
  gpsSerial.begin(GPS_BAUDRATE);
  
  Wire.begin(D2, D1);         // I2C: SDA=D2, SCL=D1
  mpu.begin();
  mpu.calcGyroOffsets();      // Auto-calibrate IMU

  Serial.println("\n\nSystem ready - waiting for GPS...");
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
    Serial.println("{\"error\":\"No GPS signal\"}");
    delay(1000);
  }
}

void processData() {
  StaticJsonDocument<512> doc;

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

  // IMU Data
  mpu.update();
  JsonObject imu = doc.createNestedObject("imu");
  imu["accelerometer"]["x"] = mpu.getAccX();
  imu["accelerometer"]["y"] = mpu.getAccY();
  imu["accelerometer"]["z"] = mpu.getAccZ();
  imu["gyroscope"]["x"] = mpu.getGyroX();
  imu["gyroscope"]["y"] = mpu.getGyroY();
  imu["gyroscope"]["z"] = mpu.getGyroZ();

  // Print to Serial Monitor
  serializeJsonPretty(doc, Serial); // "Pretty" format for readability
  Serial.println("\n-------------------");
}
