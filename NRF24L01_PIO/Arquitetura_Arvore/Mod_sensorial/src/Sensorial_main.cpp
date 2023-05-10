//Include Libraries
#include <master.h>

//create an RF24 object
RF24 radio(CE, CSN);

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
  mensagem_enviada[0] = (float)tag;
  enviar_mensagem(mensagem_enviada,endereco_central);

  Serial.println((int)endereco_central);
  *(mensagem_enviada+1) += 1;
  *(mensagem_enviada+2) += 0.01;
  delay(tempo);
  long int t_f = millis();
  delay(2000-(t_f-t_i)); // garante que ambos sensores farão um ciclo igual
}