/*
  mosquitto_sub -h test.mosquitto.org -t "TCC_horta_teste1"
*/

//Include Libraries
#include <Arduino.h>
#include <EEPROM.h>
#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>
#include <RF24Mesh.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <TaskScheduler.h>

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

// ---------------------------------------< variáveis globais controladas pelo servidor >------------------------------------------

char server_online[3] =     "00";
bool entrada_novo_modulo =  true;
bool saida_modulo =         false;
int modulo_inativo =        0;
bool prev_irr_status = 0;
bool* prev_irr_status_ptr = &prev_irr_status;
float umidade_do_solo = 0;
float *umidade_do_solo_ptr = &umidade_do_solo;
unsigned long ti = millis();

/* ==========================================< WIFI CONFIG >============================================
*/
const char *ssid = "Xaiomi 12 Lite"; // Enter your WiFi name
const char *password = "Hend3011";  // Enter WiFi password// MQTT Broker
const char *mqtt_broker = "test.mosquitto.org"; // Enter your WiFi or Ethernet IP
const char *topic = "TCC_horta_teste1";
const int mqtt_port = 1883;
WiFiClient espClient;
PubSubClient client(espClient);

void callback(char *topic, byte *payload, unsigned int length) 
{
 Serial.print("Message arrived in topic: ");
 Serial.println(topic);
 Serial.print("Message:");
 
 for (int i = 0; i < length; i++) {
  Serial.print((char) payload[i]);
 }
 
 Serial.println();
 Serial.println(" - - - - - - - - - - - -");
}


// ==========================================< Funções para comunicação com o servidor >============================================
String inputData = ""; // Used to store the received characters
void listen_to_server()
{
  
  if (Serial.available())
  {
    char c = Serial.read();
    
    if (c == '\r') // Check if Enter key is pressed
    {
      // Process the received data
      inputData.trim();
      if (inputData.length() == 2)
      {
        char data[2];
        data[0] = inputData[0];
        data[1] = inputData[1];

        char* server_online_ptr = &server_online[0];
        *server_online_ptr = data[0];
        *(server_online_ptr + 1) = data[1];

        if (data[0] == '0' && data[1] == '0')
        {
          *server_online_ptr = false;
          Serial.println("Server OFF");
        }
        else if (data[0] == '1' && data[1] == '0')
        {
          *server_online_ptr = false;
          Serial.println("Server OFF");
        }
        else if (data[0] == '0' && data[1] == '1')
        {
          Serial.println("Irrigação desligada");
        }
        else if (data[0] == '1' && data[1] == '1')
        {
          Serial.println("Irrigação ligada");
        }

        Serial.flush();
      }
      
      // Reset the input data for the next input
      inputData = "";
    }
    else
    {
      inputData += c;
    }
  }
}
bool reconnect() 
{
  unsigned long connection_timeout_timer = millis();
  Serial.println("Reconectando ao servidor MQTT");

  while (!client.connected() && (millis() - connection_timeout_timer < 10000))
  {
      String client_id = "esp8266-client-";
      client_id += String(WiFi.macAddress());
      Serial.printf("The client %s connects to mosquitto mqtt broker\n", client_id.c_str());
    if (client.connect(client_id.c_str())) 
    {
      
      Serial.println("");
      unsigned long tf = millis();
      Serial.println("Conectado");
      Serial.print("Tempo total de execução: ");
      Serial.print((tf-ti)/1000);
      Serial.println(" s");
      client.subscribe(topic);
    } 
    else 
    {
      Serial.print(".");
    }
    ESP.wdtFeed();
    yield();
    delay(500);
  }

  if (client.connected())
  {
     //connecting to a mqtt broker
    // publish and subscribe
    client.publish(topic, "Central Online");
    return 1;
  }
  else
  {
    Serial.print("failed, rc=");
    Serial.println(client.state());
    return 0;
  }
  delay(1);
}

