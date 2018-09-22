#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLEUUID.h>

BLEServer *pServer = NULL;
BLECharacteristic *temperatureCharacteristic = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;

class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;
    };

    void onDisconnect(BLEServer *pServer)
    {
        deviceConnected = false;
    }
};

//https://www.bluetooth.com/specifications/gatt/services
//https://www.bluetooth.com/specifications/gatt/characteristics
//https://www.bluetooth.com/specifications/assigned-numbers/environmental-sensing-service-characteristics
//https://www.hackster.io/ManniAT/esp32-ble-environmental-gatt-server-14cceb
//https://nordicsemiconductor.github.io/Nordic-Thingy52-FW/documentation/firmware_architecture.html

void setup()
{
    Serial.begin(115200);

    // Create the BLE Device
    BLEDevice::init("LoggerDaisy [blegate]");

    // Create the BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create the BLE Service
    BLEService *pService = pServer->createService(BLEUUID((uint16_t) 0x181A));

    // Create a BLE Characteristic (Temperature sensing)
    temperatureCharacteristic = pService->createCharacteristic(BLEUUID((uint16_t) 0x2A6E),
                                                     BLECharacteristic::PROPERTY_READ |
                                                     BLECharacteristic::PROPERTY_NOTIFY);

    // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
    // Create a BLE Descriptor
    temperatureCharacteristic->addDescriptor(new BLE2902());

    // Start the service
    pService->start();

    // Start advertising
    pServer->getAdvertising()->start();
    Serial.println("Waiting a client connection to notify...");
}

void loop()
{
    // notify changed value
    if (deviceConnected)
    {
        float degree = temperatureRead();
        temperatureCharacteristic->setValue(degree);
        temperatureCharacteristic->notify();

        delay(100); // bluetooth stack will go into congestion, if too many packets are sent
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected)
    {
        delay(1000); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }

    // connecting
    if (deviceConnected && !oldDeviceConnected)
    {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}
