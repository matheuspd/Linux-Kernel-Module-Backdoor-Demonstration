#include <WiFi.h>

const char* ssid = "WIFI_NAME";
const char* password = "PSWD";

WiFiServer server1(PORT1); // Port 1
WiFiServer server2(PORT2); // Port 2

WiFiClient client1;
WiFiClient client2;

String buffer1 = "";
String buffer2 = "";

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
  if (!client1.connected()) {
    client1 = server1.available();
  }

  if (!client2.connected()) {
    client2 = server2.available();
  }

  if (client1.connected()) {
    while (client1.available()) {
      char c = client1.read();
      if (c == '\n') {
        Serial.println(buffer1);
        if (client2.connected()) {
          client2.print(buffer1 + c);
        }
        buffer1 = "";
      } else {
        buffer1 += c;
      }
    }
  }

  if (client2.connected()) {
    while (client2.available()) {
      char c = client2.read();
      if (c == '\n') {
        Serial.println(buffer2);
        if (client1.connected()) {
          client1.print(buffer2 + c);
        }
        buffer2 = "";
      } else {
        buffer2 += c;
      }
    }
  }
}