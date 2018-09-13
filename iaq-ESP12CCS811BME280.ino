/**
   CCS811 and BME-280 Sensors data collecting
   @version 1.2
   @author skitsanos
*/

//Sensors stack
#include "Adafruit_CCS811.h"
#include <Wire.h>
#include "BlueDot_BME280.h"

Adafruit_CCS811 ccs;
BlueDot_BME280 bme;

void (*resetFunc)(void) = 0;

int counter = 0;
int measures = 19;

//HTTP Stack
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

ESP8266WiFiMulti WiFiMulti;
String apiEndPoint = "http://192.168.1.25";

void log(String type, String message)
{
    Serial.println("[" + type + "] " + message);
}

void setup()
{
    Serial.begin(115200);
    Serial.println();

    log("*", "LoggerDaisy iAQ Monitor, ESP12CCS811BME280");

    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP("ACCESS_POINT_NAME", "ACCESS_POINT_PASSWORD");

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
    if (ccs.available())
    {
        ccs.setEnvironmentalData(bme.readHumidity(), bme.readTempC());
        delay(500);

        if (!ccs.readData())
        {
            Serial.println("eCO2, ppm \t TVOC, ppb \t Temp, °C \t Humidity");
            Serial.print(ccs.geteCO2());
            Serial.print(" \t ");
            Serial.print(ccs.getTVOC());
            Serial.print(" \t ");
            Serial.print(bme.readTempC());
            Serial.print(" \t ");
            Serial.println(bme.readHumidity());

            if ((WiFiMulti.run() == WL_CONNECTED))
            {
                log("i", "WIFI connected");
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
                log("e", "There is no WIFI connection.");
            }
        } else
        {
            Serial.println("ERROR!");
            while (1);
        }

        counter++;

        if (counter > measures)
        {
            resetFunc();
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
