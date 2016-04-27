/*
 * iAQ Engine via I2C
 * Requires 4.7KOhm resistors on SDA and SCL
 * @author skitsanos (http://skitsanos.com)
 * Wiring:
 * SCL - A5
 * SDA - A4
 * VCC - 5V
 * GND - GND
 */

#include <Wire.h>
 
#define IAQ_ADDRESS 0x5A

uint16_t predict;
uint8_t statu;
int32_t resistance;
uint16_t tvoc;

void setup() {
  Serial.begin(115200);
  Serial.println("iAQ Engine via I2C");

  Wire.begin();
}

void loop()
{     
  readAllBytes();
  checkStatus();

  Serial.print("CO2eq:");
  Serial.print(predict);
  Serial.print(", Status:");
  Serial.print(statu, HEX);
  Serial.print(", Resistance:");
  Serial.print(resistance);
  Serial.print(", TVoC:");
  Serial.println(tvoc);

  delay(2000);
}
void readAllBytes() 
{
  Wire.requestFrom(IAQ_ADDRESS, 9);

  predict = (Wire.read()<< 8 | Wire.read()); 
  statu = Wire.read();
  resistance = (Wire.read()& 0x00)| (Wire.read()<<16)| (Wire.read()<<8| Wire.read());
  tvoc = (Wire.read()<<8 | Wire.read());
}

void checkStatus()
{
  if(statu == 0x10)
  {
    Serial.println("Warming up...");
  }
  else if(statu == 0x00)
  {
    Serial.println("Ready");  
  }
  else if(statu == 0x01)
  {
    Serial.println("Busy");  
  }
  else if(statu == 0x80)
  {
    Serial.println("Error");  
  }
  else
  Serial.println("No Status, check module");  
}
