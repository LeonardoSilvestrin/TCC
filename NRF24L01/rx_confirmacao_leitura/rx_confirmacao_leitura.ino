//Include Libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE 0
#define CSN 2

//create an RF24 object
RF24 radio(CE, CSN);  // CE, CSN

//canal_de_recepcao through which two modules communicate.
const byte canal_de_recepcao[6] = "00001";
const byte canal_de_transmissao[6] = "00010";

int mensagem = -1;
bool ack;

void setup()
{
  Serial.begin(9600);
  delay(2000);
  radio.begin(); 
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.openWritingPipe(canal_de_transmissao);
  radio.openReadingPipe(0,canal_de_recepcao);
  if(radio.isChipConnected()){
    Serial.println("Chip Conectado");
    Serial.println("Iniciando Transmissão!");
    Serial.println("Esperando Mensagem:");
  }
}


void receber_mensagem(bool *ack, int *mensagem){
  radio.startListening();
  if (radio.available()){
    byte buffer_mensagem[sizeof(mensagem)];
    radio.read(&buffer_mensagem,sizeof(buffer_mensagem));
    memcpy(&mensagem, &buffer_mensagem, sizeof(buffer_mensagem));
    *ack = 1;
  }
}
void printar_mensagem_recebida(int mensagem, int ack){
    Serial.println(" ------------------------------------- Início da mensagem  -----------------------------------");
    Serial.println(mensagem);
    Serial.println(ack);
    Serial.println(" -------------------------------- mensagem recebida com sucesso ------------------------------");
}
void loop(){
  receber_mensagem(&ack, &mensagem);
  if (ack != 0){
    printar_mensagem_recebida(mensagem, ack);
  long int t0 = millis();
  radio.stopListening();
  while(ack){
    long int tempo_passado = millis()-t0;
    byte buffer_ack[sizeof(ack)];
    memcpy(&buffer_ack, &ack, sizeof(buffer_ack));
    radio.write(&buffer_ack,sizeof(buffer_ack));
    delay(100);
    if (tempo_passado > 5000){
      ack = 0;
      }
  delay(50);
  }
  }
}
