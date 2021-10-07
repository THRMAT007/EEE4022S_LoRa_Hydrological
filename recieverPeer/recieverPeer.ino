
#include <SPI.h>
#include <LoRa.h>

//define the pins used by the transceiver module
#define SS 5
#define RST 14
#define DI0 2


#define TX_P 17
#define BAND 433E6
#define ENCRYPT 0x35

void setup() 
{
  Serial.begin(115200);
  while (!Serial);
 
  Serial.println("LoRa Receiver");
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
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");
 
    // read packet
    while (LoRa.available()) {
      Serial.print((char)LoRa.read());
    }
 
    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
}
