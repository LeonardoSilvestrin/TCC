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

const uint64_t endereco_sensor_1 = 0xF0F0F0F0AA;
const uint64_t endereco_sensor_2 = 0xF0F0F0F066;
int contador = 0;
float leituras[4];

void setup(){
  Serial.begin(uC_serial);
  delay(2000);
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.openReadingPipe(1, endereco_sensor_1);
  radio.openReadingPipe(2, endereco_sensor_2);
  radio.startListening();
  if (radio.isChipConnected()){
    Serial.println("Chip conectado");
  }
}

void printar_mensagem(int contador, float* leituras){
  Serial.print("Número de mensagens recebidas: ");
  Serial.println(contador);
  Serial.println(" --------------------- Início da mensagem  ------------------------");
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

float receber_mensagem(int contador){
  if (radio.available()){
    byte buffer_mensagem[sizeof(leituras)];
    radio.read(&buffer_mensagem,sizeof(buffer_mensagem));
    memcpy(&leituras, buffer_mensagem, sizeof(buffer_mensagem));
    printar_mensagem(contador, leituras);
    return 1;
  }
  else{
    Serial.println("Falha de conexão com o sensor ");
    return 0;
  }
}

void loop(){
  receber_mensagem(contador);
  receber_mensagem(contador);
  delay(5000);
}