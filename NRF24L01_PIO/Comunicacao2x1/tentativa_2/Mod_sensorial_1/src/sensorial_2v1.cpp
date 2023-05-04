//Include Libraries
#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>


#if defined(ARDUINO_ARCH_ESP8266)
  // configuração para ESP8266
  #define CE 0
  #define CSN 2
#elif defined(ARDUINO_ARCH_AVR)
#define CE 9
#define CSN 10
#if defined(ARDUINO_AVR_UNO)
    // configuração para Arduino UNO
    #define tag 1
    #define tempo 0
  #elif defined(ARDUINO_AVR_NANO)
    // configuração para Arduino NANO
    #define tag 2
    #define tempo 1000
  #endif
#endif
#define uC_serial  9600


//create an RF24 object
RF24 radio(CE, CSN);

const uint64_t enderecos[]= {0xF0F0F0F0F000,0xF0F0F0F0F001,0xF0F0F0F0F002};

float mensagem_enviar[3];
float mensagem_receber[2];

int tamanho_mensagem_out = sizeof(mensagem_enviar);
int tamanho_mensagem_in = sizeof(mensagem_receber);


// ==================================================DECLARAÇÕES===========================================================
void enviar_mensagem(float* mensagem, uint64_t endereco)
{
  radio.stopListening();
  radio.openWritingPipe(endereco);
  byte buffer_mensagem[tamanho_mensagem_out];
  memcpy(buffer_mensagem, mensagem, tamanho_mensagem_out);
  radio.write(&buffer_mensagem, sizeof(buffer_mensagem));
  memcpy(mensagem_enviar,mensagem,tamanho_mensagem_out);
}

int receber_mensagem(int pipe, uint64_t endereco)
{
  radio.openReadingPipe(pipe,endereco);
  radio.startListening();
  if (radio.available())
  {
    byte buffer_mensagem[tamanho_mensagem_in];
    radio.read(&buffer_mensagem,sizeof(buffer_mensagem));
    memcpy(mensagem_receber, buffer_mensagem, sizeof(buffer_mensagem));
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
  for (i = 0; i<sizeof(mensagem_enviar)/sizeof(float); i++)
  {
    Serial.println(mensagem_enviar[i]);
  }
}

void printar_recebida()
{
    Serial.println("---------");
    Serial.println("Mensagem Recebida:");
    int i;
    for (i = 0; i<sizeof(mensagem_receber)/sizeof(float); i++)
    {;
      Serial.println(mensagem_receber[i]);
    }
}

// =============================================================================================================================

void setup()
{
  Serial.begin(uC_serial);
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.setChannel(125);
  if(radio.isChipConnected())
  {
    Serial.println("Chip Conectado");
  }
}

//float mensagem[3];
int num_msg_enviadas = 0;
void loop()
{
  uint64_t endereco_central = enderecos[tag];
  long int t_i = millis();
  mensagem_enviar[0] = (float)tag;
  enviar_mensagem(mensagem_enviar,endereco_central);
  printar_enviada();
  Serial.println((int)endereco_central);
  *(mensagem_enviar+1) += 1;
  *(mensagem_enviar+2) += 0.01;
  delay(tempo);
  long int t_f = millis();
  delay(2000-(t_f-t_i)); // garante que ambos sensores farão um ciclo igual
}
