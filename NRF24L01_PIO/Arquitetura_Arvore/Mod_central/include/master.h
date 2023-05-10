#include <sensor_data.h>
#ifndef master_H
#define master_H


// Funções definidas para lidar com mensagens

void enviar_mensagem(float* mensagem, uint64_t endereco);
int receber_mensagem(int pipe, uint64_t endereco);
void printar_mensagem_enviada(float* mensagem);
void printar_mensagem_recebida(float* mensagem);
void filtrar_mensagem(float* mensagem);
bool mensagem_repetida(sensor_data sensor, int id, int mensagem_id);

// tratamento dos módulos sensoriais

extern sensor_data sensores[num_sensores];
void setup_sensores();


#endif // FUNCOES_H