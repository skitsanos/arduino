#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

int scanTime = 15; //In seconds

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
      if (advertisedDevice.haveServiceUUID())
      {
        Serial.println();
        Serial.println(advertisedDevice.getServiceUUID().to128().toString().c_str());

        if (advertisedDevice.haveServiceData())
        {
          Serial.println();
          Serial.println(advertisedDevice.getServiceDataUUID().to128().toString().c_str());
          Serial.println(advertisedDevice.getServiceData().c_str());
        }
      }


      BLEAddress macFound = advertisedDevice.getAddress();
      BLEAddress macNeeded =  BLEAddress("12:3b:6a:1b:15:b4");

      Serial.println(macFound.toString().c_str());

      if (macFound.equals(macNeeded)) {
        Serial.println("found the key");
      }
    }
};

void bleScan() {

  BLEScan* pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(false); //active scan uses more power, but get results faster
  BLEScanResults foundDevices = pBLEScan->start(scanTime);
  Serial.print("Devices found: ");
  Serial.println(foundDevices.getCount());
  Serial.println("Scan done!");
}

void setup() {
  Serial.begin(115200);
  Serial.println("Scanning...");

  BLEDevice::init("");

  bleScan();
}

void loop() {
  // put your main code here, to run repeatedly:
  bleScan();
  delay(15000);
}
