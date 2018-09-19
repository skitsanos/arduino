#include <Arduino.h>

//Bluetooth support
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID        "e12a9786-8a1a-41c5-b948-136424696777" //https://www.uuidgenerator.net/version4
#define CHARACTERISTIC_UUID "df7651c0-ae49-4840-9265-8018f4f9b3e0"

uint64_t chipid;

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

String getMac()
{
    String MACString = "";
    uint64_t chipId = ESP.getEfuseMac();
    for (int i = 0; i < 6; i++)
    {
        if (i > 0) MACString.concat(":");
        uint8_t Octet = chipId >> (i * 8);
        if (Octet < 16) MACString.concat("0");
        MACString.concat(String(Octet, HEX));
    }
    return MACString;
}

uint32_t getFlashSize()
{
    return ESP.getFlashChipSize();
}

void setup()
{
    esp_chip_info_t chip_info;

    Serial.begin(115200);
    Serial.println();

    log("!", "Booting...");
    log("i", "MAC: " + getMac());
    log("i", "Flash: " + String(getFlashSize()) + " bytes");

    BLEDevice::init("LoggerDaisy [Node32]");
    BLEServer *pServer = BLEDevice::createServer();
    BLEService *pService = pServer->createService(SERVICE_UUID);
    BLECharacteristic *pCharacteristic = pService->createCharacteristic(
            CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE
    );

    pCharacteristic->setValue("Hello World says Neil");
    pService->start();
    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->start();
    log("i","Characteristic defined! Now you can read it in your phone!");
}

void loop()
{

}
