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
#define LED 3
//create an RF24 object
RF24 radio(CE, CSN); 
//address through which two modules communicate.

const uint64_t endereco_central= 0xF0F0F0F066;
int contador = 0;
float leituras[4];

void setup(){
  Serial.begin(uC_serial);
  delay(2000);
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.openReadingPipe(1, endereco_central);
  radio.startListening();
  if (radio.isChipConnected()){
    Serial.println("Chip conectado");
  }
  pinMode(LED,OUTPUT);
}


float* receber_mensagem(){
  if (radio.available()){
    byte buffer_mensagem[sizeof(leituras)];
    radio.read(&buffer_mensagem,sizeof(buffer_mensagem));
    memcpy(&leituras, buffer_mensagem, sizeof(buffer_mensagem));
    return leituras;
  }
  else{
    return 0;
  }
}

void loop(){
  receber_mensagem();
  if (leituras[2] >= 50){
    digitalWrite(LED,HIGH);
  }
  else{
    digitalWrite(LED,LOW);
  }
  delay(100);
}