int reconnect_all()
//retorna 0 -> tudo desconectado
//retorna 1-> wifi conectado;
//retorna 2-> wifi e servidor conectado; 
//retorna 3 ->wifi conectado, status do servidor desconhecido;
{
    if (WiFi.status() != WL_CONNECTED) 
    { 
      unsigned long startTime_wifi = millis();
      unsigned long Timeout_wifi = 10000; 
      Serial.print("Tentando reconectar ao WiFi");
      while (WiFi.status() != WL_CONNECTED && millis() - startTime_wifi < Timeout_wifi) 
      {
        delay(100);
        Serial.print(".");
        yield();
        ESP.wdtFeed();
      }
      if (WiFi.status() == WL_CONNECTED)
      {
        delay(1);
        Serial.println(""); 
        Serial.println("Wi-fi conectado com sucesso.");
        Serial.println("Estabelecendo conexão com o MQTT broker.");
        
        client.setServer(mqtt_broker, mqtt_port);
        client.setCallback(callback);
        unsigned long lastReconnectAttempt = millis();
        while(millis() - lastReconnectAttempt < 1000) 
        {
          if(reconnect()){return 2;}
          lastReconnectAttempt = millis();
        }

        unsigned long tf = millis();
        Serial.println("Falha em conectar, operando offline.");
        Serial.print("Tempo total de execução: ");
        Serial.print((tf-ti)/1000);
        Serial.println(" s");
        return 1;
      }
      else
      {
        unsigned long tf = millis();
        Serial.println("Falha em conectar, operando offline.");
        Serial.print("Tempo total de execução: ");
        Serial.print((tf-ti)/1000);
        Serial.println(" s");
        return 0;
      }
    }
    return 3;
  }
