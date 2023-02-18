//Include Libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//create an RF24 object
RF24 radio(0, 2);  // CE, CSN

//address through which two modules communicate.
const byte address[6] = "00001";
int contador = 0;
float leituras[4];
void setup()
{
  Serial.begin(9600);
  delay(2000);
  radio.begin();
  //set the address
  radio.openReadingPipe(0, address);

  //Set module as transmitter
   radio.startListening();
  Serial.println("Esperando Mensagem:");
  Serial.println(radio.isChipConnected());
}
void loop()
{
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
}
