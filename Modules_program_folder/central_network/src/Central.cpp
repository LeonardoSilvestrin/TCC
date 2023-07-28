//Include Libraries
#include <Arduino.h>

#include <EEPROM.h>

#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>
#include <RF24Mesh.h>

#if defined(ARDUINO_ARCH_ESP8266)   // configuração para ESP8266
  #define CE 0
  #define CSN 2
#elif defined(ARDUINO_ARCH_AVR) // configuração para UNO
  #define CE 9
  #define CSN 10
#endif

#define uC_serial  9600 // velocidade de comunicação do micro via porta serial
#define baseID 255


RF24                          radio(CE, CSN); // CE, CSN
RF24Network                   network(radio);
RF24Mesh                      mesh(radio, network);


int new_sensor_id =           baseID;
int denial_id =               0;
int sensor_id = 1;

// ------------------------------------esses dois comandos devem vir do servidor --------------------------
bool entrada_novo_modulo =  true;
bool saida_modulo =         false;
int modulo_inativo =        0;
//-----------------------------------------Network_configuration minha_rede;----------------------------------------------------------------
bool server_online = false;

//==========================================================================================================================================
const int TAMANHO_FILA = 254;
struct DadoSensor 
{
  float id;
  float bateria;
  float temperatura;
  float umidade_do_solo;
};
class Received_data
{  private:
    DadoSensor fila[TAMANHO_FILA];
    int inicioFila;
    int fimFila;

  public:
    Received_data() : inicioFila(0), fimFila(0) {}

    bool adicionarNaFila(float v1, float v2, float v3, float v4) 
    {
      int proximoFim = fimFila + 1; // % TAMANHO_FILA;  // Próximo índice no final da fila

      if (proximoFim == inicioFila) 
      {
        // Fila cheia
        return false;
      }

      fila[fimFila].id = v1;
      fila[fimFila].bateria = v2;
      fila[fimFila].temperatura = v3;
      fila[fimFila].umidade_do_solo = v4;

      fimFila = proximoFim;
      return true;
    }

    bool removerDaFila() 
    {
      if (inicioFila == fimFila) 
      {
        // Fila vazia
        return false;
      }
      fila[inicioFila].id                = 0;
      fila[inicioFila].bateria           = 0;
      fila[inicioFila].temperatura       = 0;
      fila[inicioFila].umidade_do_solo   = 0;

      inicioFila ++;
      return true;
    }
    // aqui tem bug
    bool enviarDadosArmazenados() 
    {
      while (inicioFila != fimFila) 
      {
        if (enviarDadoParaServidor(fila[inicioFila])) 
        {
          removerDaFila();
        } 
        else 
        {
          // Se não conseguir enviar o dado, saia do loop
          return false;
        }
      }
      return true;
    }


    // Simulação do envio de dados para o servidor (substitua pelo código de envio real)
    bool enviarDadoParaServidor(DadoSensor& dado) 
    {
      if(server_online)
      {
        Serial.println("Dados Enviados para o servidor: ");
        Serial.print("ID: ");
        Serial.println(dado.id);
        Serial.print("Bateria: ");
        Serial.println(dado.bateria);
        Serial.print("Temperatura: ");
        Serial.println(dado.temperatura);
        Serial.print("Umidade do solo: ");
        Serial.println(dado.umidade_do_solo);
        Serial.println("");
      }
      // Código para enviar o dado para o servidor aqui (substitua por sua implementação real)
      // Retorne true se o envio for bem-sucedido, ou false caso contrário
      // Por simplicidade, estamos retornando true aqui
      return true;
    }
  };

Received_data dados_na_fila;
// ==========================================Funções de interpretação e tratamento de mensagens============================================

void listen_to_server()
{
  if (Serial.available() > 0) 
  {
    char data = Serial.read();
    bool* server_online_ptr = &server_online;
    
    if (data == '0')
    {
      *server_online_ptr = false;
      Serial.println("Server set to OFF");
    }
    if (data == '1')
    {
      *server_online_ptr = true;
      Serial.println("Server set to ON");
    }
  }
}

