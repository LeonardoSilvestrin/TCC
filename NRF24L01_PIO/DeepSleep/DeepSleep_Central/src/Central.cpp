//Include Libraries
#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>

#if defined(ARDUINO_ARCH_ESP8266)   // configuração para ESP8266
  #define CE 0
  #define CSN 2
  #define tempo 1000
#elif defined(ARDUINO_ARCH_AVR) // configuração para UNO
  #define CE 9
  #define CSN 10
  #define tempo 2000
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
  radio.startListening();
  radio.openReadingPipe(0,enderecos[0]);
  if (radio.available())
  {
    byte buffer_mensagem[tamanho_mensagem_in];
    radio.read(&buffer_mensagem,sizeof(buffer_mensagem));
    memcpy(&mensagem_recebida, buffer_mensagem, sizeof(buffer_mensagem));
    //radio.read(&mensagem_recebida, tamanho_mensagem_in);
    return 1;
  }
  else
  {
    return 0;
  }
}

void enviar_e_printar()
{
  enviar_mensagem(mensagem_enviar);
  int i;
  *(mensagem_enviar+1)= *(mensagem_enviar+1) + 0.01;
  Serial.println("---------");
  Serial.println("Mensagem Enviada:");
  for (i = 0; i<2; i++)
  {
    Serial.println(mensagem_enviar[i]);
  }

}

void receber_e_printar()
{
  receber_mensagem();
  if(receber_mensagem())
  {
    Serial.println("---------");
    Serial.println("Mensagem Recebida:");
    int i;
    for (i = 0; i<2; i++)
    {;
      Serial.println(mensagem_recebida[i]);
    }
  }
  else
  {
    Serial.println("Falha de Conexão");
  }
}

void setup()
{
  Serial.begin(uC_serial);
  radio.begin();
  radio.enableAckPayload();
  radio.enableDynamicPayloads();
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MIN);
  if(radio.isChipConnected())
  {
    Serial.println("Chip Conectado");
  }
}


void loop()
{
  int delta = 0;
  int* ponteiro_delta = &delta;
  long int t0 = millis();
  int voltas = 0;
  while(millis() - t0 < 5000)
  {
    if (voltas > 0)
    {
      *ponteiro_delta = 100*receber_mensagem();
    }
    else
    {
      receber_e_printar();
    }
    voltas ++;
    delay(1000);
  }
  long int t1 = millis();
  int voltas_2 = 0;
  while(millis() - t1 < 5000)
  {
    if (voltas_2 > 0)
    {
      enviar_mensagem(mensagem_enviar);
    }
    else
    {
      enviar_e_printar();
    }
    voltas_2++;
    delay(500);
  }
  delay(1000+delta);
}

