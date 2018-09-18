### ESP8266 Webserver Example

Serves eCO2, TVOC, temperature and humidity data to remote API end-point and via inbound http requests.

**What it does**

- Unit runs in dual mode, as Access Point as a WIFI client;
- Using file system (SPIFFS) to serve web content and sensors data;
- AMS CCS811 and Bosch BME280 sensors used to collect data;
- Data collected from sensors stored to sensors-data.js file so it can be collected later on http request;
- Sends sensors data via HTTP to remote API end-point

**Dependenscies**

- FS
- Adafruit_CCS811
- Wire
- BlueDot_BME280
- ESP8266WiFi
- WiFiClient
- ESP8266HTTPClient
- ESP8266WebServer
