#include <SPI.h>
#include <LoRa.h>

#include <WiFi.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#define DEVICE "ESP32"

// WiFi AP SSID
#define WIFI_SSID "XXXXX" 
// WiFi password
#define WIFI_PASSWORD "XXXX"
// InfluxDB v2 server url, e.g. https://eu-central-1-1.aws.cloud2.influxdata.com (Use: InfluxDB UI -> Load Data -> Client Libraries)
#define INFLUXDB_URL "https://us-east-1-1.aws.cloud2.influxdata.com"
// InfluxDB v2 server or cloud API token (Use: InfluxDB UI -> Data -> API Tokens -> <select token>)
#define INFLUXDB_TOKEN "XXXXXXXX"
// InfluxDB v2 organization id (Use: InfluxDB UI -> User -> About -> Common Ids )
#define INFLUXDB_ORG "twittythorburn@gmail.com"
// InfluxDB v2 bucket name (Use: InfluxDB UI ->  Data -> Buckets)
#define INFLUXDB_BUCKET "ESP32HydroData"

//define the pins used by the transceiver module
#define SS 5
#define RST 14
#define DI0 2
#define TX_P 17
#define BAND 433E6
#define ENCRYPT 0x35
#define SpreadingFactor 12
#define CodingRate 8


// Set timezone string according to https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
// Examples:
//  Pacific Time: "PST8PDT"
//  Eastern: "EST5EDT"
//  Japanesse: "JST-9"
//  Central Europe: "CET-1CEST,M3.5.0,M10.5.0/3"
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"

// InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// Data point
Point sensor("ESP32 Ground Station data");

//frame counter


void setup() {
  Serial.begin(115200);

  // Setup wifi
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  // Add tags for inlfuxDB
  sensor.addTag("device", DEVICE);
  sensor.addTag("SSID", WiFi.SSID());

  // Accurate time is necessary for certificate validation and writing in batches
  // For the fastest time sync find NTP servers in your area: https://www.pool.ntp.org/zone/
  // Syncing progress and the time will be printed to Serial.
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
  while (!Serial);
  
  //LoRa setip
  Serial.println("LoRa Receiver");
  LoRa.setTxPower(TX_P);
  LoRa.setSyncWord(ENCRYPT);
  LoRa.setCodingRate4(CodingRate);
  LoRa.setSpreadingFactor(SpreadingFactor);
  
  LoRa.setPins(SS, RST, DI0);
  if (!LoRa.begin(BAND)) 
  {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  
}

void loop() {
  String Message = "";
  float temp = 0.0;
  float hum = 0.0;
  float moisture = 0.0;
  int counter =0;
  
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet ");
 
    // read packet
    while (LoRa.available()) {
      Message += (char)LoRa.read();
      //Serial.println((char)LoRa.read());
    }

  //splitting message into the three values
  int delimiter_0, delimiter_1 ,delimiter_2 ,delimiter_3, delimiter_4;
  delimiter_0 = Message.indexOf("%");
  delimiter_1 = Message.indexOf("%", delimiter_0 + 1);
  delimiter_2 = Message.indexOf("%", delimiter_1 + 1);
  delimiter_3 = Message.indexOf("%", delimiter_2 + 1);
  delimiter_4 = Message.indexOf("%", delimiter_3 + 1);
  
  //Define variables to be executed on the code later by collecting information from the readString as substrings.
  String first = Message.substring(delimiter_0 +1, delimiter_1);
  String second = Message.substring(delimiter_1 + 1, delimiter_2);
  String third = Message.substring(delimiter_2 + 1, delimiter_3);
  String fourth = Message.substring(delimiter_3 + 1, delimiter_4);

  counter = first.toFloat();
  temp = second.toFloat();
  hum = third.toFloat();
  moisture = fourth.toFloat();
  // Clear fields for reusing the point. Tags will remain untouched
  sensor.clearFields();
  
  // Store measured value into point
  // Report RSSI of currently connected network
  sensor.addField("WiFi rssi", WiFi.RSSI());
  sensor.addField("Frame", counter);
  sensor.addField("Temperature", temp);
  sensor.addField("Humidity", hum);
  sensor.addField("Moisture", moisture);
  sensor.addField("LoRa rssi" ,LoRa.packetRssi());
  sensor.addField("SNR" , LoRa.packetSnr());

  // Print what are we exactly writing
  Serial.print("Writing: ");
  Serial.println(sensor.toLineProtocol());

  // Check WiFi connection and reconnect if needed
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("Wifi connection lost");
  }

  // Write to database
  if (!client.writePoint(sensor)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }
  }
}
