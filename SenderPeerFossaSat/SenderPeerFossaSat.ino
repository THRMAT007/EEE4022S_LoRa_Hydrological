#include <SPI.h>
#include <LoRa_STM32.h>
#include <Wire.h>
#include <FOSSA-Comms.h>
#include <RadioLib.h>

 //LoRa module pins
#define SS PA4
#define RST PB0
#define DI0 PA3
#define DI1 PA2

// modem configuration
#define LORA_FREQUENCY        436.7   // MHz
#define FSK_FREQUENCY         436.9   // MHz
#define BANDWIDTH             125.0   // kHz
#define SPREADING_FACTOR      11      // -
#define CODING_RATE           8       // 4/8
#define SYNC_WORD             0x12    // used as LoRa "sync word", or twice repeated as FSK sync word (0x1212)
#define OUTPUT_POWER          17      // dBm

SX1278 radio = new Module(CS, DIO, NRST, RADIOLIB_NC);
char callsign[] = "FOSSASAT-2";


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
   
  //Serial.begin(115200);
  //while (!Serial);
 
  //Serial.println("LoRa Sender");
 
  LoRa.setTxPower(TX_P);
  LoRa.setSyncWord(ENCRYPT);
  LoRa.setCodingRate4(CodingRate);
  LoRa.setSpreadingFactor(SpreadingFactor);
  
  LoRa.setPins(SS, RST, DI0);
  if (!LoRa.begin(BAND)) 
  {
    //Serial.println("Starting LoRa failed!");
    while (1);
  }
  dht.begin();
  pinMode(pwr, OUTPUT);
  digitalWrite(pwr, LOW);
  //pinMode(data , INPUT);
}
 
void loop() {
  //Serial.print("Sending packet: ");
  //Serial.println(counter);
  delay(2000);
  float h = dht.readHumidity();       //Gets Humidity value
  float t = dht.readTemperature(); 
  delay(2000);
  digitalWrite(pwr,HIGH);
  delay(500);
  float moisture = (analogRead(data)/4096)*3.3;
  delay(500);
  digitalWrite(pwr,LOW);
  //float h = 43.2;       //Gets Humidity value
  //float t = 27.5; //Gets Temperature value
  //Serial.print(t);
  // send packet
  LoRa.beginPacket();
  //LoRa.print("hello ");
  //LoRa.print(counter);
  message = String(counter)+ "," +String(t) + "," + String(h)+","+String(moisture);
  LoRa.print(message);
  LoRa.endPacket();
 
  counter++;
 
  delay(5000);
}
