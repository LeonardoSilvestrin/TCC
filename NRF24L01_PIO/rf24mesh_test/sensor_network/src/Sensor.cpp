/*
#include<Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>
#include <RF24Mesh.h>

#if defined(ARDUINO_ARCH_ESP8266)
  // configuração para ESP8266
  #define CE 0
  #define CSN 2
#elif defined(ARDUINO_ARCH_AVR)
#define CE 9
#define CSN 10
  #if defined(ARDUINO_AVR_UNO)
      // configuração para Arduino UNO
      #define tipo 's'
  #elif defined(ARDUINO_AVR_NANO)
      #define tipo 'i'
      // configuração para Arduino NANO
  #endif
#endif

#define baseID 1
#define uC_serial  9600 // velocidade de comunicação do micro via porta serial


RF24 radio(CE, CSN); // CE, CSN
RF24Network network(radio);
RF24Mesh mesh(radio,network);

int CentralID = 0;
int myID = baseID;

int get_new_id() // solicitação para a central, header = c; conteúdo = 1 (solicitação de id)
{ 
  mesh.write(&myID,'c',sizeof(myID));
  Serial.print("myID: ");
  Serial.println(myID);
  unsigned long t0 = millis();
  while(millis()-t0<5000)
  {
    mesh.update();
    if (network.available()) 
    {
      RF24NetworkHeader header;
      int id;
      network.read(header, &id, sizeof(id));
      Serial.print(F("Novo Id recebido da central: "));
      Serial.println(id);
      Serial.print("Sucesso, ID: ");
      Serial.println(id);
      return id;
    }
    delay(1);
  }
  
  Serial.print("Falha:, ID: ");
  Serial.println(baseID);
  return baseID;
}

void connect_to_mesh(int ID)
{
  mesh.setNodeID(ID); // todos os nós começam com ID 1 para solicitar à central seu ID fixo na rede
  // Connect to the mesh
  Serial.println(F("Conectando à rede..."));
  if (!mesh.begin()) 
  {
    if (radio.isChipConnected()) 
    {
      do 
      {
        Serial.println(F("Falha ao conectar à rede\nEnviando nova solicitação de entrada..."));
        // mesh.renewAddress() will return MESH_DEFAULT_ADDRESS on failure to connect
      } 
      while (mesh.renewAddress() == MESH_DEFAULT_ADDRESS);
    } 
    else 
    {
      Serial.println(F("Antena offline."));
    }
  }
}

void setup() {
 
  Serial.begin(uC_serial);
  Serial.flush();
  Serial.println("########################################################################");
  radio.begin();
  radio.setPALevel(RF24_PA_MIN, 0);
  // Set the baseID
  
  connect_to_mesh(myID);

}

void loop() 
{ 
  mesh.update();
  if (myID == baseID)
  {
    while(myID==baseID)
    {
      Serial.println("Solicitando id para a central...");
      myID = get_new_id();
      delay(1000);
    }
    connect_to_mesh(myID);
    Serial.print("Conexão bem sucedida com a rede, meu id: ");
    Serial.println(mesh.getNodeID());
  }
}
*/


#include<Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>
#include <RF24Mesh.h>

#if defined(ARDUINO_ARCH_ESP8266)
  // configuração para ESP8266
  #define CE 0
  #define CSN 2
#elif defined(ARDUINO_ARCH_AVR)
#define CE 9
#define CSN 10
  #if defined(ARDUINO_AVR_UNO)
      // configuração para Arduino UNO
      #define tipo 's'
  #elif defined(ARDUINO_AVR_NANO)
      #define tipo 'i'
      // configuração para Arduino NANO
  #endif
#endif

#define baseID 1
#define uC_serial  9600 // velocidade de comunicação do micro via porta serial


RF24 radio(CE, CSN); // CE, CSN
RF24Network network(radio);
RF24Mesh mesh(radio,network);

int CentralID = 0;
int myID = baseID;

int get_new_id() // solicitação para a central, header = c; conteúdo = 1 (solicitação de id)
{ 
  int message[2] = {myID, 10};
  byte buffer[sizeof(message)];  // Buffer with payload size of 12 bytes
  memcpy(buffer, &message, sizeof(message)); 
  
  mesh.write(&buffer,'c',sizeof(buffer));
  
  Serial.print("myID: ");
  Serial.println(myID);
  unsigned long t0 = millis();
  while(millis()-t0<5000)
  {
    mesh.update();
    if (network.available()) 
    {
      RF24NetworkHeader header;
      int id;
      network.read(header, &id, sizeof(id));
      Serial.print(F("Novo Id recebido da central: "));
      Serial.println(id);
      Serial.print("Sucesso, ID: ");
      Serial.println(id);
      return id;
    }
    delay(1);
  }
  
  Serial.print("Falha:, ID: ");
  Serial.println(baseID);
  return baseID;
}

void connect_to_mesh(int ID)
{
  mesh.setNodeID(ID); // todos os nós começam com ID 1 para solicitar à central seu ID fixo na rede
  // Connect to the mesh
  Serial.println(F("Conectando à rede..."));
  if (!mesh.begin()) 
  {
    if (radio.isChipConnected()) 
    {
      do 
      {
        Serial.println(F("Falha ao conectar à rede\nEnviando nova solicitação de entrada..."));
        // mesh.renewAddress() will return MESH_DEFAULT_ADDRESS on failure to connect
      } 
      while (mesh.renewAddress() == MESH_DEFAULT_ADDRESS);
    } 
    else 
    {
      Serial.println(F("Antena offline."));
    }
  }
}

void setup() {
 
  Serial.begin(uC_serial);
  Serial.flush();
  Serial.println("########################################################################");
  radio.begin();
  radio.setPALevel(RF24_PA_MIN, 0);
  // Set the baseID
  
  connect_to_mesh(myID);

}

void loop() 
{ 
  mesh.update();
  if (myID == baseID)
  {
    while(myID==baseID)
    {
      Serial.println("Solicitando id para a central...");
      myID = get_new_id();
      delay(1000);
    }
    connect_to_mesh(myID);
    Serial.print("Conexão bem sucedida com a rede, meu id: ");
    Serial.println(mesh.getNodeID());
  }
}