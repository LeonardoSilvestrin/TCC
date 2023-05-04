#include <setup_rede.h>


RF24 radio(CE,CSN);

const uint64_t enderecos[]= {0xF0F0F0F0F000,0xF0F0F0F0F001,0xF0F0F0F0F002};

float mensagem_enviada[2];
float mensagem_recebida[3];
float mensagem_filtrada[3];

int tamanho_mensagem_out = sizeof(mensagem_enviada);
int tamanho_mensagem_in = sizeof(mensagem_recebida);

const int num_sensores = 2;