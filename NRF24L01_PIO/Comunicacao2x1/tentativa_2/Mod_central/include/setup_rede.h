#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>


#ifndef setup_rede_H
#define setup_rede_H

#ifdef ARDUINO_ARCH_ESP8266 // muda a configuração dos pinos dependendo da placa que estou usando
// CE, CSN esp8266
#define CE 0
#define CSN 2
// CE, CSN arduino
#else
#define CE 9
#define CSN 10
#endif 
#define uC_serial 9600


extern const uint64_t enderecos[];
extern const int num_sensores;

extern float mensagem_enviada[2];
extern float mensagem_recebida[3];
extern float mensagem_filtrada[3];

extern int tamanho_mensagem_out;
extern int tamanho_mensagem_in;
extern RF24 radio;

#endif
