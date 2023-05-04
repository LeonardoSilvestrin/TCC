#include <master.h>

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
  setup_sensores();
}



void loop()
{
  for(int i = 0; i< num_sensores;i++)
  {
    receber_mensagem(i,sensores[i].get_endereco());
    filtrar_mensagem(mensagem_recebida);

    if ((int)mensagem_filtrada[0]==sensores[i].get_id())
    {
      if(!(mensagem_repetida(sensores[i],(int)mensagem_filtrada[0],(int)mensagem_filtrada[1])))
      {
        sensores[i].set_ultima_mensagem(mensagem_filtrada);
        printar_mensagem_recebida(sensores[i].get_ultima_mensagem());
      }
    }
    delay(100);
  }
  delay(1000);
}
