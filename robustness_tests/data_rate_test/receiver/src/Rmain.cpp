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

int* message_list = nullptr;  // Ponteiro para a lista
int list_size = 0;       // Tamanho atual da lista

void print_list_to_serial(int* list, int size) 
{
  for (int i = 0; i < size; i++) 
  {
    Serial.print(list[i]);
    if (i < size - 1) {
      Serial.print(",");
    }
  }
  Serial.println();  // Print a newline character at the end
}

void setup() 
{
  Serial.begin(9600);
  // put your setup code here, to run once:
  radio.begin();
  radio.setPALevel(RF24_PA_MIN); //potencia mínima, desativa a amplificação de ruído
  radio.setRetries(5, 5); // seta o padrão de 5 retries no envio de mensagem com 1500us (5*250 + 250) de intervalo entre elas
  radio.setChannel(125); //seta o canal 125 para operação => 2,525 GHz (2,4 + 0,125) 

  const uint64_t address = 1; // Replace with your desired address
  const uint64_t address2 = 2; // Replace with your desired address
  radio.openReadingPipe(1, address); 
  radio.openReadingPipe(2, address2); 
  radio.startListening();
  Serial.println("");
}

unsigned long receivedData;
unsigned long t_break = 10000;
unsigned long t_last_message = 0;
bool ja_printou = false;
void loop() 
{
  if(radio.available())
  {
    radio.read(&receivedData, sizeof(receivedData));
    int* temp_list = (int*)realloc(message_list, (list_size + 1) * sizeof(int));  // Reallocate memory for an additional element
    if (temp_list) 
    {
      message_list = temp_list;  // Assign the reallocated memory to the message_list pointer
      message_list[list_size] = receivedData;  // Add the received data to the list
      list_size++;
    }
    t_last_message = millis();
  }
  if (millis()-t_last_message > t_break && !ja_printou)
  {
    print_list_to_serial(message_list, list_size);
    ja_printou = true;
  }
  yield();
}
