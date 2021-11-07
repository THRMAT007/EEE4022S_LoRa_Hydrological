// include the libraries
// Code modified by Matthew Thorburn from FoSSA system github repo: https://github.com/FOSSASystems
#include <FOSSA-Comms.h>
#include <RadioLib.h>

// pin definitions
#define CS                    5      // SPI chip select
#define DIO                   2       // DIO0 for SX127x, DIO1 for SX126x
#define NRST                  14      // NRST pin (optional)
#define BUSY                  4       // BUSY pin (SX126x-only)
SX1278 radio = new Module(CS, DIO, NRST, RADIOLIB_NC);

// satellite callsign
char callsign[] = "FOSSASAT-2";

// last transmission timestamp
uint32_t lastTransmit = 0;

// transmission period in ms
const uint32_t transmitPeriod = 4000;

// interrupt flags
volatile bool receivedFlag = false;
volatile bool enableInterrupt = true;

// interrupt service routine for data reception
void setFlag(void) {
  if(!enableInterrupt) {
    return;
  }

  receivedFlag = true;
}

void setup() {
  Serial.begin(9600);

  // initialize SX1262
  Serial.print(F("Initializing ... "));
  int state = radio.begin(436.7, 125.0, 11, 8, 0x0F0F);
  if (state == ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true);
  }

  // set interrupt service routine
  radio.setDio1Action(setFlag);

  // start listening
  radio.startReceive();
}

void loop() {
  // check if it's time to transmit
  if(millis() - lastTransmit >= transmitPeriod) {
    // disable reception interrupt
    enableInterrupt = false;
    detachInterrupt(digitalPinToInterrupt(2));

    // save timestamp
    lastTransmit = millis();

    Serial.println(F("Transmitting packet ... "));

    // data to transmit
    uint8_t functionId = CMD_RETRANSMIT;



    
    char optData[] = "Hello there";
    uint8_t optDataLen = strlen(optData);

    // build frame
    uint8_t len = FCP_Get_Frame_Length(callsign, optDataLen);
    uint8_t* frame = new uint8_t[len];
    FCP_Encode(frame, callsign, functionId, optDataLen, (uint8_t*)optData);
    PRINT_BUFF(frame, len);

    // send data
    int state = radio.transmit(frame, len);
    delete[] frame;

    // check transmission success
    if (state == ERR_NONE) {
      Serial.println(F("Success!"));
    }

    // set radio mode to reception
    Serial.println(F("Waiting for response ... "));
    radio.setDio1Action(setFlag);
    radio.startReceive();
    enableInterrupt = true;
  }

  // check if new data were received
  if(receivedFlag) {
    // disable reception interrupt
    enableInterrupt = false;
    receivedFlag = false;

    // read received data
    size_t respLen = radio.getPacketLength();
    uint8_t* respFrame = new uint8_t[respLen];
    int state = radio.readData(respFrame, respLen);

    // check reception success
    if (state == ERR_NONE) {
      // print raw data
      Serial.print(F("Received "));
      Serial.print(respLen);
      Serial.println(F(" bytes:"));
      PRINT_BUFF(respFrame, respLen);

      // get function ID
      uint8_t functionId = FCP_Get_FunctionID(callsign, respFrame, respLen);
      Serial.print(F("Function ID: 0x"));
      Serial.println(functionId, HEX);

      // check optional data
      uint8_t respOptDataLen = FCP_Get_OptData_Length(callsign, respFrame, respLen);
      if(respOptDataLen > 0) {
        // frame contains optional data
        uint8_t* respOptData = new uint8_t[respOptDataLen];
        FCP_Get_OptData(callsign, respFrame, respLen, respOptData);

        // print optional data
        Serial.print(F("Optional data ("));
        Serial.print(respOptDataLen);
        Serial.println(F(" bytes):"));
        PRINT_BUFF(respOptData, respOptDataLen);
        delete[] respOptData;
      }

    } else {
      Serial.println(F("Reception failed, code "));
      Serial.println(state);

    }

    // enable reception interrupt
    delete[] respFrame;
    enableInterrupt = true;
  }
}
