/*
    Compiled for the NODEMCU-32S
 */

#include <esp_now.h>
#include <WiFi.h>

#define WIFI_CHANNEL 1
esp_now_peer_info_t slave;
uint8_t remoteMac[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

const uint8_t maxDataFrameSize = 200;
const esp_now_peer_info_t *peer = &slave;
uint8_t dataToSend[maxDataFrameSize];
byte cnt = 0;

struct __attribute__((packed)) ESPNOW_DATA
{
    uint8_t type;
    uint8_t msg;
} espnow_data;

void setup()
{
    Serial.begin(115200);
    Serial.println();

    WiFi.mode(WIFI_STA);
    Serial.println(WiFi.softAPmacAddress());

    WiFi.disconnect();
    if (esp_now_init() == ESP_OK)
    {
        Serial.println("ESP NOW INIT!");
    } else
    {
        Serial.println("ESP NOW INIT FAILED....");
    }

    memcpy(&slave.peer_addr, &remoteMac, 6);
    slave.channel = WIFI_CHANNEL;
    slave.encrypt = 0;

    if (esp_now_add_peer(peer) == ESP_OK)
    {
        Serial.println("Added Peer!");
    }

    esp_now_register_send_cb(OnDataSent);
}

void loop()
{
    float deviceTemperature = temperatureRead();
    uint8_t msg = (uint8_t) deviceTemperature;

    espnow_data.type = 'c';
    espnow_data.msg = msg;

    uint8_t bs[sizeof(espnow_data)];
    memcpy(bs, &espnow_data, sizeof(espnow_data));

    if (esp_now_send(slave.peer_addr, bs, sizeof(espnow_data)) == ESP_OK)
    {
        Serial.println("Sent: " + String(deviceTemperature));
    } else
    {
        Serial.printf("\r\nDID NOT SEND....");
    }
    delay(250);
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    Serial.print("\r\nLast Packet Send Status:\t");
    Serial.print(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
