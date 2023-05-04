#include <setup_rede.h>


RF24 radio(CE,CSN);

const uint64_t enderecos[]= {0xF0F0F0F0F000,0xF0F0F0F0F001,0xF0F0F0F0F002};

float mensagem_enviada[tamanho_mensagem_out];
float mensagem_recebida[tamanho_mensagem_in];
float mensagem_filtrada[tamanho_mensagem_in];
