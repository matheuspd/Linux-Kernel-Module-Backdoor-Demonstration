#include <WiFi.h>

const char* ssid = "WIFI_NAME";
const char* password = "WIFI_PSW";

WiFiServer server1(PORT1); // "Attacker"
WiFiServer server2(PORT2); // Backdoor

WiFiClient client1;
WiFiClient client2;

const size_t packetSize = 1024;
char packet[packetSize];

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(10000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server1.begin();
  server2.begin();
}

void loop() {
  // Check the server for pending client 1 connections
  if (server1.hasClient()) {
    if (!client1 || !client1.connected()) {
      client1 = server1.available();
      Serial.println("Cliente 1 conectado");
    }
  }

  // Check the server for pending client 2 connections
  if (server2.hasClient()) {
    if (!client2 || !client2.connected()) {
      client2 = server2.available();
      Serial.println("Cliente 2 conectado");
    }
  }

  // Check if data is available for reading from client 1
  if (client1 && client1.connected() && client1.available()) {
    size_t bytesRead = client1.readBytes(packet, packetSize);

    // Redirect packet to Client 2
    if (client2 && client2.connected()) {
      client2.write(packet, bytesRead);
    }
  }

  // Check if data is available for reading from client 2
  if (client2 && client2.connected()) {
    while (client2.available()) {
      size_t bytesRead = client2.readBytes(packet, packetSize);

      // Redirect packet to Client 1
      if (client1 && client1.connected()) {
        client1.write(packet, bytesRead);
      }
    }
  }
}