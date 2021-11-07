# EEE4022S_LoRa_Hydrological
The Files in the repo were used in the development of a hydrological monitoring network.

The code developed was written in c++ and impleneted using the Arduino IDE.
This required support for ESP32 and STM32 systems to be imported to the Arduino IDE and guide to do so can be found below:
https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/
https://circuitdigest.com/microcontroller-projects/getting-started-with-stm32-blue-pill-development-board-stm32f103c8-using-arduino-ide

The nessesary Libaries for the operator at the ground station are:
Adafruit_Unified_Sensor
DHT_sensor_library
FOSSA-Comms-master
RadioLib-master
tiny-AES-c-master
NTPClient
arduino-LoRa-STM32-master

These libaries can be found on the Ardunio IDE libary manager, with the expection of FOSSA-Comms-master, tiny-AES-c-master and arduino-LoRa-STM32-master .
These libaries can be found in fossa files folder.

Within the Active folder is the working software developed for this project.
Matt_PingPong is a modification of the FossaSat PingPong software that was used to attempt a connection between the ground station and FossaSat-2
SenderPeer is the code used by the stm32f4 blackpill to collect hydrological data, and to transmit this data over Lora. This was used in the peer to peer testing of the system. a non working version was written to transmit to FossaSat-2 but as it was non-funciton it was move to the invactive folder. 
RecieverPeerInfluxDB is the code used on the esp32 nodemcu to recieve lora packets from the end sensing end node during the peer to peer testing. the recived data was then sent to the InfluxDB via a wifi connection.

For the configuration of the sensing end node and ground station please view the deployment guide.


Software used:
Arduino IDE
STM32 Cube Programmer
InfluxDB
