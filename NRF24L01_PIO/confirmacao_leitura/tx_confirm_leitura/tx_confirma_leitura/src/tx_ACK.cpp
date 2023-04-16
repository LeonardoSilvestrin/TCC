//Include Libraries
#include <Arduino.h>
#include <SPI.h>
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
const byte address[6] = "00001"; // Endereço de envio
unsigned int contador = 0; // Inicializa o contador em zero

void setup() {
  Serial.begin(9600);
  radio.begin(); // Inicia a comunicação com o módulo NRF24L01
  radio.setPALevel(RF24_PA_MIN); // Define a potência de transmissão do módulo
  radio.openWritingPipe(address); // Define o endereço do receptor para envio
}

void loop() {
  radio.stopListening(); // Coloca o módulo em modo de transmissão
  radio.write(&contador, sizeof(contador)); // Envia o valor do contador
  Serial.print("Contador enviado: ");
  Serial.println(contador);
  long int tempo = millis();
  while (millis()-tempo < 5000){
    if (radio.isAckPayloadAvailable()) { // Verifica se há um ACK disponível
      unsigned int ack_contador; // Variável para armazenar o valor do contador recebido no ACK
      radio.read(&ack_contador, sizeof(ack_contador)); // Lê o valor do contador no ACK recebido
      Serial.print("ACK recebido: ");
      Serial.println(ack_contador);
    }
  }

  contador++; // Incrementa o contador
  delay(1000); // Aguarda 1 segundo antes de enviar o próximo pacote
}

