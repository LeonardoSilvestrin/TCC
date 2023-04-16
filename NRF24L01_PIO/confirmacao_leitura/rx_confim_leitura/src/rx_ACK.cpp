//Include Libraries
#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#ifdef ARDUINO_ARCH_ESP8266 // muda a configuração dos pinos dependendo da placa que estou usando
// CE, CSN esp8266
#define CE 0
#define CSN 2
// CE, CSN arduino
#else
#define CE 9
#define CSN 10
#endif 
  
RF24 radio(CE, CSN); // CE, CSN
const byte endereco[6] = "00001"; // Endereço de envio
  
void setup() {
  Serial.begin(9600);
  radio.begin(); // Inicia a comunicação com o módulo NRF24L01
  radio.setPALevel(RF24_PA_MIN); // Define a potência de transmissão do módulo
  radio.openReadingPipe(1, endereco); // Define o endereço do receptor para recepção
  radio.startListening();
}

void loop() {
  if (radio.available()) { // Verifica se há dados disponíveis para leitura
    int contador; // Variável para armazenar o valor do contador recebido
    radio.read(&contador, sizeof(contador)); // Lê o valor do contador

    Serial.print("Contador recebido: ");
    Serial.println(contador);
    long int tempo = millis();
    while (millis()-tempo < 6000){
      radio.writeAckPayload(1, &contador, sizeof(contador)); // Envia o ACK com o valor do contador recebido
  }
}
}