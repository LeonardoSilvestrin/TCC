#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>


#if defined(ARDUINO_ARCH_ESP8266)
  #define CE 0
  #define CSN 2
#elif defined(ARDUINO_ARCH_AVR)
  #define CE 9
  #define CSN 10
#endif 


RF24 radio(CE, CSN);


void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  radio.begin();
  radio.setPALevel(RF24_PA_MIN); //potencia mínima, desativa a amplificação de ruído
  radio.setRetries(5, 5); // seta o padrão de 5 retries no envio de mensagem com 1500us (5*250 + 250) de intervalo entre elas
  radio.setChannel(125); //seta o canal 125 para operação => 2,525 GHz (2,4 + 0,125) 

  const uint64_t address = 2; // Replace with your desired address
  radio.openWritingPipe(address); 
  radio.stopListening(); // Stop listening to send data
}

unsigned long delta_t = 1e2; // 10k microssegundos = 10 milissegundos
//const unsigned long T = (60ul*1000ul)*.02; // 5 minutos
const unsigned long T = 5000; // 5 segundos
int num_tests = 10;
int n = 1;

void loop() 
{
  while(n<=num_tests)
  {
    int message = n; // Message to be transmitted
    unsigned long t_loop = 0;
    unsigned long t_cycle_init = millis();
    while(t_loop < T + t_cycle_init)
    {
      t_loop = millis();
      radio.write(&message, sizeof(message));
      Serial.print(message);
      Serial.print(",");
      delayMicroseconds(delta_t);
    }
    Serial.println();
    //delta_t-=5;
    //if(delta_t == 0)
    //{
    //  Serial.print("End of test");
    //  while(1){}
    //}
    n++;
  }
  while(1){}
}