// WiThrottleProtocol library: FastTime example
//
// Shows how to use the Time library (https://github.com/PaulStoffregen/Time)
// to parse FastTime timestamp 
// DO NOT use Serial Monitor but a "real" terminal (PuTTY, RealTerm) 
// Tested with ESP32-DEVKITC development board
//
// Luca Dentella, 2020; Peter Akers, 2025

#include <WiFi.h>
#include <WiThrottleProtocol.h>
#include <TimeLib.h>

// WiFi and server configuration
// Note: the ESP32 can only use the 2.4gHz frequences (not 5gHz) 
// and only channels below 10
const char* ssid = "MySSID";
const char* password =  "MyPWD";
IPAddress serverAddress(192,168,1,1);
int serverPort = 12090;

// Global objects
WiFiClient client;
WiThrottleProtocol wiThrottleProtocol;
unsigned long lastPrint = 0;
  
void setup() {
  
  delay(5000); // this pause added only to help with debugging 
               // to give youy a chnace to open the serila monitor.
               // You would not normally have it.

  Serial.begin(115200);
  Serial.println("WiThrottleProtocol Delegate Demo");
  Serial.println();

  // Connect to WiFi network
  Serial.println("Connecting to WiFi..."); 
  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED) {
    Serial.println("Trying to connect...");
    delay(1000);  
  }
  
  Serial.print("Connected with IP: "); Serial.println(WiFi.localIP());

  // Connect to the server
  Serial.print("Connecting to the server: "); 
  Serial.print(serverAddress); Serial.print(":"); Serial.println(serverPort);
  if (!client.connect(serverAddress, serverPort)) {
    Serial.println("connection failed");
    while(1) delay(1000);
  }
  Serial.println("Connected to the server");

  // Uncomment for logging on Serial
  //wiThrottleProtocol.setLogStream(&Serial);

  // Pass the communication to wiThrottleProtocol
  wiThrottleProtocol.connect(&client);
  Serial.println("WiThrottle connected");
  wiThrottleProtocol.setDeviceName("myFirstThrottle");  
}
  
void loop() {

  // parse incoming messages
  wiThrottleProtocol.check();

  // every second, get the current fasttime, parse and print
  if(millis() - lastPrint > 1000) {
    time_t fastTime = (time_t)wiThrottleProtocol.getCurrentFastTime();
    Serial.print(hour(fastTime));
    Serial.print(":");
    Serial.print(minute(fastTime));
    Serial.print("\r");
    lastPrint = millis();    
  }  
}
