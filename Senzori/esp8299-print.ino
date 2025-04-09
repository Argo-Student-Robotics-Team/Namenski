#include <TinyGPS++.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <MPU6050_light.h>

#define GPS_BAUDRATE 38400
#define GPS_RX_PIN D5  // GPS TX → ESP8266 D5 (GPIO14)

TinyGPSPlus gps;
MPU6050 mpu(Wire);

void setup() {
  Serial.begin(115200);  // For Serial Monitor
  Serial1.begin(GPS_BAUDRATE, SERIAL_8N1, GPS_RX_PIN, 255); // Hardware UART on D5
  
  Wire.begin(D2, D1);    // I2C: SDA=D2 (GPIO4), SCL=D1 (GPIO5)
  mpu.begin();
  mpu.calcGyroOffsets();

  Serial.println("\nSystem ready - waiting for GPS...");
}

void loop() {
  // Read GPS data from Serial1
  while (Serial1.available() > 0) {
    if (gps.encode(Serial1.read())) {
      processData();
    }
  }

  // ... rest of your code ...
}
