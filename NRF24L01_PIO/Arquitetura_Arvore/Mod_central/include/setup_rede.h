#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>

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

#define num_sensores 2
#define tamanho_mensagem_out 2
#define tamanho_mensagem_in 3

extern uint64_t enderecos[num_sensores];
extern const uint64_t endereco_central;
extern const uint64_t endereco_modulo_entrando_na_rede;
extern uint64_t endereco_do_novo_modulo;

extern float mensagem_enviada[tamanho_mensagem_out];
extern float mensagem_recebida[tamanho_mensagem_in];
extern float mensagem_filtrada[tamanho_mensagem_in];

extern RF24 radio;
extern RF24Network network;

void gerar_enderecos();

#endif
