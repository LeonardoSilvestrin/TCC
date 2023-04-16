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
    #define endereco 0xF0F0F0F0AA
  #elif defined(ARDUINO_AVR_NANO)
    // configuração para Arduino NANO
    #define endereco 0xF0F0F0F066
  #endif
#endif
#define uC_serial  9600


//create an RF24 object/dev/ttyUSB0
RF24 radio(CE, CSN);

const uint64_t address = endereco;
int contador = 0;
float leituras[4];
float sensorID(const uint64_t endereco_sensor){ 
  switch (endereco_sensor) {
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
  radio.openWritingPipe(address);
  radio.stopListening();
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  Serial.print("Começando transmissão - ");
  Serial.println(radio.isChipConnected());
}

void enviar_mensagem(float* mensagem){
  long int t0 = millis();
  while (millis()-t0 < 5000){
    byte buffer_mensagem[sizeof(mensagem)];
    memcpy(buffer_mensagem, &mensagem, sizeof(mensagem));
    radio.write(&buffer_mensagem, sizeof(buffer_mensagem));
  }
}

void loop()
{
  contador++;
  leituras[0]= sensorID(endereco);
  leituras[1]= contador;
  leituras[2]= 0;
  leituras[3]= 0;
  enviar_mensagem(leituras);
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
  // -------------------------- end printar mensagem enviada ---------------------------
  delay(1000);
}
