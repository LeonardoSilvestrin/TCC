#include <sensor_data.h>

// ----- set and get sensor id -------------
void sensor_data::set_id(int id)
{
  this->id = id;
}
int sensor_data::get_id()
{
  return this->id;
}
//------------------------------------------
// ----- set and get ultima mensagem -------
void sensor_data::set_ultima_mensagem(float* mensagem)
{
  for (int i=0;i<tamanho_mensagem_in/sizeof(float);i++)
  {
    this->ultima_mensagem_recebida_sensor[i] = mensagem[i];
  }
}
float* sensor_data::get_ultima_mensagem()
{
  return this->ultima_mensagem_recebida_sensor;
}
//------------------------------------------
// ----- set and get endereco --------------
void sensor_data::set_endereco(uint64_t endereco)
{
  this->endereco_sensor = endereco;
}
uint64_t sensor_data::get_endereco()
{
  return this->endereco_sensor;
}
