#include <SPI.h>
#include <LoRa_STM32.h>
#include <Wire.h>

 //LoRa module pins
#define CS PA4
#define RST PB0
#define DI0 PA3
#define DI1 PA2

//LoRa parametes 
#define TX_P 17
#define BAND 433E6
#define ENCRYPT 0x35
#define SpreadingFactor 12
#define CodingRate 8

//DHT parameters
#include<DHT.h>                     //Library for using DHT sensor 
#define DHTPIN PB1 
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

//HW-390 parameters
#define pwr PA1
#define data PA0 
int counter = 0;
String message = ""; 

void setup() {
  Serial.begin(115200);
  Serial.printf("init: ");
  pinMode(pwr, OUTPUT);
  pinMode(PC13, OUTPUT);
  digitalWrite(pwr, LOW);
  digitalWrite(PC13, LOW);
  //Serial.println("LoRa Sender");
 
  LoRa.setTxPower(TX_P);
  LoRa.setSyncWord(ENCRYPT);
  LoRa.setCodingRate4(CodingRate);
  LoRa.setSpreadingFactor(SpreadingFactor);
  
  LoRa.setPins(CS, RST, DI0);
  if (!LoRa.begin(BAND)) 
  {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
    // initialize digital pin PB2 as an output.
  dht.begin();
}
 
void loop() {
  digitalWrite(PC13, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(2000);      
  digitalWrite(PC13, LOW); // turn the LED off by making the voltage LOW
  delay(100);   
  float h = dht.readHumidity();       //Gets Humidity value
  float t = dht.readTemperature();  //Gets Temperature value
  digitalWrite(pwr, LOW);
  delay(100);  
  float m = (analogRead(data)/4096)*3.3;
  delay(100);
  digitalWrite(pwr, LOW);
  //Serial.printf("Temp: ");
  //Serial.println(t);
  //Serial.printf("Humid: ");
  
  //Serial.println(h);
  //Serial.printf("mositure: ");
  //Serial.println(m);
  //Serial.print("Sending packet: ");
  LoRa.beginPacket();
  //LoRa.print("hello ");
  //LoRa.print(counter);
  message = "FossaSat-2&CMD_RETRANSMITT&MT%"+String(counter)+ "%" +String(t) + "%" + String(h)+"%"+String(m)+'%';
  Serial.println(message);
  LoRa.print(message);
  LoRa.endPacket();
         
  counter++;
 
  delay(120000);
}
