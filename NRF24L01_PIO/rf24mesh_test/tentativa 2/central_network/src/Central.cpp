//Include Libraries
#include <Arduino.h>
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

// ------------------------------------esses dois comandos devem vir do servidor --------------------------
  bool entrada_novo_modulo =  true;
  bool saida_modulo =         false;
//---------------------------------------------------------------------------------------------------------

class network_configuration
{
  private:
    int num_of_nodes = 1;
    bool addresses_in_use[256];
    uint8_t unique_hardware_id_list[256*8-2*8]; // -2x9 por que não precisa salvar o ID único da central nem do endereço de entrada 255
    bool module_types[256];

  public:

    //construtor da classe
    network_configuration()
    {
      this->addresses_in_use[0]=true;
      this->addresses_in_use[255]=true;
      for(int i = 1;i<255;i++)
      {
        this-> addresses_in_use[i] = false;
      }
    }
    
    void recover_network_config(int num_of_nodes, bool* addresses_in_use, uint32_t* unique_hardware_id_list, bool* module_types)
    {  
      this->num_of_nodes = num_of_nodes;
      for (int i = 0; i < 256; i++) 
      {
        this->addresses_in_use[i] = addresses_in_use[i];
        this->module_types[i] = module_types[i];
        for(int j=0;j<8;j++)
        {
          this->unique_hardware_id_list[i+j] = unique_hardware_id_list[i+j];
        }
      }
    }
  
    // adiciona entidade na rede
    void add_entity(int id, char sensor_type, uint8_t* unique_hardware_id)
    {
      if (num_of_nodes == 254)
      {
        Serial.println("Erro: Número máximo de módulos atingido.");
        while (1){}
      }
      if (sensor_type != 's' && sensor_type != 'i')
      {
       Serial.println("Erro: Módulo de tipo desconhecido.");
       Serial.print("Tipo recebido: ");
       Serial.println(sensor_type);
       while(1){}   
      }
      if (sensor_type == 's')
      {
        this->module_types[id] = 0;
      }
      if (sensor_type == 'i')
      {

        this->module_types[id] = 1;
      }
      this->num_of_nodes++;
      this->addresses_in_use[id]=true;
      Serial.print("unique hardware id computado: ");
      for(int i=0;i<8;i++)
      {
        this->unique_hardware_id_list[8*(id-1)+i] = unique_hardware_id[i];
        Serial.print(unique_hardware_id_list[8*(id-1)+i],HEX);
        Serial.print(" ");
      }
      Serial.println("");
    }
    
    // remove entidade da rede
    void remove_entity(int id)
    {
      if (num_of_nodes == 1)
      {
       Serial.println("Erro: não há módulos para remover");
       while(1){} 
      }
      this->num_of_nodes--;
      this->addresses_in_use[id]=false;
      for(int i=0;i<8;i++)
      {
        this->unique_hardware_id_list[id+i] = (uint8_t)0;
      }
    }
    
    //ponteiro para o vetor que guarda o stattus da rede
    bool* addresses()

    {
      return addresses_in_use;
    }
    
    // printa os endereços disponíveis e utilizados da rede (lista de booleanos)
    void print_mesh_stattus()
    {
      Serial.println("==========================MESH STATTUS============================");
      for(int i = 0;i<=255;i++)
      {
        Serial.print(addresses_in_use[i]);
      }
      Serial.println("\n=================================================================");
    }
    
    // gera um novo ID (não altera o stattus da rede)
    int generate_new_ID()
    {
      for(int i=1;i<255;i++)
      {
        if(!addresses_in_use[i])
        {
          return i;
        }
      }
      return 0;
    }
    
    //checa se o ID já está registrado na rede
    bool is_id_in_the_mesh(int id)
    {
      uint8_t number_of_ids_in_use = mesh.addrListTop;
      for(int i = 0;i<number_of_ids_in_use;i++)
      {
        if((uint8_t)id == mesh.addrList[i].nodeID)
        {
          return 1;
        }  
      }
      return 0;
    }
    
    uint8_t* get_module_recorded_unique_id(int id)
    {
      return &unique_hardware_id_list[8*(id-1)];
    }

    int get_type(int id)
    {
      return module_types[id];
    }

};


network_configuration minha_rede;

// ==========================================Funções de interpretação e tratamento de mensagens============================================

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

