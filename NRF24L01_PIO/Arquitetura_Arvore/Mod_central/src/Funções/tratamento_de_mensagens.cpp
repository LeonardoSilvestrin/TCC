#include <master.h>

// ==================================================FUNÇÕES GLOBAIS===========================================================
// --------------------------------------------funções de receber e enviar mensagens---------------------------------------
void enviar_mensagem(float* mensagem, int node_tag) 
{
  RF24NetworkHeader header(node_tag);
  bool ok = network.write(header, &mensagem, sizeof(mensagem));
  if (ok) {
    Serial.print("Mensagem enviada para nó ");
    Serial.println(node_tag);
  } else {
    Serial.print("Erro ao enviar mensagem para nó ");
    Serial.println(node_tag);
  }
}
bool receber_mensagem(float* mensagem, int node_tag) {
  if (network.available()) 
  {
    RF24NetworkHeader header(node_tag);
    network.peek(header);
    uint64_t origem = header.from_node(); 
    network.read(header, mensagem, sizeof(mensagem)); 
    return true;   
  }
  else
  {
    return false;
  }
}
// -------------------------------------------------------------------------
// -------------------------------------------- funções de serial print ---------------------------------------

void printar_mensagem_enviada(float* mensagem)
{
  int i;
  Serial.println("---------");
  Serial.println("Mensagem Enviada:");
  for (i = 0; i<tamanho_mensagem_out/sizeof(float); i++)
  {
    Serial.println(mensagem[i]);
  }
}

void printar_mensagem_recebida(float* mensagem)
{
    Serial.println("---------");
    Serial.println("Mensagem Recebida:");
    int i;
    for (i = 0; i<tamanho_mensagem_in/sizeof(float); i++)
    {
      Serial.println(mensagem[i]);
    }
}

// --------------------------------------------------------------------------------------------------------------------------
void filtrar_mensagem(float* mensagem)
{
  bool id_sensor_valido = 0;
  bool id_mensagem_plausivel = 0;
  bool leitura_plausivel = 0;
  for(int i=0;i<num_sensores;i++)
  {
    if ((int)mensagem[0]==i+1) //checa se o id associado à mensagem recebida está na lista de sensores (1,2) no caso
    {
      id_sensor_valido = 1;
    }
  }
  if(mensagem[1] == std::floor(mensagem[1]) && mensagem[1] > 0) // checa se o id da mensagem é um inteiro positivo
  {
    id_mensagem_plausivel = 1;
  }
  if(mensagem[2]>=0 && mensagem[2]<=100) // checa se a leitura está entre 0 e 100, sendo uma leitura plausível
  {
    leitura_plausivel = 1;
  }
  if (id_sensor_valido && id_mensagem_plausivel && leitura_plausivel )
  {
    for (int i = 0;i<tamanho_mensagem_in/sizeof(float);i++)
    {
      mensagem_filtrada[i] = mensagem[i];
    }
  }
}

bool mensagem_repetida(sensor_data sensor,int id, int mensagem_id)
{
  if(sensor.get_id() == id && (int)sensor.get_ultima_mensagem()[1] == mensagem_id)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

// =============================================================================================================================
