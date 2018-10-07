#include <esp_now.h>
#include <WiFi.h>
#include "SSD1306.h"

#define I2C_SDA 5
#define I2C_SCL 4

SSD1306 display(0x3c, 5, 4);

#define WIFI_CHANNEL 1

const char *ssid = "Skitsanos";
const char *password = "chagsameah";

struct __attribute__((packed)) ESPNOW_DATA
{
    char type;
    uint8_t *msg;
} espnow_data;


String mac2String(char *addr)
{
    static char str[18];

    if (addr == NULL) return "";

    snprintf(str, sizeof(str), "%02x:%02x:%02x:%02x:%02x:%02x", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    return str;
}

void showLogo()
{
    display.clear();
    //display.drawXbm(32, 0, logo_width, logo_height, logo_bits);
    display.display();

    delay(500);
}

void showWifiIcon()
{
    display.drawVerticalLine(1, 6, 2);
    display.drawVerticalLine(3, 4, 4);
    display.drawVerticalLine(5, 2, 6);
    display.drawVerticalLine(7, 0, 8);
    display.display();
}

void showNoWifiIcon()
{
    display.drawVerticalLine(1, 6, 2);
    display.drawVerticalLine(3, 4, 4);
    display.drawVerticalLine(5, 2, 6);
    display.drawVerticalLine(7, 0, 8);
    display.drawHorizontalLine(0, 1, 4);
    display.display();
}

void showProgress(uint8_t progress, String text)
{
    display.clear();
    display.drawString(64, 10, "LOGGERDAISY");
    display.drawProgressBar(20, 24, 88, 6, progress);
    display.drawString(64, 39, text);
    display.display();
}

void showStatus(String text)
{
    display.clear();
    display.drawString(64, 10, "LOGGERDAISY");
    display.drawHorizontalLine(20, 24, 88);
    display.drawString(64, 30, text);
    display.display();

    showWifiIcon();
}

void showSensors()
{
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 10, "LOGGERDAISY");
    display.drawHorizontalLine(20, 24, 88);

    display.setTextAlignment(TEXT_ALIGN_LEFT);
    //display.drawString(10, 30, String(bme.readTemperature()) + "°C");

    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    //display.drawString(118, 30, String(bme.readPressure() / 100.0F) + "hPa");

    display.setTextAlignment(TEXT_ALIGN_LEFT);
    //display.drawString(10, 44, String(bme.readHumidity()) + "%");

    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    //display.drawString(118, 44, String(mhz19.getPPM()) + "ppm");

    display.display();

    showWifiIcon();
}

void showEspNowPackets(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 10, "LOGGERDAISY");
    display.drawHorizontalLine(20, 24, 88);

    display.drawString(64, 30, "Received " + String(data_len) + " bytes");
    display.drawString(64, 44, "from " + mac2String((char *) mac_addr));

    showWifiIcon();
}

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
    memcpy(&espnow_data, data, sizeof(espnow_data));

    Serial.println(espnow_data.type);
    Serial.println((char *) espnow_data.msg);

    showEspNowPackets(mac_addr, data, data_len);
}

void setup()
{
    Serial.begin(115200);
    Serial.println();

    WiFi.mode(WIFI_AP_STA);
    Serial.println(WiFi.softAPmacAddress());

    // Initialising the UI will init the display too.
    display.init();

    display.flipScreenVertically();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_10);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        showProgress(10, "Trying WIFI...");
        showNoWifiIcon();
        delay(500);
    }

    if (esp_now_init() == ESP_OK)
    {
        Serial.println("ESPNow Init Success!");
        showProgress(40, "ESP-NOW Enabled");
    } else
    {
        Serial.println("ESPNow Init Failed....");
        showProgress(40, "ESP-NOW Failed");
    }

    esp_now_register_recv_cb(OnDataRecv);

    showStatus("Ready " + mac2String((char *) ESP.getEfuseMac()));

    showWifiIcon();
}

void loop()
{
    yield();
}
