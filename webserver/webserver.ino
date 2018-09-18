/*
 * Embedded web server
 * @version 1.0
 * @author skitsanos
 */

//Filesystem support
#include "FS.h"

//HTTP Stack
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);

const char *ssid = "...";
const char *password = "...";

//Utils
void (*resetFunc)(void) = 0;

int debug_verbose = 1;

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

void setup()
{
    Serial.begin(115200);
    Serial.println();

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
}

void loop()
{
    //handle HTTP requests
    server.handleClient();
}
