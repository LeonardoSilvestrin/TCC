#include <setup_rede.h>


RF24 radio(CE,CSN);
RF24Network network(radio);

uint64_t enderecos[];
void gerar_enderecos()
{
    for(int i=0;i<num_sensores;i++)
    {
        enderecos[i] = (uint64_t)i;
    }
}

const uint64_t endereco_modulo_entrando_na_rede = enderecos[0];
const uint64_t endereco_central= enderecos[1];
uint64_t endereco_do_novo_modulo;


float mensagem_enviada[tamanho_mensagem_out];
float mensagem_recebida[tamanho_mensagem_in];
float mensagem_filtrada[tamanho_mensagem_in];
