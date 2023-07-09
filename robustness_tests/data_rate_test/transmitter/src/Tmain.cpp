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
RF24 network(radio);
RF24 mesh(network); 

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  radio.begin();
  radio.setPALevel(RF24_PA_MIN); //potencia mínima, desativa a amplificação de ruído
  radio.setChannel(125); //seta o canal 125 para operação => 2,525 GHz (2,4 + 0,125)
  const uint64_t address = 1; // Replace with your desired address
  radio.openWritingPipe(address); 
  radio.stopListening(); // Stop listening to send data
}

unsigned int delta_t = 20; // start frequency test = 1k uS = 1ms
// tests counter for different values of delta_t
int tests_counter = 0;

// redundant tests for the same value of delta_t
const int num_of_redundant_tests = 10;
int redundant_test_counter = 0;

// number of messages sent per redundant test
const unsigned long num_of_messages_per_test = 500;
unsigned long message_counter = 0;



void loop() 
{
  while(delta_t>=0)
  {
    redundant_test_counter = 0;
    Serial.print("Test number: ");
    Serial.println(tests_counter+1);
    Serial.print(", delay: ");
    Serial.print(delta_t);
    Serial.println(" microseconds");
    while(redundant_test_counter < num_of_redundant_tests)
    {
      message_counter = 0;
      unsigned long t0 = millis();
      while(message_counter<num_of_messages_per_test)
      {
        int message = redundant_test_counter;
        radio.write(&message, sizeof(message));
        Serial.print(message);
        Serial.print(",");
        message_counter++;
        delayMicroseconds(delta_t);
      }
      redundant_test_counter++;
      Serial.println();    
    }
    delta_t-= 1;
    tests_counter++;
  }
  while(1){}
}