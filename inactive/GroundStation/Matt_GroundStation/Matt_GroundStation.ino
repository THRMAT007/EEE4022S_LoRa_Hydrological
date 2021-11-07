/*
   FOSSA Ground Station Example

   Tested on Arduino Uno and SX1268, can be used with any LoRa radio
   from the SX127x or SX126x series. Make sure radio type (line 23)
   and pin mapping (lines 26 - 29) match your hardware!

   References:

   RadioLib error codes:
   https://jgromes.github.io/RadioLib/group__status__codes.html

   FOSSASAT-1B Communication Guide:

*/

// include all libraries
#include <RadioLib.h>
#include <FOSSA-Comms.h>

//#define USE_GFSK                    // uncomment to use GFSK
//#define USE_SX126X                    // uncomment to use SX126x

// pin definitions
#define CS                    5      // SPI chip select
#define DIO                   2       // DIO0 for SX127x, DIO1 for SX126x
#define NRST                  14      // NRST pin (optional)
#define BUSY                  4       // BUSY pin (SX126x-only)

// modem configuration
#define LORA_FREQUENCY        436.7   // MHz
#define FSK_FREQUENCY         436.9   // MHz
#define BANDWIDTH             125.0   // kHz
#define SPREADING_FACTOR      11      // -
#define CODING_RATE           8       // 4/8
#define SYNC_WORD             0x12    // used as LoRa "sync word", or twice repeated as FSK sync word (0x1212)
#define OUTPUT_POWER          10      // dBm
#define CURRENT_LIMIT         140     // mA
#define LORA_PREAMBLE_LEN     8       // symbols
#define BIT_RATE              9.6     // kbps
#define FREQ_DEV              5.0     // kHz SSB
#define RX_BANDWIDTH          39.0    // kHz SSB
#define FSK_PREAMBLE_LEN      16      // bits
#define DATA_SHAPING          RADIOLIB_SHAPING_0_5     // BT product
#define TCXO_VOLTAGE          1.6
#define WHITENING_INITIAL     0x1FF   // initial whitening LFSR value

// set up radio module
SX1278 radio = new Module(CS, DIO, NRST, RADIOLIB_NC);


// flags
volatile bool interruptEnabled = true;
volatile bool transmissionReceived = false;

// satellite callsign
char callsign[] = "FOSSASAT-2";

// transmission password
const char* password = "password";

// encryption key
const uint8_t encryptionKey[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00};

// radio ISR
void onInterrupt() {
  if (!interruptEnabled) {
    return;
  }

  transmissionReceived = true;
}

void printStatTemperature(const char* str, uint8_t* respOptData, uint8_t& pos) {
  Serial.print(str);
  Serial.print(FCP_System_Info_Get_Temperature(respOptData, pos), 2); pos += sizeof(int16_t); Serial.print('\t');
  Serial.print(FCP_System_Info_Get_Temperature(respOptData, pos), 2); pos += sizeof(int16_t); Serial.print('\t');
  Serial.print(FCP_System_Info_Get_Temperature(respOptData, pos), 2); pos += sizeof(int16_t); Serial.print('\n');
}

void printStatCurrent(const char* str, uint8_t* respOptData, uint8_t& pos) {
  Serial.print(str);
  Serial.print(FCP_System_Info_Get_Current(respOptData, pos), 2); pos += sizeof(int16_t); Serial.print('\t');
  Serial.print(FCP_System_Info_Get_Current(respOptData, pos), 2); pos += sizeof(int16_t); Serial.print('\t');
  Serial.print(FCP_System_Info_Get_Current(respOptData, pos), 2); pos += sizeof(int16_t); Serial.print('\n');
}

void printStatVoltage(const char* str, uint8_t* respOptData, uint8_t& pos) {
  Serial.print(str);
  Serial.print(FCP_System_Info_Get_Voltage(respOptData, pos), 2); pos += sizeof(uint8_t); Serial.print('\t');
  Serial.print(FCP_System_Info_Get_Voltage(respOptData, pos), 2); pos += sizeof(uint8_t); Serial.print('\t');
  Serial.print(FCP_System_Info_Get_Voltage(respOptData, pos), 2); pos += sizeof(uint8_t); Serial.print('\n');
}

