#include <master.h>

sensor_data sensores[2];

void setup_sensores()
{
    for (int i=0;i<num_sensores;i++)
    {
        sensores[i].set_id(i+1);
        sensores[i].set_endereco(enderecos[i+1]);
        sensores[i].set_ultima_mensagem(mensagem_recebida);
    }
}