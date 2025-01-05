// WiThrottleProtocol library: Delegate example
//
// Shows how to create a delegate class to handle callbacks
// Tested with ESP32-DEVKITC development board
//
// Luca Dentella, 2020; Peter Akers, 2025

#include <WiFi.h>
#include <WiThrottleProtocol.h>

// Delegate class
class MyDelegate : public WiThrottleProtocolDelegate {
  
  public:
    void receivedVersion(String version) {     
      Serial.print("Received version: "); Serial.println(version);  
    }
};

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
MyDelegate myDelegate;
  
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

  // Pass the delegate instance to wiThrottleProtocol
  wiThrottleProtocol.setDelegate(&myDelegate);

  // Pass the communication to wiThrottleProtocol
  wiThrottleProtocol.connect(&client);
  Serial.println("WiThrottle connected");
  wiThrottleProtocol.setDeviceName("myFirstThrottle");  
}
  
void loop() {

  // parse incoming messages
  wiThrottleProtocol.check();
}
