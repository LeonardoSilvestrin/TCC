#include <Arduino.h>
#include <ESP8266WiFi.h>

const char* ssid = "Xiaomi 12 Lite";
const char* password = "hend3011";

WiFiServer server(80);

void setup() {
  Serial.begin(9600); // Start serial communication at 115200 baud rate
  delay(10);
  
  // Connect to Wi-Fi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Start the server
  server.begin();
  Serial.println("Server started");
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
}

unsigned long counter = 0;
void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New client connected");
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("");
    client.println("<html><body><h1>Hello, World!</h1></body></html>");
    delay(1);
    Serial.println("Client disconnected");
  }
}