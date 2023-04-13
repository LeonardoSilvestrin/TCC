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

//create an RF24 object
RF24 radio(CE, CSN);  // CE, CSN

const byte canal_de_transmissao[6] = "00001";
const byte canal_de_recepcao[6] = "00010";

int contador_de_pacotes =  0; 
int contador = 0;
int ack;

void setup(){
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
  }
}

void transmitir_mensagem(int mensagem) {
    radio.stopListening();
    radio.openWritingPipe(canal_de_transmissao);
    byte buffer_mensagem[sizeof(mensagem)];
    memcpy(buffer_mensagem, &mensagem, sizeof(mensagem));
    radio.write(&buffer_mensagem, sizeof(buffer_mensagem));
}
void printar_mensagem_transmitida(int mensagem){
    Serial.print("mensagens enviadas: ");
    Serial.println(contador_de_pacotes);
    Serial.print("mensagens enviadas com sucesso: ");
    Serial.println(mensagem);
    Serial.println("Aguardando Resposta");
}
//char aguardar_resposta();
  

void loop()
{
  bool transmitir = 1;
  Serial.println(" ------------------------------------- Início da transmissão  -------------------------------");
  while (transmitir){
    int mensagem = contador;
    long int t_transmissao_0 = millis();
    long int t_transmissao;
    while(t_transmissao < 5000){
      t_transmissao = t_transmissao_0 - millis();
      transmitir_mensagem(mensagem);
    }
    printar_mensagem_transmitida(mensagem);
    long int t0 = millis();
    bool resposta_recebida = 0;
    radio.startListening();
    Serial.print("esperando resposta");
    while(!resposta_recebida){
     long int tempo_espera = millis()-t0;
     if (radio.available()) {
       byte buffer_ack[sizeof(ack)];
       radio.read(&buffer_ack,sizeof(buffer_ack));
       memcpy(&ack, buffer_ack, sizeof(buffer_ack));
       if (ack == 1) {
         resposta_recebida = 1;
         contador_de_pacotes++;
         Serial.println("Dados enviados com sucesso!");
         Serial.println(" ------------------------------------- fim da transmissão -----------------------------------");
         transmitir = 0;
         break;
        }
     }
     if (tempo_espera>5000){
        Serial.println("timeout. Reenviando mensagem...");
        break; 
     }
    }
    contador_de_pacotes++;  
    delay(50);
  }
}