void printStatFloat(const char* str, uint8_t* respOptData, uint8_t& pos) {
  float f = 0;
  Serial.print(str);
  memcpy(&f, respOptData + pos, sizeof(float)); pos += sizeof(float); Serial.print(f, 2); Serial.print('\t');
  memcpy(&f, respOptData + pos, sizeof(float)); pos += sizeof(float); Serial.print(f, 2); Serial.print('\t');
  memcpy(&f, respOptData + pos, sizeof(float)); pos += sizeof(float); Serial.print(f, 2); Serial.print('\n');
}

void sendFrame(uint8_t functionId, uint8_t optDataLen = 0, uint8_t* optData = NULL) {
  // build frame
  uint8_t len = FCP_Get_Frame_Length(callsign, optDataLen);
  uint8_t* frame = new uint8_t[len];
  FCP_Encode(frame, callsign, functionId, optDataLen, optData);

  // send data
  int state = radio.transmit(frame, len);
  delete[] frame;

  // check transmission success
  if (state == ERR_NONE) {
    Serial.println(F("sent successfully!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
  }
}

// function to print controls
void printControls() {
  Serial.println(F("------------- Controls -------------"));
  Serial.println(F("p - send ping frame"));
  Serial.println(F("i - request satellite info"));
  Serial.println(F("l - request last packet info"));
  Serial.println(F("r - send message to be retransmitted"));
  Serial.println(F("B - request store and forward message"));
  Serial.println(F("------------------------------------"));
}

void decode(uint8_t* respFrame, uint8_t respLen) {
  // print raw data
  Serial.print(F("Received "));
  Serial.print(respLen);
  Serial.println(F(" bytes:"));

  // get function ID
  uint8_t functionId = FCP_Get_FunctionID(callsign, respFrame, respLen);

  if(functionId != RESP_CAMERA_PICTURE) {
    // print packet info
    Serial.print(F("RSSI: "));
    Serial.print(radio.getRSSI());
    Serial.println(F(" dBm"));
    Serial.print(F("SNR: "));
    Serial.print(radio.getSNR());
    Serial.println(F(" dB"));
    Serial.print(F("Function ID: 0x"));
    Serial.println(functionId, HEX);
    PRINT_BUFF(respFrame, respLen);
  }

  // check optional data
  uint8_t* respOptData = nullptr;
  uint8_t respOptDataLen = 0;
  if (functionId < PRIVATE_OFFSET) {
    // public frame
    respOptDataLen = FCP_Get_OptData_Length(callsign, respFrame, respLen);
  } else {
    // private frame
    respOptDataLen = FCP_Get_OptData_Length(callsign, respFrame, respLen, encryptionKey, password);
  }
  Serial.print(F("Optional data ("));
  Serial.print(respOptDataLen);
  Serial.println(F(" bytes):"));
  if (respOptDataLen > 0) {
    // read optional data
    respOptData = new uint8_t[respOptDataLen];
    if (functionId < PRIVATE_OFFSET) {
      // public frame
      FCP_Get_OptData(callsign, respFrame, respLen, respOptData);
    } else {
      // private frame
      FCP_Get_OptData(callsign, respFrame, respLen, respOptData, encryptionKey, password);
    }

    if(functionId != RESP_CAMERA_PICTURE) {
      PRINT_BUFF(respOptData, respOptDataLen);
    }
  }

  // process received frame
  switch (functionId) {
    case RESP_PONG:
      Serial.println(F("Pong!"));
      break;

    case RESP_SYSTEM_INFO: {
      Serial.println(F("System info:"));

      Serial.print(F("batteryVoltage = "));
      Serial.print(FCP_Get_Battery_Voltage(respOptData));
      Serial.println(" V");

      Serial.print(F("batteryChargingCurrent = "));
      Serial.print(FCP_Get_Battery_Charging_Current(respOptData), 4);
      Serial.println(" mA");

      uint32_t onboardTime = 0;
      memcpy(&onboardTime, respOptData + 3, sizeof(uint32_t));
      Serial.print(F("onboardTime = "));
      Serial.println(onboardTime);

      uint8_t powerConfig = 0;
      memcpy(&powerConfig, respOptData + 7, sizeof(uint8_t));
      Serial.print(F("powerConfig = 0b"));
      Serial.println(powerConfig, BIN);

      uint16_t resetCounter = 0;
      memcpy(&resetCounter, respOptData + 8, sizeof(uint16_t));
      Serial.print(F("resetCounter = "));
      Serial.println(resetCounter);

      Serial.print(F("voltageXA = "));
      Serial.print(FCP_System_Info_Get_Voltage(respOptData, 10));
      Serial.println(" V");

      Serial.print(F("voltageXB = "));
      Serial.print(FCP_System_Info_Get_Voltage(respOptData, 11));
      Serial.println(" V");

      Serial.print(F("voltageZA = "));
      Serial.print(FCP_System_Info_Get_Voltage(respOptData, 12));
      Serial.println(" V");

      Serial.print(F("voltageZB = "));
      Serial.print(FCP_System_Info_Get_Voltage(respOptData, 13));
      Serial.println(" V");

      Serial.print(F("voltageY = "));
      Serial.print(FCP_System_Info_Get_Voltage(respOptData, 14));
      Serial.println(" V");

      Serial.print(F("batteryTemp = "));
      Serial.print(FCP_System_Info_Get_Temperature(respOptData, 15));
      Serial.println(" deg C");

      Serial.print(F("boardTemp = "));
      Serial.print(FCP_System_Info_Get_Temperature(respOptData, 17));
      Serial.println(" deg C");

      uint32_t errCounter = 0;
      memcpy(&errCounter, respOptData + 19, sizeof(uint32_t));
      Serial.print(F("errCounter = "));
      Serial.println(errCounter);
    } break;

    case RESP_PACKET_INFO: {
      Serial.println(F("Packet info:"));

      Serial.print(F("SNR = "));
      Serial.print(respOptData[0] / 4.0);
      Serial.println(F(" dB"));

      Serial.print(F("RSSI = "));
      Serial.print(respOptData[1] / -2.0);
      Serial.println(F(" dBm"));

      uint16_t counter = 0;
      Serial.print(F("valid LoRa frames = "));
      memcpy(&counter, respOptData + 2, sizeof(uint16_t));
      Serial.println(counter);

      Serial.print(F("invalid LoRa frames = "));
      memcpy(&counter, respOptData + 4, sizeof(uint16_t));
      Serial.println(counter);

      Serial.print(F("valid FSK frames = "));
      memcpy(&counter, respOptData + 6, sizeof(uint16_t));
      Serial.println(counter);

      Serial.print(F("invalid FSK frames = "));
      memcpy(&counter, respOptData + 8, sizeof(uint16_t));
      Serial.println(counter);
    } break;

    case RESP_REPEATED_MESSAGE:
      Serial.println(F("Got repeated message:"));
      for (uint8_t i = 0; i < respOptDataLen; i++) {
        Serial.write(respOptData[i]);
      }
      Serial.println();
      break;

    case RESP_DEPLOYMENT_STATE:
      Serial.println(F("Got deployment counter:"));
      Serial.println(respOptData[0]);
      break;

    case RESP_STATISTICS: {
        Serial.println(F("Got stats:\t\tunit\tmin\tavg\tmax"));
        uint8_t flags = respOptData[0];
        uint8_t pos = sizeof(flags);

        if (flags & 0x01) {
          printStatTemperature("Temp panel Y\t\tdeg C\t", respOptData, pos);
          printStatTemperature("Temp top\t\tdeg C\t", respOptData, pos);
          printStatTemperature("Temp bottom\t\tdeg C\t", respOptData, pos);
          printStatTemperature("Temp battery\t\tdeg C\t", respOptData, pos);
          printStatTemperature("Temp sec. battery\tdeg C\t", respOptData, pos);
        }

        if (flags & 0x02) {
          printStatCurrent("Current XA\t\tmA\t", respOptData, pos);
          printStatCurrent("Current XB\t\tmA\t", respOptData, pos);
          printStatCurrent("Current ZA\t\tmA\t", respOptData, pos);
          printStatCurrent("Current ZA\t\tmA\t", respOptData, pos);
          printStatCurrent("Current Y\t\tmA\t", respOptData, pos);
          printStatCurrent("Current MPPT\t\tmA\t", respOptData, pos);
        }

        if (flags & 0x04) {
          printStatVoltage("Voltage XA\t\tV\t", respOptData, pos);
          printStatVoltage("Voltage XB\t\tV\t", respOptData, pos);
          printStatVoltage("Voltage ZA\t\tV\t", respOptData, pos);
          printStatVoltage("Voltage ZB\t\tV\t", respOptData, pos);
          printStatVoltage("Voltage Y\t\tV\t", respOptData, pos);
          printStatVoltage("Voltage MPPT\t\tV\t", respOptData, pos);
        }

        if (flags & 0x08) {
          printStatFloat("Light panel Y\t\tlux\t", respOptData, pos);
          printStatFloat("Light top\t\tlux\t", respOptData, pos);
        }

        if (flags & 0x10) {
          printStatFloat("Angle velocity X\trad/s\t", respOptData, pos);
          printStatFloat("Angle velocity Y\trad/s\t", respOptData, pos);
          printStatFloat("Angle velocity Z\trad/s\t", respOptData, pos);
          printStatFloat("Acceleration X\t\tm/s^2\t", respOptData, pos);
          printStatFloat("Acceleration Y\t\tm/s^2\t", respOptData, pos);
          printStatFloat("Acceleration Z\t\tm/s^2\t", respOptData, pos);
          printStatFloat("Magn. induction X\ttesla\t", respOptData, pos);
          printStatFloat("Magn. induction Y\ttesla\t", respOptData, pos);
          printStatFloat("Magn. induction Z\ttesla\t", respOptData, pos);
        }

      } break;

    case RESP_FULL_SYSTEM_INFO: {
        Serial.println(F("System info:"));

        Serial.print(F("batteryVoltage = "));
        Serial.print(FCP_Get_Battery_Voltage(respOptData));
        Serial.println(" V");

        Serial.print(F("batteryChargingCurrent = "));
        Serial.print(FCP_Get_Battery_Charging_Current(respOptData), 4);
        Serial.println(" mA");

        uint32_t onboardTime = 0;
        memcpy(&onboardTime, respOptData + 3, sizeof(uint32_t));
        Serial.print(F("onboardTime = "));
        Serial.println(onboardTime);

        uint8_t powerConfig = 0;
        memcpy(&powerConfig, respOptData + 7, sizeof(uint8_t));
        Serial.print(F("powerConfig = 0b"));
        Serial.println(powerConfig, BIN);

        uint16_t resetCounter = 0;
        memcpy(&resetCounter, respOptData + 8, sizeof(uint16_t));
        Serial.print(F("resetCounter = "));
        Serial.println(resetCounter);

        Serial.print(F("voltageXA = "));
        Serial.print(FCP_System_Info_Get_Voltage(respOptData, 10));
        Serial.println(" V");

        Serial.print(F("currentXA = "));
        Serial.print(FCP_System_Info_Get_Current(respOptData, 11));
        Serial.println(" mA");

        Serial.print(F("voltageXB = "));
        Serial.print(FCP_System_Info_Get_Voltage(respOptData, 13));
        Serial.println(" V");

        Serial.print(F("currentXB = "));
        Serial.print(FCP_System_Info_Get_Current(respOptData, 14));
        Serial.println(" mA");

        Serial.print(F("voltageZA = "));
        Serial.print(FCP_System_Info_Get_Voltage(respOptData, 16));
        Serial.println(" V");

        Serial.print(F("currentZA = "));
        Serial.print(FCP_System_Info_Get_Current(respOptData, 17));
        Serial.println(" mA");

        Serial.print(F("voltageZB = "));
        Serial.print(FCP_System_Info_Get_Voltage(respOptData, 19));
        Serial.println(" V");

        Serial.print(F("currentZB = "));
        Serial.print(FCP_System_Info_Get_Current(respOptData, 20));
        Serial.println(" mA");

        Serial.print(F("voltageY = "));
        Serial.print(FCP_System_Info_Get_Voltage(respOptData, 22));
        Serial.println(" V");

        Serial.print(F("currentY = "));
        Serial.print(FCP_System_Info_Get_Current(respOptData, 23));
        Serial.println(" mA");

        Serial.print(F("tempPanelY = "));
        Serial.print(FCP_System_Info_Get_Temperature(respOptData, 25));
        Serial.println(" deg C");

        Serial.print(F("boardTemp = "));
        Serial.print(FCP_System_Info_Get_Temperature(respOptData, 27));
        Serial.println(" deg C");

        Serial.print(F("tempBottom = "));
        Serial.print(FCP_System_Info_Get_Temperature(respOptData, 29));
        Serial.println(" deg C");

        Serial.print(F("batteryTemp = "));
        Serial.print(FCP_System_Info_Get_Temperature(respOptData, 31));
        Serial.println(" deg C");

        Serial.print(F("secBatteryTemp = "));
        Serial.print(FCP_System_Info_Get_Temperature(respOptData, 33));
        Serial.println(" deg C");

        Serial.print(F("mcuTemp = "));
        Serial.print(FCP_System_Info_Get_Temperature(respOptData, 35));
        Serial.println(" deg C");

        float lightVal = 0;
        memcpy(&lightVal, respOptData + 37, sizeof(float));
        Serial.print(F("lightPanelY = "));
        Serial.println(lightVal, 2);

        memcpy(&lightVal, respOptData + 41, sizeof(float));
        Serial.print(F("lightTop = "));
        Serial.println(lightVal, 2);

        uint8_t fault = 0;
        memcpy(&fault, respOptData + 45, sizeof(uint8_t));
        Serial.print(F("faultX = 0x"));
        Serial.println(fault, HEX);

        memcpy(&fault, respOptData + 46, sizeof(uint8_t));
        Serial.print(F("faultY = 0x"));
        Serial.println(fault, HEX);

        memcpy(&fault, respOptData + 47, sizeof(uint8_t));
        Serial.print(F("faultZ = 0x"));
        Serial.println(fault, HEX);

        uint32_t errCounter = 0;
        memcpy(&errCounter, respOptData + 48, sizeof(uint32_t));
        Serial.print(F("errCounter = "));
        Serial.println(errCounter);

        uint8_t rxLen = 0;
        memcpy(&rxLen, respOptData + 52, sizeof(uint8_t));
        Serial.print(F("fskRxLen = "));
        Serial.println(rxLen);

        memcpy(&rxLen, respOptData + 53, sizeof(uint8_t));
        Serial.print(F("loraRxLen = "));
        Serial.println(rxLen);

        uint8_t sensors = 0;
        memcpy(&sensors, respOptData + 54, sizeof(uint8_t));
        Serial.print(F("sensors = 0x"));
        Serial.println(sensors, HEX);

      } break;


    case RESP_GPS_LOG: {
      for(uint8_t i = 0; i < respOptDataLen; i++) {
        Serial.write(respOptData[i]);
      }
      Serial.println();

    } break;

    case RESP_GPS_LOG_STATE: {
      Serial.println(F("GPS log state:"));
      uint32_t ul = 0;

      memcpy(&ul, respOptData, sizeof(uint32_t));
      Serial.print(F("length = "));
      Serial.println(ul);

      memcpy(&ul, respOptData + sizeof(uint32_t), sizeof(uint32_t));
      Serial.print(F("last entry = "));
      Serial.println(ul, HEX);

      memcpy(&ul, respOptData + 2*sizeof(uint32_t), sizeof(uint32_t));
      Serial.print(F("last fix = "));
      Serial.println(ul, HEX);
    } break;

    case RESP_ACKNOWLEDGE: {
      Serial.print(F("Frame ACK, functionId = 0x"));
      Serial.print(respOptData[0], HEX);
      Serial.print(F(", result = 0x"));
      Serial.println(respOptData[1], HEX);
    } break;

    default:
      Serial.println(F("Unknown function ID!"));
      break;
  }

  if(functionId != RESP_CAMERA_PICTURE) {
    printControls();
  }

  if (respOptDataLen > 0) {
    delete[] respOptData;
  }
}

void getResponse(uint32_t timeout) {
  uint32_t start = millis();
  while (millis() - start <= timeout) {
    if (transmissionReceived) {
      // disable reception interrupt
      interruptEnabled = false;
      transmissionReceived = false;

      // read received data
      size_t respLen = radio.getPacketLength();
      uint8_t* respFrame = new uint8_t[respLen];
      int state = radio.readData(respFrame, respLen);

      if (state == ERR_NONE) {
        decode(respFrame, respLen);
      } else {
        Serial.print(F("Error, code "));
        Serial.println(state);
      }

      delete[] respFrame;

      // enable reception interrupt
      radio.startReceive();
      interruptEnabled = true;
    }
  }
}

void sendPing() {
  Serial.print(F("Sending ping frame ... "));

  // send the frame
  sendFrame(CMD_PING);
}

void requestInfo() {
  Serial.print(F("Requesting system info ... "));

  // send the frame
  sendFrame(CMD_TRANSMIT_SYSTEM_INFO);
}

void requestPacketInfo() {
  Serial.print(F("Requesting last packet info ... "));

  // send the frame
  sendFrame(CMD_GET_PACKET_INFO);
}

void requestRetransmit() {
  Serial.println(F("Enter message to be sent:"));
  Serial.println(F("(max 32 characters, end with LF or CR+LF)"));

  // get data to be retransmited
  char optData[32];
  uint8_t bufferPos = 0;
  while (bufferPos < 32) {
    while (!Serial.available());
    char c = Serial.read();
    Serial.print(c);
    if ((c != '\r') && (c != '\n')) {
      optData[bufferPos] = c;
      bufferPos++;
    } else {
      break;
    }
  }

  // wait for a bit to receive any trailing characters
  delay(100);

  // dump the serial buffer
  while (Serial.available()) {
    Serial.read();
  }

  Serial.println();
  Serial.print(F("Requesting retransmission ... "));

  // send the frame
  optData[bufferPos] = '\0';
  uint8_t optDataLen = strlen(optData);
  sendFrame(CMD_RETRANSMIT, optDataLen, (uint8_t*)optData);
}

void requestRetransmitCustom() {
  Serial.println(F("Enter message to be sent:"));
  Serial.println(F("(max 32 characters, end with LF or CR+LF)"));

  // get data to be retransmited
  uint8_t optData[32 + 7];
  optData[0] = 0x07;
  optData[1] = 0x06;
  optData[2] = 0x08;
  optData[3] = 0x08;
  optData[4] = 0x00;
  optData[5] = 0x01;
  optData[6] = 20;
  uint8_t bufferPos = 7;
  while (bufferPos < 32 + 7) {
    while (!Serial.available());
    char c = Serial.read();
    Serial.print(c);
    if ((c != '\r') && (c != '\n')) {
      optData[bufferPos] = (uint8_t)c;
      bufferPos++;
    } else {
      break;
    }
  }

  // wait for a bit to receive any trailing characters
  delay(100);

  // dump the serial buffer
  while (Serial.available()) {
    Serial.read();
  }

  Serial.println();
  Serial.print(F("Requesting retransmission ... "));

  // send the frame
  uint8_t optDataLen = bufferPos - 1;
  sendFrame(CMD_RETRANSMIT_CUSTOM, optDataLen, optData);
}

int16_t setLoRa() {
  int state = radio.begin(LORA_FREQUENCY,
                          BANDWIDTH,
                          SPREADING_FACTOR,
                          CODING_RATE,
                          SYNC_WORD,
                          OUTPUT_POWER,
                          LORA_PREAMBLE_LEN,
                          TCXO_VOLTAGE);
  radio.setCRC(true);
  radio.setCurrentLimit(CURRENT_LIMIT);
  #ifdef USE_SX126X
  radio.setWhitening(true, WHITENING_INITIAL);
  #endif
  return(state);
}

int16_t setGFSK() {
  int state = radio.beginFSK(FSK_FREQUENCY,
                             BIT_RATE,
                             FREQ_DEV,
                             RX_BANDWIDTH,
                             OUTPUT_POWER,
                             FSK_PREAMBLE_LEN,
                             TCXO_VOLTAGE);
  uint8_t syncWordFSK[2] = {SYNC_WORD, SYNC_WORD};
  radio.setSyncWord(syncWordFSK, 2);
  radio.setDataShaping(DATA_SHAPING);
  radio.setCurrentLimit(CURRENT_LIMIT);
  #ifdef USE_SX126X
    radio.setCRC(2);
    radio.setWhitening(true, WHITENING_INITIAL);
  #else
    radio.setCRC(true);
  #endif
  return (state);
}

void sendUnknownFrame() {
  radio.implicitHeader(strlen(callsign) + 1);
  sendPing();
  radio.explicitHeader();
}

void getStats(uint8_t mask) {
  Serial.print(F("Sending stats request ... "));
  sendFrame(CMD_GET_STATISTICS, 1, &mask);
}


void getFullSystemInfo() {
  Serial.print(F("Sending full system info request ... "));
  sendFrame(CMD_GET_FULL_SYSTEM_INFO);
}


void addStoreAndForward(uint32_t id, const char* msg) {
  Serial.print(F("Adding store and forward message ... "));
  uint8_t optData[32];
  memcpy(optData, &id, sizeof(uint32_t));
  memcpy(optData + sizeof(uint32_t), msg, strlen(msg));
  sendFrame(CMD_STORE_AND_FORWARD_ADD, sizeof(uint32_t) + strlen(msg), optData);
}

void requestStoreAndForward(uint32_t id) {
  Serial.print(F("Requesting store and forward message ... "));
  uint8_t optData[4];
  memcpy(optData, &id, sizeof(uint32_t));
  sendFrame(CMD_STORE_AND_FORWARD_REQUEST, 4, optData);
}


void setup() {
  Serial.begin(115200);
  Serial.println(F("FOSSASAT-2 Ground Station Demo Code"));

  // initialize the radio
  #ifdef USE_GFSK
    int state = setGFSK();
  #else
    int state = setLoRa();
  #endif

  if (state == ERR_NONE) {
    Serial.println(F("Radio initialization successful!"));
  } else {
    Serial.print(F("Failed to initialize radio, code: "));
    Serial.println(state);
    while (true);
  }

  #ifdef USE_SX126X
    radio.setDio1Action(onInterrupt);
  #else
    radio.setDio0Action(onInterrupt);
  #endif

  // begin listening for packets
  radio.startReceive();

  // provide seed for PRNG
  randomSeed(analogRead(A6));

  printControls();
}

void loop() {
  // check serial data
  if (Serial.available()) {
    // disable reception interrupt
    interruptEnabled = false;
    #ifdef USE_SX126X
      radio.clearDio1Action();
    #else
      radio.clearDio0Action();
    #endif

    // get the first character
    char serialCmd = Serial.read();

    // wait for a bit to receive any trailing characters
    delay(50);

    // dump the serial buffer
    while (Serial.available()) {
      Serial.read();
    }

    // process serial command
    switch (serialCmd) {
      case 'p':
        sendPing();
        break;
      case 'i':
        requestInfo();
        break;
      case 'l':
        requestPacketInfo();
        break;
      case 'r':
        requestRetransmit();
        break;
      case 'B':
        requestStoreAndForward(0x1337BEEF);
        break;
      default:
        Serial.print(F("Unknown command: "));
        Serial.println(serialCmd);
        break;
    }

    // for some reason, when using SX126x GFSK and listening after transmission,
    // the next packet received will have bad CRC,
    // and the data will be the transmitted packet
    // the only workaround seems to be resetting the module
    #if defined(USE_GFSK) && defined(USE_SX126X)
      radio.sleep(false);
      delay(10);
      setGFSK();
    #endif

    // set radio mode to reception
    #ifdef USE_SX126X
      radio.setDio1Action(onInterrupt);
    #else
      radio.setDio0Action(onInterrupt);
    #endif
    radio.startReceive();
    interruptEnabled = true;
  }

  // check if new data were received
  if (transmissionReceived) {
    // disable reception interrupt
    interruptEnabled = false;
    transmissionReceived = false;

    // read received data
    size_t respLen = radio.getPacketLength();
    uint8_t* respFrame = new uint8_t[respLen];
    int state = radio.readData(respFrame, respLen);

    // check reception success
    if (state == ERR_NONE) {
      decode(respFrame, respLen);

    } else if (state == ERR_CRC_MISMATCH) {
      Serial.println(F("Got CRC error!"));
      Serial.print(F("Received "));
      Serial.print(respLen);
      Serial.println(F(" bytes:"));
      //PRINT_BUFF(respFrame, respLen);

    } else {
      Serial.println(F("Reception failed, code "));
      Serial.println(state);

    }

    // enable reception interrupt
    delete[] respFrame;
    radio.startReceive();
    interruptEnabled = true;
  }
}
