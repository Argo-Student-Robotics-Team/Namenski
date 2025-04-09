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
#include <MPU6050.h>
#include <SoftwareSerial.h>

#define GPS_BAUDRATE 38400    // NEO-M9N default baudrate
#define JSON_BAUDRATE 9600    // Output baudrate
#define GPS_RX_PIN D5         // GPIO14 (D5) for GPS RX
#define JSON_TX_PIN D6         // GPIO12 (D6) for JSON output

TinyGPSPlus gps;
MPU6050 mpu;
SoftwareSerial jsonSerial(JSON_TX_PIN, 255);  // TX-only serial for JSON

void setup() {
  Serial.begin(GPS_BAUDRATE);   // Hardware UART for GPS (RX=D6 not used)
  jsonSerial.begin(JSON_BAUDRATE); // Software serial for output
  Wire.begin(D2, D1);          // ESP8266 I2C (SDA=D2, SCL=D1)
  
  // Initialize MPU6050
  mpu.initialize();
  if(!mpu.testConnection()) {
    jsonSerial.println("{\"error\":\"MPU6050 fail\"}");
    while(1);
  }
}

void loop() {
  // Process GPS data
  while(Serial.available() > 0) {
    if(gps.encode(Serial.read())) {
      processData();
    }
  }

  // GPS timeout check
  if(millis() > 5000 && gps.charsProcessed() < 10) {
    jsonSerial.println("{\"error\":\"No GPS\"}");
    delay(1000);
  }
}

void processData() {
  StaticJsonDocument<512> doc;

  // GPS Data
  if(gps.location.isValid()) {
    doc["latitude"] = gps.location.lat();
    doc["longitude"] = gps.location.lng();
    doc["altitude"] = gps.altitude.meters();
    doc["speed"] = gps.speed.kmph();
  }

  if(gps.date.isValid() && gps.time.isValid()) {
    char timestamp[25];
    snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02dT%02d:%02d:%02dZ",
             gps.date.year(), gps.date.month(), gps.date.day(),
             gps.time.hour(), gps.time.minute(), gps.time.second());
    doc["timestamp"] = timestamp;
  }

  // IMU Data
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  
  JsonObject imu = doc.createNestedObject("imu");
  imu["accelerometer"]["x"] = ax;
  imu["accelerometer"]["y"] = ay;
  imu["accelerometer"]["z"] = az;
  
  imu["gyroscope"]["x"] = gx;
  imu["gyroscope"]["y"] = gy;
  imu["gyroscope"]["z"] = gz;

  // Serialize and output
  String output;
  serializeJson(doc, output);
  jsonSerial.println(output);
}