bool compare_hardware_id_to_record(int sensor_network_id, uint8_t* id_2)
{
  uint8_t* registered_uinique_address = minha_rede.get_module_recorded_unique_id(sensor_network_id);
  for (int i = 0; i<8;i++)
  {
    if(!(registered_uinique_address[i] == id_2[i]))
    {
      return 0;
    }
  }
  return 1;
}
// ========================================================= CONFIG ('c') MESSAGE ================================================================
bool answer_id_request_and_listen_to_ACK(int id, int message_to)
{
  if(mesh.write(&id,'c',sizeof(id),message_to))
  {
    //================== print ========================
    if(id != 255)
    {
      Serial.print("Módulo ");
      Serial.print(" inserido na rede, ID: ");
      Serial.println(id);
      return 1;
    }
    else
    {
      Serial.print("Módulo ");
      Serial.println(" degenerado solicitando reinserção na rede, resetando ID para 255... ");
      return 1;
    }
    if(saida_modulo)
    {
      Serial.print("Módulo ");
      Serial.print(" retirado na rede, ID: ");
      Serial.println(id);
      return 1;
    }
    return 1;
  }
  else // se o write nao teve sucesso a central não deve computar mudanças na rede
  {
    Serial.print("Sem resposta do Módulo solicitante");
    minha_rede.print_mesh_stattus();
    return 0;
  }
}

// processa mensagem do tipo 'c' -> novo sensor solicitando entrada na rede.
// retorna 1 se o processo for concluído (sensor solicitante recebeu id e está computado na rede) e 0 caso contrário

// conteudo da mensagem de configuracao: id do hardware