//---------------------------------------------< configurando fila de dados para sevidor >----------------------------------------------------------------------------------

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

      *umidade_do_solo_ptr = v4;
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
      if((int)dado.id +(int)dado.bateria+(int)dado.temperatura+(int)dado.umidade_do_solo== 0){ESP.restart();}
      if(server_online)
      {
        String data  = String((int)dado.id) + ","+ String((int)dado.bateria) + ","+ String((int)dado.temperatura) + "," + String((int)dado.umidade_do_solo);
        client.publish(topic,data.c_str());

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

//-----------------------------------------Network_configuration minha_rede;----------------------------------------------------------------

class Network_configuration
{
  private:
    bool network_changed = false;
    int num_of_modules = 1;
    bool ids_in_use[256];
    uint8_t unique_hardware_id_list[256*8-2*8]; // -2x8 por que não precisa salvar o ID único da central nem do endereço de entrada 255
    bool module_types[254];
    //==========================================================================================================================================
    // EEPROM Indexes to get data back from EEPROM after reset
    int EEPROM_index_num_of_modules = 0; // starts in eeprom[0]
    int EEPROM_index_ids_in_use = EEPROM_index_num_of_modules + sizeof(num_of_modules); // starts in eeprom [4]
    int EEPROM_index_unique_hardware_id_list = EEPROM_index_ids_in_use + sizeof(ids_in_use)/8; // starts in eeprom[36]
    int EEPROM_index_module_types = EEPROM_index_unique_hardware_id_list + sizeof(unique_hardware_id_list); //starts in eeprom[2332]

  public:

    //construtor da classe
    Network_configuration()
    {
      this->ids_in_use[0]=true;
      this->ids_in_use[255]=true;
      for(int i = 1;i<255;i++)
      {
        this-> ids_in_use[i] = false;
      }
    }
    
    void save_num_of_modules()
    {
      EEPROM.write(EEPROM_index_num_of_modules, (uint8_t)num_of_modules);    
      EEPROM.commit();
    }
 
    void recover_num_of_modules()
    {
      uint8_t _num_of_modules = 0;
      EEPROM.get(EEPROM_index_num_of_modules, _num_of_modules);
      this -> num_of_modules = (int)_num_of_modules;
    }
    
    /* ids in use com o print da eeprom para testes
    void save_ids_in_use_and_print()
    {
      uint8_t buffer_ids_in_use[sizeof(ids_in_use)/8];
      for (int i = 0; i < (int)(sizeof(ids_in_use)/8); i++) 
      {
        buffer_ids_in_use[i] = 0;
        Serial.print(i);
        Serial.print(" - ");
        for(int j = 0; j< 8; j++)
        {
          int ids_aux_index = 8*i+j;
          bool bit = ids_in_use[ids_aux_index];
          buffer_ids_in_use[i] |= (bit<<j);
          Serial.print(ids_in_use[ids_aux_index]);
        }
        Serial.print(" -- ");
        Serial.println(buffer_ids_in_use[i]);
      }
      //EEPROM.put(EEPROM_index_ids_in_use, buffer_ids_in_use);
    }
    */
    
    void save_ids_in_use()
    {
      uint8_t buffer_ids_in_use_to_eeprom[sizeof(ids_in_use)/8];
      for (int i = 0; i < (int)(sizeof(ids_in_use)/8); i++) 
      {
        buffer_ids_in_use_to_eeprom[i] = 0;
        for(int j = 0; j< 8; j++)
        {
          int ids_aux_index = 8*i+j;
          bool bit = ids_in_use[ids_aux_index];
          buffer_ids_in_use_to_eeprom[i] |= (bit<<j);
        }
      }
      EEPROM.put(EEPROM_index_ids_in_use, buffer_ids_in_use_to_eeprom);    
      EEPROM.commit();
    }
     
    void recover_ids_in_use()
    {
      uint8_t buffer_ids_in_use_from_eeprom[sizeof(ids_in_use)/8]; 
      EEPROM.get(EEPROM_index_ids_in_use,buffer_ids_in_use_from_eeprom);

      for (int i = 0; i < (int)(sizeof(ids_in_use)/8); i++) 
      {
        for (int j = 0; j<8;j++)
        {
          bool bit = buffer_ids_in_use_from_eeprom[i] & (1<<j);
          this->ids_in_use[8*i+j] = bit;
        }
      }
    }

    void save_module_types()
    {
      uint8_t buffer_module_types[sizeof(module_types)/8];
      for (int i = 0; i < (int)(sizeof(module_types)/8); i++) 
      {
        buffer_module_types[i] = 0;
        for(int j = 0; j< 8; j++)
        {
          int aux_index = 8*i+j;
          bool bit = module_types[aux_index];
          buffer_module_types[i] |= (bit<<j);
        }
      EEPROM.put(EEPROM_index_module_types, buffer_module_types);    
      EEPROM.commit();
      }
    }

    void recover_module_types()
    {
      uint8_t buffer_module_types[sizeof(module_types)/8];
      EEPROM.get(EEPROM_index_ids_in_use,buffer_module_types);
      for (int i = 0; i < (int)(sizeof(module_types)/8); i++) 
      {       
        for (int j = 0; j<8;j++)
        {
          bool bit = buffer_module_types[i] & (1<<j);
          this->module_types[8*i+j] = bit;
        }
      }
    }
    
    void save_hardware_unique_ids()
    {
      uint8_t buffer_unique_hardware_id_list[sizeof(unique_hardware_id_list)];
      memcpy(buffer_unique_hardware_id_list, unique_hardware_id_list,sizeof(unique_hardware_id_list));
      EEPROM.put(EEPROM_index_unique_hardware_id_list,buffer_unique_hardware_id_list);    
      EEPROM.commit();
      
    }
    
    
    /*recover_hardware_ids com o print da eeprom para testes
    */
   void recover_hardware_unique_ids_and_print()
    {
      uint8_t buffer_unique_hardware_id_list[sizeof(unique_hardware_id_list)];
      EEPROM.get(EEPROM_index_unique_hardware_id_list,buffer_unique_hardware_id_list);
      memcpy(unique_hardware_id_list,buffer_unique_hardware_id_list,sizeof(unique_hardware_id_list));
      Serial.print("Numero de sensores: ");
      Serial.println(num_of_modules);
      for(int i = 0; i<num_of_modules;i++) // max size = (int)sizeof(unique_hardware_id_list)/8
      {
        Serial.print(i+1);
        Serial.print(" - ");
        for(int j=0;j<8;j++)
        {
          Serial.print(unique_hardware_id_list[8*i+j],HEX);
          if(j<7){Serial.print("-");}
        }
        Serial.println("");
      }
    }/**/

    void recover_hardware_unique_ids()
    {
      uint8_t buffer_unique_hardware_id_list[sizeof(unique_hardware_id_list)];
      EEPROM.get(EEPROM_index_unique_hardware_id_list,buffer_unique_hardware_id_list);
      memcpy(unique_hardware_id_list,buffer_unique_hardware_id_list,sizeof(unique_hardware_id_list));
    }
    
    void save_network_config()
    {
      save_num_of_modules();
      save_ids_in_use();
      save_hardware_unique_ids();
      save_module_types();
      this-> network_changed = false;
    }
    
    void recover_network_config()
    {
      recover_num_of_modules();
      recover_ids_in_use();
      recover_hardware_unique_ids_and_print();
     // recover_hardware_unique_ids();
      recover_module_types();
      // a partir dos dados adicionar tudo à rede
      for(int i = 1; i<num_of_modules;i++) // max size = (int)sizeof(unique_hardware_id_list)/8
      {
        if (ids_in_use[i]==1)
        {
          if(module_types[i-1]==1)
          {
            add_entity(i,'i',&unique_hardware_id_list[8*(i-1)],0);
          }
          if(module_types[i-1]==0)
          {
            add_entity(i,'s',&unique_hardware_id_list[8*(i-1)],0);
          }
        }
      }
      Serial.println("");
    }
  
    // adiciona entidade na rede
    void add_entity(int id, char sensor_type, uint8_t* unique_hardware_id, bool new_sensor)
    {
      if (num_of_modules >= 254)
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
        this->module_types[id] = false;
      }
      if (sensor_type == 'i')
      {

        this->module_types[id] = true;
      }
      if(new_sensor){this->num_of_modules++;}
      this->ids_in_use[id]=true;
      Serial.print("Unique hardware id computado: ");
      for(int i=0;i<8;i++)
      {
        this->unique_hardware_id_list[8*(id-1)+i] = unique_hardware_id[i];
        Serial.print(unique_hardware_id_list[8*(id-1)+i],HEX);
        Serial.print(" ");
      }
      Serial.println("");
      if(new_sensor){this-> network_changed = true;}
    }
    
    // remove entidade da rede
    void remove_entity(int id)
    {
      if (num_of_modules == 1)
      {
       Serial.println("Erro: não há módulos para remover");
       while(1){} 
      }
      this->num_of_modules--;
      this->ids_in_use[id]=false;
      for(int i=0;i<8;i++)
      {
        this->unique_hardware_id_list[id+i] = (uint8_t)0;
      }
      this -> network_changed = true;
    }
    
    //ponteiro para o vetor que guarda o stattus da rede
    bool* get_ids()

    {
      return ids_in_use;
    }
    
    int get_type(int id)
    {
      return module_types[id];
    }
    int get_num_of_modules()
    {
      return num_of_modules;
    }

    // printa os endereços disponíveis e utilizados da rede (lista de booleanos)
    void print_mesh_stattus()
    {
      Serial.println("==========================MESH STATTUS============================");
      for(int i = 0;i<=255;i++)
      {
        Serial.print(ids_in_use[i]);
      }
      Serial.println("\n=================================================================");
    }
    
    // gera um novo ID (não altera o stattus da rede)
    int generate_new_ID()
    {
      for(int i=1;i<255;i++)
      {
        if(!ids_in_use[i])
        {
          return i;
        }
      }
      return 0;
    }
    
    //checa se o ID já está registrado na rede
    bool is_id_in_the_mesh(int id)
    {
      if(id == 255){return 1;}
      uint8_t number_of_ids_in_use = num_of_modules;
      for(int i = 0;i<number_of_ids_in_use;i++)
      {
        if(ids_in_use[id]==1)
        {
          return 1;
        }  
      }
      return 0;
    }
    
    uint8_t* get_module_recorded_unique_id(int id)
    {
      return &unique_hardware_id_list[8*(id)];
    }

    bool get_network_changed()
    {
      return this->network_changed;
    }
    void set_network_changed(bool did_it)
    {
      this->network_changed = did_it;
    }
};

