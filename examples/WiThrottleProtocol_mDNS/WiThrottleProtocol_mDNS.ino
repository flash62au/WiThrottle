// WiThrottleProtocol library: mDNS example
//
// Shows how to create an instance of WiThrottleProtocol
// and how to browse for WiThrottle protocol servers
// Tested with ESP32-DEVKITC development board
//
// Peter Akers, 2025

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiThrottleProtocol.h>

//  ESPmDNS problem - the attribute to get the IP addess was changed in v5
#if ESP_IDF_VERSION_MAJOR < 5
  #define ESPMDNS_IP_ATTRIBUTE_NAME MDNS.IP(i)
#else
  #define ESPMDNS_IP_ATTRIBUTE_NAME MDNS.address(i)
#endif

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
IPAddress serverAddress; // this value will be discovered
int serverPort;          // this value will be discovered

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

  // setup the bonjour listener
  if (!MDNS.begin("WiThrottleProtocol_mDNS")) {
    Serial.println("Error setting up MDNS responder!");
    delay(2000);
  } else {
    Serial.println("MDNS responder started");
  }

  const char * service = "withrottle";
  const char * proto= "tcp";
  Serial.print("Browsing for service _");   Serial.print(service);   
  Serial.print("._"); Serial.print(proto);  
  Serial.print(".local. on ");   Serial.print(ssid); Serial.println(" ... ");

  int timer = millis();
  int noOfWitServices = 0;
  while ( (noOfWitServices == 0) && ((millis()-timer) <= 10000) ) { // try for 10 seconds 
    noOfWitServices = MDNS.queryService(service, proto);
  }

  if (noOfWitServices == 0) {
    Serial.print("No WiThrottle servers found");
  } else {
    for (int i = 0; i < noOfWitServices; i++) {
      Serial.print(i); Serial.print(": "); Serial.print(MDNS.hostname(i));
      Serial.print(" - "); Serial.print(ESPMDNS_IP_ATTRIBUTE_NAME);
      Serial.print(": "); Serial.print(MDNS.hostname(i));
      Serial.print(": "); Serial.println(MDNS.port(i));
    }
  }


  // Connect to the first server found
  int i = 0;
  serverAddress = ESPMDNS_IP_ATTRIBUTE_NAME;
  serverPort = MDNS.port(i);

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
  wiThrottleProtocol.addLocomotive("S3");

}
  
void loop() {

  // parse incoming messages
  wiThrottleProtocol.check();
}
