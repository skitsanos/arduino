# Arduino code bits
Code examples to test sensors used in various projects

### Embedded web server with SPIFFS support
- [webserver.ino](https://github.com/skitsanos/arduino/blob/master/webserver/webserver.ino)

### ESP8266/NodeMCU with AMS CCS811 and Bosch BME-280 sensors

- [iaq-ESP12CCS811BME280.ino](https://github.com/skitsanos/arduino/blob/master/iaq-ESP12CCS811BME280.ino) - collects data from CCS811 and BME-280 sensors and sends them over via HTTP GET request
- [iaq-ESP12CCS811BME280httpd.ino](https://github.com/skitsanos/arduino/blob/master/iaq-ESP12CCS811BME280httpd.ino) - same as above, with addition of dual-mode WIFI and HTTP server.

**Notes**
- The equivalent CO2 (eCO2) output range for CCS811 is from 400ppm to 8192ppm. Values outside this range are clipped.
- The Total Volatile Organic Compound (TVOC) output range for CCS811 is from 0ppb to 1187ppb. Values outside this range are clipped. This is calibrated to a typical TVOC mixture in an indoor environment. If the ratio of compounds in the environment is significantly different the TVOC output will be affected as some VOC compounds will have greater or lesser effect on the sensor


### References

- [Arduino IDE](https://www.arduino.cc/en/Main/Software)
- [Tutorial Arduino Firmware Upload on ESP8266](http://www.esp8266.nu/index.php/Tutorial_Arduino_Firmware_Upload)
