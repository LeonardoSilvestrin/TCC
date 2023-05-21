#include <Arduino.h>
#include <EEPROM.h>
#include <DHT.h>
#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>
#include <RF24Mesh.h>
#include <ArduinoUniqueID.h>

#if defined(ARDUINO_ARCH_ESP8266)
  // configuração para ESP8266
  #define CE 0
  #define CSN 2
#elif defined(ARDUINO_ARCH_AVR)
#define CE 9
#define CSN 10
  #if defined(ARDUINO_AVR_UNO)
      // configuração para Arduino UNO
  #elif defined(ARDUINO_AVR_NANO)
      // configuração para Arduino NANO
  #endif
#endif

#define tipo 's'
#define baseID 255
#define uC_serial  9600 // velocidade de comunicação do micro via porta serial
#define dht_pin 5

RF24 radio(CE, CSN); // CE, CSN
RF24Network network(radio);
RF24Mesh mesh(radio,network);

int CentralID = 0;
int myID = baseID;
int myOldID;
char myType = 's';
int message_counter;

DHT mydht;
float umidade_do_ar = 0x00, temperatura = 0x00;

int id_update() // solicitação para a central, header = c; conteúdo =  array de 8 bytes contendo o unique id do hardware;
{
  uint8_t message[9];
  for(int i = 0; i<8;i++)
  {
    message[i] = UniqueID[i];
  }
  message[8] = (uint8_t)myType;
  mesh.write(&message,'c',sizeof(message));
  Serial.print("myID: ");
  Serial.println(mesh.getNodeID());
  unsigned long t0 = millis();
  while(millis()-t0<1000) // fica 1 segundo em loop escutando se a central está respondendo
  {
    mesh.update();
    if (network.available()) 
    {
      RF24NetworkHeader header;
      int id;
      network.read(header, &id, sizeof(id));
      Serial.print(F("ID recebido da central: "));
      Serial.println(id);
      return id;
      delay(1);
    }
  }
  Serial.print("Falha, resetando ID para base: ");
  Serial.println(baseID);
  return baseID;
}


bool connect_to_mesh(int ID)
{
  mesh.setNodeID(ID);
  // Connect to the mesh
  Serial.println(F("Conectando à rede..."));
  if (!mesh.begin(125,RF24_250KBPS)) 
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
      return 0;
    }
  }
  if (ID!= baseID)
  {
    Serial.print("Conexão Estabelecida com sucesso, meu id: ");
    Serial.println(mesh.getNodeID());
  }
  return 1;
}

void EEPROM_save_id(int id)
{ 
  EEPROM.write(0, (byte)id);  // Grava o ID do sensor na EEPROM, posição 0
}
int EEPROM_get_id()
{
  byte id;
  EEPROM.get(0,id); //lê o byte da posição 0 da eeprom e retorna como inteiro;
  return (int)id;
}

void setup() 
{
  Serial.begin(uC_serial); //inicia a comunicação serial
  Serial.println("########################################################################");
  
  mydht.setup(dht_pin); //setup do sensor de umidade do ar e temperatura dht11
  
  // setup do radio
  radio.begin();
  radio.setPALevel(RF24_PA_MIN); //potencia mínima, desativa a amplificação de ruído
  radio.setRetries(5, 5); // seta o padrão de 5 retries no envio de mensagem com 5ms de intervalo entre elas
  radio.setChannel(125); //seta o canal 125 para operação
  
  // solicitando ID antigo da EEPROM
  EEPROM.begin();
  myOldID = EEPROM_get_id();
  if (myOldID == 0 || myOldID == baseID)
  {
    myID = baseID;
  }
  else
  {
    myID = myOldID;
  }
  Serial.print("Meu último ID: ");
  Serial.println(myID);

  Serial.print("UniqueID: ");
	for (size_t i = 0; i < UniqueIDsize; i++)
	{
		if (UniqueID[i] < 0x10)
			Serial.print("0");
		  Serial.print(UniqueID[i], HEX);
		  Serial.print(" ");
	}
	Serial.println();
  connect_to_mesh(myID); // conecta na malha com o ID antigo ou o ID 255 para novos sensores
}

void loop() 
{ 
  
  mesh.update();
  myID = id_update();
  while(myID==baseID)
  {
    Serial.println("Solicitando novo id para a central...");
    myID = id_update();
    connect_to_mesh(myID);
    delay(1);
  }
  if (myID == 0) // se a central responder a solicitação de id com 0 o sensor fica em loop infinito.
  {
    myID = baseID;
    message_counter = 0;
    Serial.print("Desconectado da rede.");
    while (1){}      
  }
  Serial.println("Ta chegando");

  EEPROM_save_id(myID);

  // aqui em baixo temos a função ler sensores e printar:
  // ==============================================================
  // umidade_do_ar = mydht.getHumidity();
  // temperatura = mydht.getTemperature();
  // int umidade_do_solo_in = analogRead(A0);
  // float umidade_do_solo = map(umidade_do_solo_in,0,1024,0,100);
  // Serial.println("------------------");
  // Serial.print("Umidade do ar: ");
  // Serial.println(umidade_do_ar);
  // Serial.print("Umidade do solo: ");
  // Serial.println(umidade_do_solo);
  // Serial.print("temperatura: ");
  // Serial.println(temperatura);
  // ==============================================================
  delay(100);
}