bool process_c_message(RF24NetworkHeader header)
{
  // --------------------------------------------------------recebimento da mensagem de configuração --------------------------------------------------
  uint8_t uniqueID_and_type[9];
  network.read(header, uniqueID_and_type, sizeof(uniqueID_and_type));
  int requester_network_id =    (int)mesh.getNodeID(header.from_node);
  uint8_t* requester_hardware_id = uniqueID_and_type;
  char sensor_type = (char)requester_hardware_id[8];
  Serial.println("Solicitação de configuração recebida.");
  Serial.print("ID Hardware: ");
  for(int i =0 ; i<8;i++)
  {
    Serial.print(requester_hardware_id[i],HEX);
    Serial.print(" ");
  }
  Serial.println("");
  Serial.print("ID Network: ");
  Serial.println(requester_network_id);
  Serial.print("Sensor sensor_type: ");
  Serial.println((char)requester_hardware_id[8]);
  
  minha_rede.print_mesh_stattus();
  // *********************************************************************************************************************************************************************
  
  // O tipo de resposta dado pela central vai depender das flags de entrada e saída de módulo que são recebidas do servidor, cada caso está implementado a seguir seguindo
  // o modelo do protocolo de entrada da rede.

  // *********************************************************************************************************************************************************************
  /*
  // =================================================< PRIMEIRO CASO: há solicitação de << entrada e saída >> simultâneas >============================================= 
  if (entrada_novo_modulo && saida_modulo)
  {
    Serial.println("========ATENÇÃO========\n comandos de entrada e saída ativados simultâneamente!\n por favor realize uma operação de cada vez. ");
    return 0;
  }
  */
  // =================================================< SEGUNDO CASO: Há solicitação de << entrada >> de módulo >========================================================
  if(entrada_novo_modulo)
  {
    // se o id for 255 e o contador 0, temos um novo sensor entrando na rede
    if(requester_network_id == 255) 
    {
      int new_sensor_id = minha_rede.generate_new_ID();
      Serial.println("Novo sensor solicitando entrada na rede, gerando ID.");
      if(answer_id_request_and_listen_to_ACK(new_sensor_id,requester_network_id))
      {
        minha_rede.add_entity(new_sensor_id,sensor_type,requester_hardware_id);
        // entrada_novo_modulo = false;
        return 1;
      }
    }

    Serial.print("Posicao inicial do endereço na memoria: ");
    Serial.println((int)minha_rede.get_module_recorded_unique_id(requester_network_id));
    Serial.print("Conteudo da posicao: ");
    Serial.println(*minha_rede.get_module_recorded_unique_id(requester_network_id),HEX);
    
    // se o id não for 255 e estiver registrado na rede, temos duas possibilidades:
    if(requester_network_id != 255 && minha_rede.is_id_in_the_mesh(requester_network_id))
    {
      // PRIMEIRA POSSIBILIDADE: contador recebido pela mensagem igual ao registrado na rede
      // -> Significa que um dos módulos foi reiniciado de maneira inesperada e está solicitando reentrada.
      // -> A flag de entrada_novo_modulo continua ativa (1). (talvez seja bom implementar algum aviso de possível erro nesta etapa)
      if (compare_hardware_id_to_record(requester_network_id,requester_hardware_id)) 
      {
        Serial.print("Atenção, o módulo de ID: ");
        Serial.print(requester_network_id);
        Serial.println(" foi reiniciado de maneira inesperada.\n Tentando reconexão...");
        answer_id_request_and_listen_to_ACK(requester_network_id,requester_network_id);
        return 1;
      }
      // SEGUNDA POSSIBILIDADE: o contador recebido não bate com o registrado
      // -> Significa que temos um módulo que foi reiniciado de forma incorreta tentando voltar à rede.
      // -> Para garantir que não haja erros de endereçamento, a central vai resetar o ID do sensor, e este irá pedir para reentrar na rede pelo endereço 255.
      // -> A flag de entrada_novo_modulo continua ativa (1).
      else
      {
        new_sensor_id = baseID;
        Serial.println("Sensor degenerado solicitando entrada na rede, por favor certifique-se de utilizar o protocolo de retirada corretamente quando for retirar um sensor");
        Serial.println("Resetando ID...");
        answer_id_request_and_listen_to_ACK(new_sensor_id,requester_network_id);
        return 1;
      }
    }
    else
    {
      new_sensor_id = requester_network_id;
      if(answer_id_request_and_listen_to_ACK(new_sensor_id,requester_network_id))
      {
        minha_rede.add_entity(new_sensor_id,sensor_type,requester_hardware_id);
        // entrada_novo_modulo = false;
        return 1;
      }
    }
  }
  
  /*
  // =================================================< TERCEIRO CASO: Há solicitação de << saída >> de módulo >========================================================
  if(saida_modulo)
  {
     if(requester_network_id != 255)
     {
      Serial.print("Módulo sendo retirado da rede, ID: ");
      Serial.println(requester_network_id);
      if(answer_id_request_and_listen_to_ACK(sensor_type, requester_network_id))
      {
        minha_rede.remove_entity(requester_network_id);
        // saida_modulo = false;
        return 1;
      }
     }
  }
  
  // =================================================< QUARTO CASO: Não há solicitações vindas do servidor >========================================================
  if(!(entrada_novo_modulo || saida_modulo))
    {
      if(requester_network_id != 255)
      {
      // se o contador recebido na mensagem de solicitação bater significa que temos uma reinicialização inesperada de módulo
      if (message_counter == minha_rede.get_hardware_unique_id(requester_network_id))
      {
        Serial.print("Atenção, o módulo de ID: ");
        Serial.print(requester_network_id);
        Serial.println("Foi reiniciado de maneira inesperada.\n Tentando reconexão...");
        answer_id_request_and_listen_to_ACK(sensor_type,requester_network_id);
        return 1;
      }
      else
      {
        Serial.println("Erro: Sensor degenerado solicitando entrada na rede, entrada negada.\nResetando EEPROM do módulo");
        answer_id_request_and_listen_to_ACK(sensor_type,new_sensor_id); 
        return 0;
      }
      }
    // se o contador não bater, é um módulo degenerado que foi desligado de maneira incorreta, a central retorna 0 caso não haja solicitação
    // de entrada de um novo módulo e 255 caso haja.
      else
      { 
        Serial.println("Solicitação de entrada recebida sem aprovação do servidor.\n Desativando módulo solicitante.");
        answer_id_request_and_listen_to_ACK(sensor_type,denial_id);
        return 1;
      }
    } 
  */
  Serial.println("NÃO DEVIA TER CHEGADO AQUI");
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
        // Display the incoming millis() values from the sensor nodes
        case 'c':
          process_c_message(header);
          break;
        default:
          network.read(header, 0, 0);
          Serial.println("Mensagem de tipo desconhecido");
          Serial.print("header: ");
          Serial.println((char)header.type);
          break;
      }
      return 1;
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
  radio.setRetries(5, 5);
  radio.begin();
  radio.setPALevel(RF24_PA_MIN);
  radio.setChannel(125);
  // Connect to the mesh
  if (!mesh.begin(125,RF24_250KBPS)) 
  {
    // if mesh.begin() returns false for a master node, then radio.begin() returned false.
    Serial.println(F("Hardware offline."));
  }
}
//tempo máximo que a central fica no loop principal antes de reinicar tudo
unsigned long t_max = 10000;
void loop() 
{
  // tempo decorrido desde o inicio do loop
  unsigned long tempo = millis();  
  int mensagens_recebidas=0;
  while(millis()-tempo<t_max)
  {
    mesh.update(); // Manter a malha atualizada
    mesh.DHCP(); // Essa funcao só é adicionada na central para garantir que ela enderece os periféricos corretamente
    mensagens_recebidas += listen_to_network();
    delay(1);
  }  
  if (mensagens_recebidas == 0)
  {
    Serial.print("Nenhuma mensagem recebida nos últimos ");
    Serial.print(t_max/1000);
    Serial.println(" segundos.");
  }
  
}