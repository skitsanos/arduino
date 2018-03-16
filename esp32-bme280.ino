/*
 * ESP32+BME280
 * Sends JSON packets to remote API with data collected from BME-280 sensor.
 * Uses BME280-I2C-ESP32 lib from https://github.com/Takatsuki0204/BME280-I2C-ESP32
 */
 
#include <WiFi.h>
#include <ArduinoJson.h>

#include "Adafruit_BME280.h"

#define I2C_SDA 21
#define I2C_SCL 22
#define BME280_ADDRESS 0x76
#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme(I2C_SDA, I2C_SCL);

//WIFI Access credentials:
const char *ssid = "PUT_YOUR_SSD_HERE";
const char *password = "YOUR_SSD_PASSWORD";

void ping()
{
  Serial.println("Collecting data");

  //Read sensors and generate JSON out of it

  Serial.print("Temperature = ");
  Serial.print(bme.readTemperature());
  Serial.println(" *C");

  Serial.print("Pressure = ");

  Serial.print(bme.readPressure() / 100.0F);
  Serial.println(" hPa");

  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  Serial.print("Humidity = ");
  Serial.print(bme.readHumidity());
  Serial.println(" %");

  Serial.println();

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& doc = jsonBuffer.createObject();
  doc["mac"] = String(WiFi.macAddress());
  doc["type"] = "esp32iaq";
  doc["temperature"] = bme.readTemperature();
  doc["humidity"] = bme.readHumidity();
  doc["pressure"] = bme.readPressure() / 100.0F;
  doc["voc"] = 0;

  int length = doc.measureLength();

  Serial.println("Trying API ping...");

  WiFiClient client;
  const int httpPort = 3000;
  const char *host = "192.168.1.21";
  const String url = "/api/device-ping";

  if (!client.connect(host, httpPort)) {
    Serial.println("API connection failed");
    return;
  }

  client.println(String("POST ") + url + " HTTP/1.1");
  client.println("Host: " + String(host));
  client.println("User-Agent: LoggerDaisy Sensor");
  client.println("Content-Type: application/json");
  client.println("Content-Length: " + String(length));
  client.println(); //end of headers

  String json;
  doc.printTo(json);
  Serial.println(json);
  client.println(json);

  unsigned long timeout = millis();
  while (client.available() == 0)
  {
    if (millis() - timeout > 5000) {
      Serial.println("API ping Timeout : (");
      client.stop();
      return;
    }
  }

  client.stop();

  Serial.println("API ping done");
}

void setup() {
  Serial.begin(115200);

  bool bme_status = bme.begin(BME280_ADDRESS);
  if (!bme_status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Trying WIFI...");
    delay(500);
  }
}

void loop() {
  ping();
  delay(10000);
}
