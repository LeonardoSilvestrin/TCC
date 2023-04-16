//Include Libraries
#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>


#if defined(ARDUINO_ARCH_ESP8266)
  // configuração para ESP8266
  #define CE 0
  #define CSN 2
  #define endereco_uno 0xF0F0F0F0AA
#elif defined(ARDUINO_ARCH_AVR)
#define CE 9
#define CSN 10
#if defined(ARDUINO_AVR_UNO)#define endereco 0xF0F0F0F0AA
    // configuração para Arduino UNO
    #define endereco 0xF0F0F0F0AA
  #elif defined(ARDUINO_AVR_NANO)
    // configuração para Arduino NANO
    #define endereco 0xF0F0F0F066
  #endif
#endif
#define uC_serial  9600

//create an RF24 object
RF24 radio(CE, CSN); 
//address through which two modules communicate.
int contador = 0;
float leituras[4];

void setup(){
  Serial.begin(uC_serial);
  delay(2000);
  radio.begin();
  //set the address
  radio.openReadingPipe(0, endereco_uno);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  //Set module as transmitter
   radio.startListening();
  Serial.println("Esperando Mensagem:");
  Serial.println(radio.isChipConnected());
}
void loop(){
//Read the data if available in buffer
  if (radio.available())
  {
    contador++;
    byte buffer_mensagem[sizeof(leituras)];
    radio.read(&buffer_mensagem,sizeof(buffer_mensagem));
    memcpy(&leituras, buffer_mensagem, sizeof(buffer_mensagem));
    Serial.println(" ------------------------------------- Início da mensagem  -----------------------------------");
    Serial.print("início da mensagem recebida, id - ");
    Serial.println(contador);
  for (int i = 0; i<4; i++){
      Serial.print("leitura do sensor: ");
      Serial.print(i+1);
      Serial.print(": ");
      Serial.print(leituras[i]);
      Serial.println("; ");
    };
    Serial.println("");
    Serial.println(" ------------------------------------- fim da transmissão -----------------------------------");
  }
delay(500);
}