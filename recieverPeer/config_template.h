// WiFi
#define WIFI_SSID     "Thorburn Home" // Add wifi name
#define WIFI_PASSWORD "CATRadioGMT" // Add wifi passowrd

#define ID "HydroEndNode" // Add unique name for this sensor
#define INTERVAL 60

//#define DHTPIN 32    // Which pin is DHT 11 connected to
//#define DHTTYPE DHT11 // Type DHT 11

//#define ULTRASONIC_PIN_TRIG 4 // Which pin is HC-SR04's trig connected to
//#define ULTRASONIC_PIN_ECHO 5 // Which pin is HC-SR04's echo connected to

#define INFLUX_HOST "us-east-1-1.aws.cloud2.influxdata.com" // Influx host (e.g. eu-central-1-1.aws.cloud2.influxdata.com)
#define INFLUX_ORG_ID "c517f3b3c0d543ee" // Org id
#define INFLUX_TOKEN "CdcLL2itCKZxPD82hlsOUcC2_Hf2bavwT8-DgTkGutAiuLRZEsFmYUWtIw5CfEWN3o8Kg_C_b1B8cT0z5oQ_4Q==" // Influx token
#define INFLUX_BUCKET "ESp32HydroData" // Influx bucket that we set up for this host

#define LOKI_USER "110191" // Hosted Loki user
#define LOKI_API_KEY "eyJrIjoiZDEyYWY0OTdmOTBkMjQ3MDMxZGRlYzZjZGJkYjg3NGYyZTgxYzEwNSIsIm4iOiJFU1AzMkRhdGEiLCJpZCI6NTUwNTQzfQ==" // Hosted Loki API key

// How to get the Root CA cert:
// https://techtutorialsx.com/2017/11/18/esp32-arduino-https-get-request/

#define ROOT_CA "-----BEGIN CERTIFICATE-----"
"-----BEGIN CERTIFICATE----- \n"
"MIIFODCCBN6gAwIBAgIQDyp6lxK+XXMFAk57ngzgqjAKBggqhkjOPQQDAjBKMQsw\n"
"CQYDVQQGEwJVUzEZMBcGA1UEChMQQ2xvdWRmbGFyZSwgSW5jLjEgMB4GA1UEAxMX\n"
"Q2xvdWRmbGFyZSBJbmMgRUNDIENBLTMwHhcNMjEwNjI4MDAwMDAwWhcNMjIwNjI3\n"
"MjM1OTU5WjB1MQswCQYDVQQGEwJVUzETMBEGA1UECBMKQ2FsaWZvcm5pYTEWMBQG\n"
"A1UEBxMNU2FuIEZyYW5jaXNjbzEZMBcGA1UEChMQQ2xvdWRmbGFyZSwgSW5jLjEe\n"
"MBwGA1UEAxMVc25pLmNsb3VkZmxhcmVzc2wuY29tMFkwEwYHKoZIzj0CAQYIKoZI\n"
"zj0DAQcDQgAEue3t/MQ9XSvGZMFTo7ZSVnu/4oar/5SX/ZzKETsjLYt0e5R/mQAT\n"
"+tEBcwCzkDt08iDXR1PU+DNYpQf1EjmKfqOCA3kwggN1MB8GA1UdIwQYMBaAFKXO\n"
"N+rrsHUOlGeItEX62SQQh5YfMB0GA1UdDgQWBBSQanoOPWamtWp3WfVnJhDD8E3p\n"
"tzA+BgNVHREENzA1ggx0eXBpY29kZS5jb22CFXNuaS5jbG91ZGZsYXJlc3NsLmNv\n"
"bYIOKi50eXBpY29kZS5jb20wDgYDVR0PAQH/BAQDAgeAMB0GA1UdJQQWMBQGCCsG\n"
"AQUFBwMBBggrBgEFBQcDAjB7BgNVHR8EdDByMDegNaAzhjFodHRwOi8vY3JsMy5k\n"
"aWdpY2VydC5jb20vQ2xvdWRmbGFyZUluY0VDQ0NBLTMuY3JsMDegNaAzhjFodHRw\n"
"Oi8vY3JsNC5kaWdpY2VydC5jb20vQ2xvdWRmbGFyZUluY0VDQ0NBLTMuY3JsMD4G\n"
"A1UdIAQ3MDUwMwYGZ4EMAQICMCkwJwYIKwYBBQUHAgEWG2h0dHA6Ly93d3cuZGln\n"
"aWNlcnQuY29tL0NQUzB2BggrBgEFBQcBAQRqMGgwJAYIKwYBBQUHMAGGGGh0dHA6\n"
"Ly9vY3NwLmRpZ2ljZXJ0LmNvbTBABggrBgEFBQcwAoY0aHR0cDovL2NhY2VydHMu\n"
"ZGlnaWNlcnQuY29tL0Nsb3VkZmxhcmVJbmNFQ0NDQS0zLmNydDAMBgNVHRMBAf8E\n"
"AjAAMIIBfwYKKwYBBAHWeQIEAgSCAW8EggFrAWkAdgApeb7wnjk5IfBWc59jpXfl\n"
"vld9nGAK+PlNXSZcJV3HhAAAAXpTKCpbAAAEAwBHMEUCIHVlgulBqmCzyuw1nmcB\n"
"LFSYHTz7Wyo2rfFObgQ/LbUHAiEA/kzrzGimKI/Vw+TQOtSRmDMPyk3N8OIYKAB6\n"
"GMcGrNoAdwAiRUUHWVUkVpY/oS/x922G4CMmY63AS39dxoNcbuIPAgAAAXpTKCqf\n"
"AAAEAwBIMEYCIQDdHeXhR9xTRXKZI9LFPdiXk2HRfOHqGcEqkAfjRnD22QIhALw/\n"
"D9hK8wKkaKzVe0UKWsO57bkBVeR1BLpYS9Pi/+WDAHYAQcjKsd8iRkoQxqE6CUKH\n"
"Xk4xixsD6+tLx2jwkGKWBvYAAAF6UygqYAAABAMARzBFAiEAhypm+Quwbn3ASsjy\n"
"ZZ0nmuzAhr+qMQE2RXpek4/vzTYCICpsJGPX0OXXq6Pqd9pMSHCfbkp4K+kZCX/N\n"
"ziOrAk3EMAoGCCqGSM49BAMCA0gAMEUCIAzzcI29Cey6wEqEhVW/Be9WbsTXVqVr\n"
"sDyTSfnPV0E6AiEAp/OJ7BCwZ1rIwZPGZpW7LT5wBs3ttE7FkIGoL83gQN4=\n"
"-----END CERTIFICATE-----\n"


