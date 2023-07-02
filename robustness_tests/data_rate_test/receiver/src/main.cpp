#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>


#if defined(ARDUINO_ARCH_ESP8266)
  #define CE 0
  #define CSN 2
#elif defined(ARDUINO_ARCH_AVR)
  #define CE 9
  #define CSN 10
#endif

RF24 radio(CE, CSN);


void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  radio.begin();
  radio.setPALevel(RF24_PA_MIN); //potencia mínima, desativa a amplificação de ruído
  radio.setRetries(5, 5); // seta o padrão de 5 retries no envio de mensagem com 1500us (5*250 + 250) de intervalo entre elas
  radio.setChannel(125); //seta o canal 125 para operação => 2,525 GHz (2,4 + 0,125) 

  const uint64_t address = 1; // Replace with your desired address
  radio.openReadingPipe(1, address); 
  radio.startListening();
}

int contador; // Assuming the received data size is 32 bytes or less
uint8_t receivedData;
int *pointer_to_data;
void loop() {
  if(radio.available())
  {
    radio.read(&receivedData, sizeof(receivedData));
    Serial.println(receivedData);
  }
  delay(10);
}