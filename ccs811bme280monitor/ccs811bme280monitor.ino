/**
   CCS811 and BME-280 Sensors data collecting with WIFI dual mode and HTTP server
   @version 1.2
   @author skitsanos
*/

//Filesystem support
#include "FS.h"

//Sensors stack
#include "Adafruit_CCS811.h"
#include <Wire.h>
#include "BlueDot_BME280.h"

Adafruit_CCS811 ccs;
BlueDot_BME280 bme;

void (*resetFunc)(void) = 0;

String appTitle = "LoggerDaisy iAQ Monitor, ESP12CCS811BME280httpd";
int debug_verbose = 0;

long interval_Sensors = 4500;
long previousTime = 0;

//HTTP Stack
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>

const char *ssid = "...";
const char *password = "...";
const String apiEndPoint = "http://192.168.1.25";

ESP8266WebServer server(80);

//Utils
void log(String type, String message)
{
    if (debug_verbose == 1)
    {
        Serial.println("[" + type + "] " + message);
    } else
    {
        //display only error messages
        if (type == "e")
        {
            Serial.println("[" + type + "] " + message);
        }
    }
}

String getContentType(String filename)
{ // convert the file extension to the MIME type
    if (filename.endsWith(".html")) return "text/html";
    else if (filename.endsWith(".css")) return "text/css";
    else if (filename.endsWith(".js")) return "application/javascript";
    else if (filename.endsWith(".ico")) return "image/x-icon";
    return "text/plain";
}

bool handleFileRead(String path)
{
    // send the right file to the client (if it exists)
    log("-", "handleFileRead: " + path);
    if (path.endsWith("/")) path += "index.html";         // If a folder is requested, send the index file
    String contentType = getContentType(path);            // Get the MIME type
    if (SPIFFS.exists(path))
    {                            // If the file exists
        File file = SPIFFS.open(path, "r");                 // Open it
        size_t sent = server.streamFile(file, contentType); // And send it to the client
        file.close();                                       // Then close the file again
        return true;
    }
    log("-", "File Not Found");
    return false;                                         // If the file doesn't exist, return false
}

void handler_Root()
{
    server.send(200, "text/plain", appTitle);
}

void handler_Reset()
{
    server.send(200, "text/plain", "Resetting the device...");
    ESP.reset();

    //todo: add code for redirecting to root page after 5s on device reset
}

//Application
void setup()
{
    Serial.begin(115200);
    Serial.println();

    log("*", appTitle);
    log("i", "MAC: " + String(WiFi.macAddress()));

    log("i", "Starting filesystem support");
    SPIFFS.begin();

    //WiFi.mode(WIFI_STA);
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("LoggerDaisy [Node]", "012345678");
    WiFi.softAPIP();
    //WiFi.printDiag(Serial);
    log("i", WiFi.softAPIP().toString());

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        log(">", ".");
    }

    log("i", "Local IP address:");
    log("-", WiFi.localIP().toString());

    server.onNotFound([]()
                      {                              // If the client requests any URI
                          if (!handleFileRead(server.uri()))                  // send it if it exists
                              server.send(404, "text/plain",
                                          "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
                      });
    server.begin();

    //Bosch BME280 init
    bme.parameter.communication = 0;
    bme.parameter.I2CAddress = 0x76;
    bme.parameter.sensorMode = 0b11;
    bme.parameter.IIRfilter = 0b100;
    bme.parameter.humidOversampling = 0b101;
    bme.parameter.tempOversampling = 0b101;
    bme.parameter.pressOversampling = 0b101;

    if (bme.init() != 0x60)
    {
        log("e", "Ops! BME280 Sensor not found!");
    }

    //AMS CCS811
    if (!ccs.begin())
    {
        log("e", "Failed to start CCS811 sensor! Please check your wiring.");
        while (1);
    } else
    {
        log("i", "BME280 Sensor detected!");
    }
}

void loop()
{
    //handle HTTP requests
    server.handleClient();

    unsigned long currentTime = millis();
    if (currentTime - previousTime > interval_Sensors)
    {
        previousTime = currentTime;

        //
        if (ccs.available())
        {
            ccs.setEnvironmentalData(bme.readHumidity(), bme.readTempC());
            delay(500);

            if (!ccs.readData())
            {
                //write data to file

                File f = SPIFFS.open("/sensors-data.js", "w");
                if (!f)
                {
                    log("e", "File open failed");
                } else
                {
                    f.println("let sensorsData = [" +
                              String(ccs.geteCO2()) + "," +
                              String(ccs.getTVOC()) + "," +
                              String(bme.readTempC()) + "," +
                              String(bme.readHumidity()) + "];");
                    f.close();
                }

                log(">", "eCO2, ppm \t TVOC, ppb \t Temp, °C \t Humidity");
                log(">",
                    String(ccs.geteCO2()) + " \t " +
                    String(ccs.getTVOC()) + " \t " +
                    String(bme.readTempC()) + " \t " +
                    String(bme.readHumidity()));

                //if eCO2 value and TVOC value are zero, means sensor is down and we wont need to send data request but
                // instead the notification to the server that we are fucked

                if (ccs.geteCO2() == 0 && ccs.getTVOC() == 0)
                {
                    log("e", "There is an issue with the sensor. Restarting");
                    //ESP.reset(); //or not
                } else
                {
                    log("i", "Sending data...");

                    HTTPClient http;
                    http.begin(apiEndPoint
                               + "/ping/?temperature=" + bme.readTempC()
                               + "&humidity=" + bme.readHumidity()
                               + "&eco2=" + ccs.geteCO2()
                               + "&tvoc=" + ccs.getTVOC());

                    //http.sendHeader("X-Device-MAC", String(WiFi.macAddress()));

                    int httpCode = http.GET();

                    if (httpCode > 0)
                    {
                        log("-", "Got server reply:");
                        log("-", String(httpCode));

                        if (httpCode == HTTP_CODE_OK)
                        {
                            log("-", http.getString());
                        }
                    } else
                    {
                        log("e", "Failed to ping remote API");
                    }
                }

            } else
            {
                Serial.println("ERROR!");
                resetFunc();
            }
        }
    }
}
