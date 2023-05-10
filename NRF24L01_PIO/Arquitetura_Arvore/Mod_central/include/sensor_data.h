#include <setup_rede.h>

#ifndef sensor_data_H
#define sensor_data_H

class sensor_data
{
    private:
        int id;
        float ultima_mensagem_recebida_sensor[3];
        uint64_t endereco_sensor;

    public:
        void set_id(int id);
        void set_ultima_mensagem(float* mensagem);
        void set_endereco(uint64_t endereco);
        int get_id();  // método adicionado
        float* get_ultima_mensagem();
        uint64_t get_endereco();  // método adicionado
};

#endif