Network_configuration minha_rede;

// ==========================================< Funções para interpretação e tratamento de mensagens >============================================

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
      return false;
    }
  }
  return true;
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


bool process_c_message(RF24NetworkHeader header)
// processa mensagem do tipo 'c' -> novo sensor solicitando entrada na rede.
// retorna 1 se o processo for concluído (sensor solicitante recebeu id e está computado na rede) e 0 caso contrário
// conteudo da mensagem de configuracao: id do hardware
{
  // --------------------------------------------------------recebimento da mensagem de configuração --------------------------------------------------
  // uniqueID_and_type guarda nas primeiras 8 posições o endereço único do hardware, o último byte é o tipo de sensor (uint8_t)'i' ou (uint8_t)'s'
  uint8_t uniqueID_and_type[9];

  network.read(header, uniqueID_and_type, sizeof(uniqueID_and_type));

  int requester_network_id =    (int)mesh.getNodeID(header.from_node);
  uint8_t* requester_hardware_id = uniqueID_and_type;
  char sensor_type = (char)requester_hardware_id[8];
  //--------------------------prints-------------------------
  Serial.println("Solicitação de configuração recebida.");
  Serial.print("Hardware ID: ");
  for(int i =0 ; i<8;i++)
  {network.read(header, uniqueID_and_type, sizeof(uniqueID_and_type));

    Serial.print(requester_hardware_id[i],HEX);
    Serial.print(" ");
  }
  Serial.println("");
  Serial.print("Network ID: ");
  Serial.println(requester_network_id);
  Serial.print("Sensor sensor_type: ");
  Serial.println((char)requester_hardware_id[8]);
  //minha_rede.print_mesh_stattus();
  //---------------------------------------------------------------
  // *********************************************************************************************************************************************************************
  
  // O tipo de resposta dado pela central vai depender das flags de entrada e saída de módulo que são recebidas do servidor, 
  //cada caso está implementado a seguir seguindo o modelo do protocolo de entrada da rede.

  // *********************************************************************************************************************************************************************
  
  // =================================================< PRIMEIRO CASO: há solicitação de << entrada e saída >> simultâneas >============================================= 
  if (entrada_novo_modulo && saida_modulo)
  {
    Serial.println("========ATENÇÃO========\n comandos de entrada e saída ativados simultâneamente!\n por favor realize uma operação de cada vez. ");
    return 0;
  }
  // =================================================< SEGUNDO CASO: Há solicitação de << entrada >> de módulo >========================================================
  if(entrada_novo_modulo)
  {
    // se o id for 255 e o contador 0, temos um novo sensor entrando na rede
    if(requester_network_id == 255) 
    {
      Serial.println("Novo sensor solicitando entrada na rede, gerando ID.");
      new_sensor_id = minha_rede.generate_new_ID();
      if(answer_id_request_and_listen_to_ACK(new_sensor_id,requester_network_id))
      {
        minha_rede.add_entity(new_sensor_id,sensor_type,requester_hardware_id,1);
        // entrada_novo_modulo = false;
        return 1;
      }
    }
    
    // se o id não for 255 e estiver registrado na rede, temos duas possibilidades:
    if(requester_network_id != 255 && minha_rede.is_id_in_the_mesh(requester_network_id))
    {
      // PRIMEIRA POSSIBILIDADE: hardware ID recebido pela mensagem igual ao registrado na rede
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
      // SEGUNDA POSSIBILIDADE: o hardware ID recebido não bate com o registrado
      // -> Significa que temos um módulo que foi reiniciado de forma incorreta tentando voltar à rede.
      // -> Para garantir que não haja erros de endereçamento, a central vai resetar o ID do sensor, e este irá pedir para reentrar na rede pelo endereço 255.
      // -> A flag de entrada_novo_modulo continua ativa (1).
      else
      {
        Serial.println("Sensor degenerado solicitando entrada na rede, por favor certifique-se de utilizar o protocolo de retirada corretamente quando for retirar um sensor");
        new_sensor_id = baseID;
        answer_id_request_and_listen_to_ACK(new_sensor_id,requester_network_id);
        return 1;
      }
    }
    // por fim, temos o caso onde o ID não é 255 e não está registrado na rede, nesse caso temos um módulo degenerado solicitando entrada mas ele pode manter
    // o mesmo ID
    if(requester_network_id != 255 && !minha_rede.is_id_in_the_mesh(requester_network_id))
    {
      Serial.println("Sensor degenerado solicitando entrada na rede, por favor certifique-se de utilizar o protocolo de retirada corretamente quando for retirar um sensor");
      Serial.print("Entrada permitida, ID: ");
      Serial.println(requester_network_id);

      new_sensor_id = requester_network_id;
      if(answer_id_request_and_listen_to_ACK(new_sensor_id,requester_network_id))
      {
        minha_rede.add_entity(new_sensor_id,sensor_type,requester_hardware_id,1);
        // entrada_novo_modulo = false;
        return 1;
      }
    }
  }
  
  // ===========  ======================================< TERCEIRO CASO: Há solicitação de << saída >> de módulo >========================================================
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
      if (compare_hardware_id_to_record(requester_network_id,requester_hardware_id))
      {
        new_sensor_id = requester_network_id;
        Serial.print("Atenção, o módulo de ID: ");
        Serial.print(requester_network_id);
        Serial.println("Foi reiniciado de maneira inesperada.\n Tentando reconexão...");
        answer_id_request_and_listen_to_ACK(new_sensor_id,requester_network_id);
        return 1;
      }
      else
      {
        new_sensor_id = 256;
        Serial.println("Erro: Sensor degenerado solicitando entrada na rede, entrada negada.\nResetando EEPROM do módulo");
        answer_id_request_and_listen_to_ACK(new_sensor_id,requester_network_id);
        return 0;
      }
      }
    // se o contador não bater, é um módulo degenerado que foi desligado de maneira incorreta, a central retorna 0 caso não haja solicitação
    // de entrada de um novo módulo e 255 caso haja.
      else
      { 
        new_sensor_id = denial_id;
        Serial.println("Solicitação de entrada recebida sem aprovação do servidor.\n Desativando módulo solicitante.");
        answer_id_request_and_listen_to_ACK(new_sensor_id,requester_network_id);
        return 1;
      }
    } 
  Serial.println("Sem resposta do módulo solicitante.");
  return 0;
}

