//Include Libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//create an RF24 object
RF24 radio(9, 10);  // CE, CSN

//address through which two modules communicate.
const byte address[6] = "00001";
int contador = 0;

struct pacote{
  int contador;
  char mensagem[20];
};
void setup()
{
  Serial.begin(9600);
  delay(2000);

  radio.begin();

  //set the address
  //set the address
  radio.openWritingPipe(address);

  //Set module as receiver
  radio.stopListening();

  Serial.println("Começando transmissão");
  Serial.println(radio.isChipConnected());
}

void loop()
{
  contador++;
  pacote mensagemEnviar;
  mensagemEnviar.contador = contador;
  strcpy(mensagemEnviar.mensagem, " Hello World");
  radio.write(&mensagemEnviar, sizeof(mensagemEnviar));
  char buffer[50];
  sprintf(buffer,"%d - %s",mensagemEnviar.contador,mensagemEnviar.mensagem);
  Serial.println(buffer);
  delay(2000);
}
