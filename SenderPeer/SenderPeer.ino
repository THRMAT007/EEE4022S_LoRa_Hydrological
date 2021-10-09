#include <SPI.h>
#include <LoRa_STM32.h>
 
#define SS PA4
#define RST PB0
#define DI0 PA1
 
#define TX_P 17
#define BAND 433E6
#define ENCRYPT 0x35

#include<DHT.h>                     //Library for using DHT sensor 
#define DHTPIN PA0 
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
 
int counter = 0;
String message = ""; 
void setup() {
  dht.begin(); 
  Serial.begin(115200);
  while (!Serial);
 
  Serial.println("LoRa Sender");
 
  LoRa.setTxPower(TX_P);
  LoRa.setSyncWord(ENCRYPT);
  
  LoRa.setPins(SS, RST, DI0);
  if (!LoRa.begin(BAND)) 
  {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}
 
void loop() {
  Serial.print("Sending packet: ");
  Serial.println(counter);
  
  float h = dht.readHumidity();       //Gets Humidity value
  float t = dht.readTemperature();    //Gets Temperature value
  //Serial.print(t);
  // send packet
  LoRa.beginPacket();
  //LoRa.print("hello ");
  //LoRa.print(counter);
  message = String(counter)+ "," +String(t) + "," + String(h);
  LoRa.print(message);
  LoRa.endPacket();
 
  counter++;
 
  delay(5000);
}
