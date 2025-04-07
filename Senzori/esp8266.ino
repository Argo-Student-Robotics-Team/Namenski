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
