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
#define uC_serial 9600

//create an RF24 object
RF24 radio(CE, CSN);

const byte address[6] = "00001";
int contador = 0;
float leituras[4];
  
void setup(){
  Serial.begin(uC_serial);
  delay(2000);
  radio.begin();
  //set the address
  radio.openWritingPipe(address);
  //Set module as receiver
  radio.stopListening();
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  Serial.print("Começando transmissão - ");
  Serial.println(radio.isChipConnected());
}

void loop()
{
  contador++;
  leituras[0]=contador;
  leituras[1]= contador/2;
  leituras[2]= contador*2;
  leituras[3]= 0;
  byte buffer_mensagem[sizeof(leituras)];
  memcpy(buffer_mensagem, &leituras, sizeof(leituras));
  radio.write(&buffer_mensagem, sizeof(buffer_mensagem));
  Serial.println(" ------------------------------------- Início da transmissão  -------------------------------");
  Serial.print(" id - ");
  Serial.println(contador);
  for (int i = 0; i<4; i++){
      Serial.print("leitura do sensor ");
      Serial.print(i+1);
      Serial.print(": ");
      Serial.print(leituras[i]);
      Serial.println("; ");
   };
   Serial.println("");
   Serial.println(" ------------------------------------- fim da transmissão -----------------------------------");
  delay(2000);
}
