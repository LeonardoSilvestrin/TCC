//Include Libraries
#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>

#define CE 9
#define CSN 10
#define endereco 0xF0F0F0F0AA
#define uC_serial  9600


//create an RF24 object
RF24 radio(CE, CSN);


int contador = 0;
float leituras[4];

float sensorID(const uint64_t endereco_main){ 
  switch (endereco_main) {
    case 0xF0F0F0F0AA:
      return 1.0;
    case 0xF0F0F0F066:
      return 2.0;
    default:
      return -1.0; // or any other appropriate error value
  }
}

void setup(){
  Serial.begin(uC_serial);
  delay(2000);
  radio.begin();
  radio.openWritingPipe(endereco);
  radio.stopListening();
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  Serial.print("Começando transmissão - ");
  Serial.println(radio.isChipConnected());
}

void enviar_mensagem(float* mensagem, int size){
  byte buffer_mensagem[size];
  memcpy(buffer_mensagem, mensagem, size);
  radio.write(&buffer_mensagem, sizeof(buffer_mensagem));
}

void loop()
{
  int leitura_sensor_discreta = analogRead(A0);
  float leitura_sensor = map(leitura_sensor_discreta, 0, 1023, 0, 100); 
  contador++;
  leituras[0]= sensorID(endereco);
  leituras[1]= contador;
  leituras[2]= leitura_sensor;
  leituras[3]= 0;

  enviar_mensagem(leituras,sizeof(leituras));
  
  // -------------------------- printar mensagem enviada ---------------------------
  Serial.println(" ------------------------------------- Início da transmissão  -------------------------------");
  Serial.print("ID do sensor: ");
  Serial.println(leituras[0]);
  for (int i = 1; i<4; i++){
      Serial.print("leitura do sensor ");
      Serial.print(i+1);
      Serial.print(": ");
      Serial.println(leituras[i]);
   };
   Serial.println("");
   Serial.println(" ------------------------------------- fim da transmissão -----------------------------------");
   delay(1000);
  // -------------------------- 55555555555555555555555555555end printar mensagem enviada ---------------------------
}
