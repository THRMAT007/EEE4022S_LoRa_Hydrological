/*
  Blink onboard LED at 0.1 second interval
*/
#include<DHT.h>                     //Library for using DHT sensor 
#define DHTPIN PB1 
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);     //initilize object dht for class DHT with DHT pin with STM32 and DHT type as DHT11
 
void setup() {
  // initialize digital pin PB2 as an output.
  pinMode(PC13, OUTPUT); // LED connect to pin PC13
  dht.begin();                                  
}

void loop() {
  Serial.begin(115200);
  digitalWrite(PC13, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(100);               // wait for 100mS
  digitalWrite(PC13, LOW);    // turn the LED off by making the voltage LOW
  delay(100);               // wait for 100mS
  float h = dht.readHumidity();       //Gets Humidity value
  float t = dht.readTemperature();    //Gets Temperature value
  Serial.printf("Temp: ");
  Serial.println(t);
  Serial.printf("Humid: ");
  Serial.println(h);
  delay(500);
}
