////Include Libraries
//#include <Arduino.h>
//#include <SPI.h>
//#include <RF24.h>
//#include <nRF24L01.h>
//
//#ifdef ARDUINO_ARCH_ESP8266 // muda a configuração dos pinos dependendo da placa que estou usando
//// CE, CSN esp8266
//#define CE 0
//#define CSN 2
//// CE, CSN arduino
//#else
//#define CE 9
//#define CSN 10
//#endif 
//#define uC_serial 9600
////create an RF24 object
//RF24 radio(CE, CSN); 
////address through which two modules communicate.
//
//const uint64_t endereco_mod_sensorial = 0xF0F0F0F0AA;
//const uint64_t endereco_mod_irrigacao = 0xF0F0F0F066;
//int contador = 0;
//float leituras[4];
//
//void setup(){
//  Serial.begin(uC_serial);
//  delay(2000);
//  radio.begin();
//  radio.setDataRate(RF24_250KBPS);
//  radio.setPALevel(RF24_PA_LOW);
//  radio.openReadingPipe(1, endereco_mod_sensorial);
//  radio.openWritingPipe(endereco_mod_irrigacao);
//  radio.startListening();
//  if (radio.isChipConnected()){
//    Serial.println("Chip conectado");
//  }
//}
//
//void printar_mensagem(int contador, float* leituras){
//  Serial.print("Número de mensagens recebidas: ");
//  Serial.println(contador);
//  Serial.println(" --------------------- Início da leituras  ------------------------");
//  Serial.print("ID do Módulo Sensorial: ");
//  Serial.println(leituras[0]);
//  for (int i = 0; i<4; i++){
//      Serial.print("Leitura do Sensor ");
//      Serial.print(i+1);
//      Serial.println(":");
//      Serial.println(leituras[i]);
//    };
//  Serial.println(" ----------------- fim da transmissão -------------------------");
//}
//
//void enviar_mensagem(float* leituras, int size, uint64_t endereco){
//  radio.stopListening();
//  byte buffer_chega_mensagem[size];
//  memcpy(buffer_chega_mensagem, leituras, size);
//  radio.write(&buffer_chega_mensagem, sizeof(buffer_chega_mensagem));
//}
//
//float* receber_mensagem(int contador){
//  if (radio.available()){
//    byte buffer_chega_mensagem[sizeof(leituras)];
//    radio.read(&buffer_chega_mensagem,sizeof(buffer_chega_mensagem));
//    memcpy(&leituras, buffer_chega_mensagem, sizeof(buffer_chega_mensagem));
//    printar_mensagem(contador, leituras);
//    return leituras;
//  }
//  else{
//    return NULL;
//  }
//}
//
//void loop(){
//  float* leituras = receber_mensagem(contador);
//  contador +=1;
//  enviar_mensagem(leituras,sizeof(float)*4,endereco_mod_irrigacao);
//  delay(10);
//}


//Include Libraries
#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>

#ifdef ARDUINO_ARCH_ESP8266 // muda a configuração dos pinos dependendo da placa que estou usando
// CE, CSN esp8266
#define CE 0
#define CSN 2
// CE, CSN arduino
#else
#define CE 9
#define CSN 10
#endif 
#define uC_serial 9600
//create an RF24 object
RF24 radio(CE, CSN); 
//address through which two modules communicate.

const uint64_t endereco_mod_sensorial = 0xF0F0F0F0AA;
const uint64_t endereco_mod_irrigacao = 0xF0F0F0F066;
int contador = 0;
float leituras[4];

void setup(){
  Serial.begin(uC_serial);
  delay(2000);
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.openReadingPipe(1, endereco_mod_sensorial);
  radio.openWritingPipe(endereco_mod_irrigacao);
  radio.startListening();
  if (radio.isChipConnected()){
    Serial.println("Chip conectado");
  }
}

void printar_mensagem(int contador, float* leituras){
  Serial.print("Número de mensagens recebidas: ");
  Serial.println(contador);
  Serial.println(" --------------------- Início da leituras  ------------------------");
  Serial.print("ID do Módulo Sensorial: ");
  Serial.println(leituras[0]);
  for (int i = 0; i<4; i++){
      Serial.print("Leitura do Sensor ");
      Serial.print(i+1);
      Serial.println(":");
      Serial.println(leituras[i]);
    };
  Serial.println(" ----------------- fim da transmissão -------------------------");
}


void loop(){
  float leituras[4];
  bool mensagem_recebida = 0;
  radio.startListening();
  if (radio.available()){
    byte buffer_chega_mensagem[sizeof(leituras)];
    radio.read(&buffer_chega_mensagem,sizeof(buffer_chega_mensagem));
    memcpy(&leituras, buffer_chega_mensagem, sizeof(buffer_chega_mensagem));
    printar_mensagem(contador, leituras);
    contador +=1;
    mensagem_recebida = 1;
  }
  if (mensagem_recebida = 1)
  {
  byte buffer_mensagem_sai[sizeof(leituras)];
  radio.stopListening();
  memcpy(buffer_mensagem_sai, leituras, sizeof(leituras));
  radio.write(&buffer_mensagem_sai, sizeof(buffer_mensagem_sai));
  }
  else
  {
    delay(100);
  }
}