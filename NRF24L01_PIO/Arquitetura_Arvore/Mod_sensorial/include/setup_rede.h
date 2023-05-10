#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>


#ifndef setup_rede_H
#define setup_rede_H

#if defined(ARDUINO_ARCH_ESP8266)
  // configuração para ESP8266
  #define CE 0
  #define CSN 2
#elif defined(ARDUINO_ARCH_AVR)
#define CE 9
#define CSN 10
#if defined(ARDUINO_AVR_UNO)
    // configuração para Arduino UNO
    #define tag 1
    #define tempo 0
  #elif defined(ARDUINO_AVR_NANO)
    // configuração para Arduino NANO
    #define tag 2
    #define tempo 1000
  #endif
#endif
#define uC_serial  9600

#define num_sensores 2
#define tamanho_mensagem_out 3
#define tamanho_mensagem_in 2

extern const uint64_t enderecos[];

extern float mensagem_enviada[tamanho_mensagem_out];
extern float mensagem_recebida[tamanho_mensagem_in];
extern float mensagem_filtrada[tamanho_mensagem_in];

extern RF24 radio;

#endif