bool process_d_message(RF24NetworkHeader header)
{
  int id_sensor = (int)mesh.getNodeID(header.from_node);
  float data_to_receive[3];
  /*
  Serial.print("id recebido: ");
  Serial.println(id_sensor);peek
  minha_rede.print_mesh_stattus();
  Serial.print("id na rede?");
  Serial.println(minha_rede.is_id_in_the_mesh(id_sensor));
  */
  
  if(id_sensor >0 && id_sensor < 255 && minha_rede.is_id_in_the_mesh(id_sensor))
  {
    if (network.available()) 
    {
      network.read(header, data_to_receive, sizeof(data_to_receive));
      Serial.println("Dados recebidos");
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

void message_discard(RF24NetworkHeader header)
{
    network.read(header,0,0);
    {
      int aux_msg = 0;
      int id_sensor = (int)mesh.getNodeID(header.from_node);
      mesh.write(&aux_msg,'a',id_sensor,sizeof(int));
      Serial.print("Mensagem de dados vinda de header inválido, ID: ");
      Serial.println(id_sensor);
      Serial.print("HEADER: ");
      Serial.println((char)header.type);
      
    }
}
// ===============================================================================================================================================
bool listen_to_network()
{
  if (network.available()) 
  {
    RF24NetworkHeader header;
    network.peek(header); //ler o header da próxima mensagem da fila
    if((is_id_valid(mesh.getNodeID(header.from_node))) && (minha_rede.is_id_in_the_mesh(mesh.getNodeID(header.from_node)) || header.type == 'c'))
    {
      switch((char)header.type) 
      {
        case 'c':
          process_c_message(header);
          return 0;
        case 'd':
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
    else
    {
      message_discard(header);
    }
  }
  return 0;
}
bool change_irrigation_status(int id_irrigador)
{
  bool status_to_send = true;
  if(mesh.write(&status_to_send,'d',sizeof(status_to_send),id_irrigador))
  {
    Serial.println(prev_irr_status);
    //================== print ========================
    if(prev_irr_status == 1)
    {
      Serial.print("Irrigação offline");
      *prev_irr_status_ptr = 0;
      return 1;
    }
    else
    {
      Serial.print("Irrigação online");
      *prev_irr_status_ptr = 1;
      return 1;
    }
  }
  else // se o write nao teve sucesso a central não deve computar mudanças na rede
  {
    Serial.println("Sem resposta do módulo de irrigação");
    return 0;
  }
}



void setup() 
{
  ti = millis();
  ESP.wdtDisable();
  EEPROM.begin(4000);
  
  Serial.begin(uC_serial);
  Serial.flush();
  Serial.println("\n########################################################################");
  Serial.println(mesh.getNodeID());
  
  radio.setRetries(5, 10);
  radio.begin();
  radio.setPALevel(RF24_PA_MIN);
  radio.setChannel(125);
  
 // minha_rede.recover_network_config();
  mesh.setNodeID(0); // ID 0 -> central
  // Conectando à malha
  if (!mesh.begin(125,RF24_250KBPS)) 
  {
    // se mesh.begin retornar falso para a central, significa que o rádio não está funcionando
    Serial.println(F("Hardware offline."));
  }

  // iniciando conexão Wi-Fi
  WiFi.begin(ssid, password);
  
  unsigned long startTime_wifi = millis();
  unsigned long Timeout_wifi = 10000;
  // Tenta conectar ao WiFi por até 10 segundos
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED && millis() - startTime_wifi < Timeout_wifi) 
  {
    delay(100);
    Serial.print(".");
    yield();
    ESP.wdtFeed();
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println(""); 
    Serial.println("Wi-fi conectado com sucesso.");
    Serial.println("Estabelecendo conexão com o MQTT broker.");
    
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    
    unsigned long startTime_server = millis();
    unsigned long Timeout_server = 5000;
    // Tenta conectar ao WiFi por até 10 segundos
    while (!client.connected() && millis() - startTime_server < Timeout_server) 
    {
      if(reconnect()){break;}
    }
  }
  else{Serial.println("Wi-Fi desconectado, iniciando modo offline.");}
  unsigned long tf = millis();
  Serial.print("Tempo total de execução: ");
  Serial.print((tf-ti)/1000);
  Serial.println(" s");
}


int irr_id = 1;
unsigned long tempo_ultima_tentativa = 0;
  
void loop() 
{ 
  mesh.update();
  mesh.DHCP();
  client.loop();
  unsigned long tempo_inicio_reconexao = millis();
  unsigned long intervalo = 60000;
  if(tempo_inicio_reconexao-tempo_ultima_tentativa>intervalo)
  {
    if (WiFi.status() != WL_CONNECTED || !client.connected()){reconnect_all();}
    tempo_ultima_tentativa = millis();
    delay(10);
  }
  listen_to_server();
  if (strcmp(server_online, "01") == 0 || strcmp(server_online, "11") == 0)
  {
    dados_na_fila.enviarDadosArmazenados();
    if (server_online[0] == '1' && prev_irr_status == 0)
    {
          change_irrigation_status(irr_id);
    }
    if (server_online[0] == '0' && prev_irr_status == 1)
    {
          change_irrigation_status(irr_id);
    }
  }



  listen_to_network();
/*
  if (umidade_do_solo < 50 && prev_irr_status == 0)
  {
      change_irrigation_status(irr_id);
  }

*/
  if (minha_rede.get_network_changed())
  {
    minha_rede.save_network_config();
    minha_rede.print_mesh_stattus();
    minha_rede.set_network_changed(0);
  }
  yield();
  ESP.wdtFeed();
  //ESP.deepSleep(5e6);
  //delay(t_cycle-tempo_final_ciclo);
}