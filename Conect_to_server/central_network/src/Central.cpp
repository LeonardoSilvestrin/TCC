#include <Arduino.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// WiFi
const char *ssid = "Xaiomi 12 Lite"; // Enter your WiFi name
const char *password = "Hend3011";  // Enter WiFi password// MQTT Broker
const char *mqtt_broker = "test.mosquitto.org"; // Enter your WiFi or Ethernet IP
const char *topic = "leo";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);
void callback(char *topic, byte *payload, unsigned int length) 
{
 Serial.print("Message arrived in topic: ");
 Serial.println(topic);
 Serial.print("Message:");
 
 for (int i = 0; i < length; i++) {
  Serial.print((char) payload[i]);
 }
 
 Serial.println();
 Serial.println(" - - - - - - - - - - - -");
}

void setup() 
{
 // Set software serial baud to 115200;
 Serial.begin(9600);
 
 // connecting to a WiFi network
 WiFi.begin(ssid, password);
 while (WiFi.status() != WL_CONNECTED) 
 {
  delay(500);
  Serial.println("Connecting to WiFi..");
 }
 
 Serial.println("Connected to the WiFi network");
 
 //connecting to a mqtt broker
 client.setServer(mqtt_broker, mqtt_port);
 client.setCallback(callback);
 
 while (!client.connected()) 
 {
 String client_id = "esp8266-client-";
 client_id += String(WiFi.macAddress());
 
 Serial.printf("The client %s connects to mosquitto mqtt broker\n", client_id.c_str());
 
 if (client.connect(client_id.c_str())) 
 {
  Serial.println("Public emqx mqtt broker connected");
 } 
 else 
 {
  Serial.print("failed with state ");
  Serial.print(client.state());
  delay(2000);
 }
}
 
 // publish and subscribe
 client.publish(topic, "Hello From ESP8266!");
 client.subscribe(topic);
}

void loop() 

{
 client.loop();
}