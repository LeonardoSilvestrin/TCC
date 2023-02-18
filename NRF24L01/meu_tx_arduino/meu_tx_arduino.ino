//Include Libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//create an RF24 object
RF24 radio(9, 10);  // CE, CSN

const byte address[6] = "00001";
int contador = 0;
float leituras[4];

void setup()
{
  Serial.begin(9600);
  delay(2000);
  radio.begin();
  //set the address
  radio.openWritingPipe(address);
  //Set module as receiver
  radio.stopListening();
  Serial.print("Começando transmissão - ");
  Serial.println(radio.isChipConnected());
}

void loop()
{
  contador++;
  leituras[0]=contador;
  leituras[1]= contador/2;
  leituras[2]= contador*contador;
  leituras[3]= sqrt(contador);
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
