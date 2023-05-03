//Include Libraries
#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>

#if defined(ARDUINO_ARCH_ESP8266)   // configuração para ESP8266
  #define CE 0
  #define CSN 2
#elif defined(ARDUINO_ARCH_AVR) // configuração para UNO
  #define CE 9
  #define CSN 10
#endif

#define uC_serial  9600 // velocidade de comunicação do micro via porta serial


RF24 radio(CE, CSN); // criando um objeto radio da biblioteca RF24

const uint64_t enderecos[]= {0xF0F0F0F0F000,0xF0F0F0F0F001,0xF0F0F0F0F002};

float mensagem_enviar[] = {1.0,1.0};
int tamanho_mensagem_out = sizeof(mensagem_enviar);
float mensagem_recebida[2];
int tamanho_mensagem_in = sizeof(mensagem_recebida);
//struct pacote 
//{
//    int id;
//    float dado;
//    char mensagem;
//};
//
//typedef struct pacote mensagem_enviar;



void enviar_mensagem(float* mensagem)
{
  radio.stopListening();
  radio.openWritingPipe(enderecos[1]);
  byte buffer_mensagem[tamanho_mensagem_out];
  memcpy(buffer_mensagem, mensagem, tamanho_mensagem_out);
  radio.write(&buffer_mensagem, sizeof(buffer_mensagem));
  //radio.write(&mensagem,tamanho_mensagem_out);
}

int receber_mensagem()
{
  radio.openReadingPipe(0,enderecos[0]);
  radio.startListening();
  if (radio.available())
  {
    byte buffer_mensagem[tamanho_mensagem_in];
    radio.read(&buffer_mensagem,sizeof(buffer_mensagem));
    memcpy(mensagem_recebida, buffer_mensagem, sizeof(buffer_mensagem));
    //radio.read(&mensagem_recebida, tamanho_mensagem_in);
    return 1;
  }
  else
  {
    return 0;
  }
}

void printar_enviada()
{
  int i;
  Serial.println("---------");
  Serial.println("Mensagem Enviada:");
  for (i = 0; i<2; i++)
  {
    Serial.println(mensagem_enviar[i]);
  }
}

void printar_recebida()
{
    Serial.println("---------");
    Serial.println("Mensagem Recebida:");
    int i;
    for (i = 0; i<2; i++)
    {;
      Serial.println(mensagem_recebida[i]);
    }
}

void setup()
{
  Serial.begin(uC_serial);
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.setChannel(100);
  if(radio.isChipConnected())
  {
    Serial.println("Chip Conectado");
  }
  pinMode(3,OUTPUT);
}

int voltas = 0;
void loop()
{
  receber_mensagem();
  delay(10);
  enviar_mensagem(mensagem_enviar);
  if(voltas%100== 0)
  {
    *(mensagem_enviar+1)= *(mensagem_enviar+1) + 0.01;
  printar_enviada();
    printar_recebida();

  mensagem_recebida[0] = -1.0;
  mensagem_recebida[1] = -1.0;
  }
  delay(10);
  voltas++;
}

