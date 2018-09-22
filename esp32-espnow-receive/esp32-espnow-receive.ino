/*
 * Receives data from devices that broadcasts via ESP-NOW
 * @version 1.0
 * @author skitsanos
 *
 * Note: This sample is for ESP32 devices
 */

#include <esp_now.h>
#include <WiFi.h>

#define WIFI_CHANNEL 1

struct __attribute__((packed)) SENSOR_DATA {
    float temp;
} sensorData;

void setup()
{
    Serial.begin(115200);
    Serial.println();

    WiFi.mode(WIFI_AP);
    Serial.println(WiFi.softAPmacAddress());
    WiFi.disconnect();

    if (esp_now_init() == ESP_OK)
    {
        Serial.println("ESPNow Init Success!");
    } else
    {
        Serial.println("ESPNow Init Failed....");
    }

    esp_now_register_recv_cb(OnDataRecv);

}

void loop()
{
    yield();
}

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
    memcpy(&sensorData, data, sizeof(sensorData));

    Serial.println(sensorData.temp);
}
