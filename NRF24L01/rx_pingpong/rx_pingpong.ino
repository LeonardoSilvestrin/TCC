//Include Libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//create an RF24 object
RF24 radio(0, 2);  // CE, CSN

//address through which two modules communicate.
const byte address[6] = "00001";
int contador = 0;

struct pacote{
  int contador;
  int mensagem = 20;
  //char mensagem[20];
};

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
    pacote mensagemRecebida;
    radio.read(&mensagemRecebida,sizeof(mensagemRecebida));
    char buffer[50];
    sprintf(buffer,"%d - %d",mensagemRecebida.contador,mensagemRecebida.mensagem);
    Serial.println(mensagemRecebida.contador);
    Serial.println(mensagemRecebida.mensagem);
  }
}
