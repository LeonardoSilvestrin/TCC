#include <master.h>

void setup()
{
  network.begin(125,endereco_central);
  Serial.begin(uC_serial);
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.setChannel(125);
  if(radio.isChipConnected())
  {
    Serial.println("Chip Conectado");
  }
  gerar_enderecos();
  setup_sensores();
}

void loop()
{
    
}