bool is_id_valid(int id)
{
  if(!(id>=1&&id<=255)) // se o id solicitando não estiver entre 1 e 255 é um id inválido
  {
    return 0;
  }
  else
  {
    return 1;
  }
}

bool is_header_valid(int header) 
{
  if ((char)header == 'c' || (char)header == 'd') 
  {
    return true;
  } 
  else 
  {
    return false;
  }
}
// ========================================================= CONFIG ('c') MESSAGE ================================================================
// responde solicitação do tipo 'c' com o novo endereço do módulo (id), retorna 1 se receber o ACK e 0 se não receber.
bool answer_id_request_and_listen_to_ACK(int id, int message_to)
{
  if(mesh.write(&id,'c',sizeof(id),message_to))
  {
    //================== print ========================
    if(!saida_modulo)
    {
      Serial.print("Módulo");
      Serial.print(" inserido na rede, ID: ");
      Serial.println(id);
      return 1;
    }
    else
    {
      Serial.print("Módulo ");
      Serial.print(" retirado na rede, ID: ");
      Serial.println(id);
      return 1;
    }
  }
  else // se o write nao teve sucesso a central não deve computar mudanças na rede
  {
    Serial.println("Sem resposta do Módulo solicitante");
    return 0;
  }
}

// processa mensagem do tipo 'c' -> novo sensor solicitando entrada na rede.
// retorna 1 se o processo for concluído (sensor solicitante recebeu id e está computado na rede) e 0 caso contrário

bool process_d_message(RF24NetworkHeader header)
{
  int id_sensor = mesh.getNodeID(header.from_node);
  float data_to_receive[3];
  if(id_sensor >0 && id_sensor < 255)
  {
    if (network.available()) 
    {
      network.read(header, data_to_receive, sizeof(data_to_receive));
      Serial.println("Dados Recebidos");
      dados_na_fila.adicionarNaFila(id_sensor, data_to_receive[0],data_to_receive[1],data_to_receive[2]);
      return 1;
    }
  }
  else
  {
    network.read(header,0,0);
    {
      Serial.print("Mensagem de dados vinda de header inválido, ID: ");
      Serial.println(id_sensor);
    }
  }
  return 0;
}

// ===============================================================================================================================================
bool listen_to_network()
{
  if (network.available()) 
  {
    RF24NetworkHeader header;
    network.peek(header); //ler o header da próxima mensagem da fila
    if(is_id_valid(mesh.getNodeID(header.from_node)))
    {
      switch (header.type) 
      {
        case 'd':
          // process_d_message(header, module_list);
          process_d_message(header);
          return 1;
        default:
          network.read(header, 0, 0);
          Serial.println("Mensagem de tipo desconhecido");
          Serial.print("header: ");
          Serial.println((char)header.type);
          break;
      }
      return 0;
    }
  }
  return 0;
}


void setup() 
{
  Serial.begin(uC_serial);
  Serial.flush();
  Serial.println("\n########################################################################");
  mesh.setNodeID(0); // 0 -> central
  Serial.println(mesh.getNodeID());
  radio.setRetries(5, 10);
  radio.begin();
  radio.setPALevel(RF24_PA_MIN);
  radio.setChannel(125);

  EEPROM.begin(4000);

  // Connect to the mesh
  if (!mesh.begin(125,RF24_250KBPS)) 
  {
    // if mesh.begin() returns false for a master node, then radio.begin() returned false.
    Serial.println(F("Hardware offline."));
  }

}
//tempo máximo que a central fica no loop principal antes de reinicar tudo

void loop()
{
  mesh.update();
  mesh.DHCP();
  listen_to_network();
  if (server_online)
  {
    dados_na_fila.enviarDadosArmazenados();
  }
  listen_to_server();
  delay(1);
}