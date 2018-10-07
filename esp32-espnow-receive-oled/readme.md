### ESP-Now packets receiver for WeMos Oled board

### Sourcing the board

- [Aliexpress](https://www.aliexpress.com/item/Lolin-ESP32-OLED-wemos-for-Arduino-ESP32-OLED-WiFi-Modules-Bluetooth-Dual-ESP-32-ESP-32S/32807531243.html?spm=2114.10010108.addToCart.4.ee411779VtIfZg&gps-id=pcDetailCartBuyAlsoBuy&scm=1007.12908.99722.0&scm_id=1007.12908.99722.0&scm-url=1007.12908.99722.0&pvid=7346d898-298c-4ce5-975a-7cf881ffa7ea), you can search for esp32 oled and you will get few variants at least
- [Bangood](https://www.banggood.com/Wemos-ESP32-OLED-Module-For-Arduino-ESP32-OLED-WiFi-Bluetooth-Dual-ESP-32-ESP-32S-ESP8266-p-1148119.html?rmmds=search)

### Display init:



```c++
#define I2C_SDA 5
#define I2C_SCL 4

SSD1306 display(0x3c, 5, 4);
```
