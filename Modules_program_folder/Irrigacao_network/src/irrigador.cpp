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
#define dht_pin 2 //D2
#define LED_pin 3 //D3

RF24 radio(CE, CSN); // CE, CSN
RF24Network network(radio);
RF24Mesh mesh(radio,network);

int CentralID = 0;
int myID = baseID;
int myOldID = baseID;
char myType = 's';

DHT mydht;

int id_update()
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
  while(millis()-t0<500) // fica 1 segundo em loop escutando se a central está respondendo
  {
    mesh.update();
    if (network.available()) 
    {
      RF24NetworkHeader header;
      int id;
      network.read(header, &id, sizeof(id));
      Serial.print(F("ID recebido da central: "));
      Serial.println(id);
      mesh.releaseAddress();
      delay(1);
      return id;
    }
  }
  Serial.print("Falha, resetando ID para base: ");
  Serial.println(baseID);
  return baseID;
}

bool send_data(float bateria, float temperatura, float umidade_do_solo)
{
  float message[3];
  message[0] = bateria;
  message[1] = temperatura;
  message[2] = umidade_do_solo;
  
  unsigned long t0 = millis();
  while(millis()-t0<1000)
  {
    if(mesh.write(&message,'d',sizeof(message)))
    {
      Serial.println("Dados enviados:");
      Serial.println(*message);
      Serial.println(*(message+1));
      Serial.println(*(message+2));
      return 1;
    }
    delay(1);
  }
  Serial.println("Timeout. Sem resposta da central");
  return 0;
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
  if ((int)id == 0 || (int)id == baseID)
  {
    id = (byte)baseID;
  }
  return (int)id;
}

void setup() 
{
  //---------------------------------------------------------------
  // Inicia a comunicação serial
  Serial.begin(uC_serial); 
  Serial.println("########################################################################");
  
  //---------------------------------------------------------------
  // Inicia o sensor de umidade do ar e temperatura dht11
  pinMode(dht_pin, INPUT);
  pinMode(LED_pin, OUTPUT);
  mydht.setup(dht_pin); 
  
  //---------------------------------------------------------------
  // Setup do radio
  radio.begin();
  radio.setPALevel(RF24_PA_MIN); //potencia mínima, desativa a amplificação de ruído
  radio.setRetries(5, 5); // seta o padrão de 5 retries no envio de mensagem com 1500us (5*250 + 250) de intervalo entre elas
  radio.setChannel(125); //seta o canal 125 para operação => 2,525 GHz (2,4 + 0,125) 
  
  //--------------------------< Extraindo ID registrado na EEPROM >-------------------------------------
  // Inicia a EEPROM e lẽ o último ID registrado
  // EEPROM.begin();
  // myOldID = EEPROM_get_id();
  myOldID = 255;
  //---------------------------------------------------------------
  // Printa o último ID de rede do módulo bem como o número de série

  Serial.print("Meu último ID: ");
  Serial.println(myOldID);
  Serial.print("UniqueID: ");
	for (size_t i = 0; i < UniqueIDsize; i++)
	{
		if (UniqueID[i] < 0x10)
			Serial.print("0");
		  Serial.print(UniqueID[i], HEX);
		  Serial.print(" ");
	}
	Serial.println();
  //---------------------------------------------------------------
  // conecta na malha com o ID antigo (ID 255 para novos sensores)
  
  connect_to_mesh(myID);  
  mesh.update();
  myID = id_update();
  connect_to_mesh(myID);
  while(myID==baseID)
  {
    Serial.println("Solicitando novo id para a central...");
    myID = id_update();
    delay(1);
    connect_to_mesh(myID);
  }
  // se a central responder a solicitação de id com 0 o sensor fica em loop infinito.
  if (myID == 0) 
  {
    myID = baseID;
    mesh.releaseAddress();
    Serial.print("Desconectado da rede.");
    while (1){}      
  }
  //---------------------------------------------------------------
  // EEPROM_save_id(myID);
}

const float msec_to_mins = 60UL*1000ul;
unsigned long t_cycle = .33*msec_to_mins;

void loop() 
{ 
  bool sent_data = false;
  myID = mesh._nodeID;
  if(myID != baseID)
  {
    // ------------------------------------------------------------
    // data loop
    unsigned long t_inicial_coleta_dados = millis();
    unsigned long t_max_para_enviar_dados = 5000;
    while(millis()-t_inicial_coleta_dados<t_max_para_enviar_dados)
    {
      // float bateria = 19;
      // float temperatura = 2.5+cc;
      // float umidade_do_solo = 70;
      float bateria = 100;
      float temperatura = mydht.getTemperature();
      int umidade_do_solo_in = analogRead(A0);
      float umidade_do_solo = map(umidade_do_solo_in,0,1024,0,100);
      if (umidade_do_solo<50)
      {
        digitalWrite(LED_pin,HIGH);
      }
      else
      {
        digitalWrite(LED_pin,LOW);
      }
      if(send_data(bateria, temperatura, umidade_do_solo))
      {
        sent_data = true;
        break;
      }
    }
   unsigned long t_final_coleta_dados = millis();
   Serial.print("Fim do ciclo, tempo:");
   Serial.println(t_final_coleta_dados);
   Serial.print("Central recebeu dados neste ciclo? ");
   Serial.println(sent_data);
   Serial.print("Meu ID de rede:");
   Serial.println(mesh.getNodeID());
   // ------------------------------------------------------------ 
   if(sent_data)
   {
    delay(t_cycle - (t_final_coleta_dados-t_inicial_coleta_dados));
   }
   else
   {
    delay(100);
   }
  }
  else
  {
    Serial.print("Erro, meu ID: ");
    Serial.println(mesh.getNodeID());
    Serial.println("Solicitando novo id para a central...");
    myID = id_update();
    connect_to_mesh(myID);
    delay(1);
  }
}