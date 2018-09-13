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
int counter = 0;
const int measures = 19;

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
    Serial.println("[" + type + "] " + message);
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

void handler_NotFound()
{
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++)
    {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
}


//Application
void setup()
{
    Serial.begin(115200);
    Serial.println();

    log("*", appTitle);

    //WiFi.mode(WIFI_STA);
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("LoggerDaisy [Node]", "012345678");
    WiFi.softAPIP();
    WiFi.printDiag(Serial);
    log("i", WiFi.softAPIP().toString());

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        log(">", ".");
    }

    log("i", "Local IP address:");
    log("-", WiFi.localIP().toString());

    //HTTP Server Requests handlers
    server.on("/", handler_Root);
    server.on("/reset", handler_Reset);
    server.onNotFound(handler_NotFound);
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
        log("e", "BME280 Sensor detected!");
    }
}

void loop()
{
    //handle HTTP requests
    server.handleClient();

    if (ccs.available())
    {
        ccs.setEnvironmentalData(bme.readHumidity(), bme.readTempC());
        delay(500);

        if (!ccs.readData())
        {
            log(">", "eCO2, ppm \t TVOC, ppb \t Temp, °C \t Humidity");
            log(">",
                String(ccs.geteCO2()) + " \t " +
                String(ccs.getTVOC()) + " \t " +
                String(bme.readTempC()) + " \t " +
                String(bme.readHumidity()));

            log("i", "Sending data...");

            HTTPClient http;
            http.begin(apiEndPoint
                       + "/ping/?temperature="
                       + bme.readTempC()
                       + "&humidity=" + bme.readHumidity()
                       + "&eco2=" + ccs.geteCO2()
                       + "&tvoc=" + ccs.getTVOC());
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
        } else
        {
            Serial.println("ERROR!");
            ESP.reset();
            //while (1);
        }

        counter++;

        //in case if we need to restart the devices after n-measures performed
        if (counter > measures)
        {
            //ESP.reset();
        }
    }

    //skip first 2 values because sensors are in 'reheat' mode and we can't rely on their data
    if (counter < 2)
    {
        delay(500);
    } else
    {
        delay(4500);
    }
}
