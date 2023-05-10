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

RF24 radio(CE, CSN); // CE, CSN
RF24Network network(radio);
RF24Mesh mesh(radio, network);

class network_configuration
{
  private:
    int num_of_nodes = 1;
    bool addresses_in_use[256]; //0 e 1 sempre em uso
  public:
    network_configuration()
    {
      this->addresses_in_use[0]=true;
      this->addresses_in_use[1]=true;
      for(int i = 2;i<=255;i++)
      {
        this-> addresses_in_use[i] = false;
      }
    }
    void add_entity(int id)
    {
      this->num_of_nodes++;
      this->addresses_in_use[id]=true;
    }
    void remove_entity(int id)
    {
      this->num_of_nodes--;
      this->addresses_in_use[id+1]=false;
    }
    bool* enderecos()
    {
      return addresses_in_use;
    }
};

class modulo
{
  private:
    int id;
    int endereco;
    char tipo;

  public:
    modulo(int id, int endereco, char tipo)
    {
      this->id = id;
      this->endereco = endereco;
      this->tipo = tipo;
    }
    void get_info()
    {
      Serial.println(id);
      Serial.println(endereco);
      Serial.println(tipo);
    }
    void set_id(int id)
    {
      this->id = id;
    }
    int get_id()
    {
      return id;
    }
      void set_endereco(int endereco)
    {
      this->endereco=endereco;
    }
    int get_endereco()
    {
      return endereco;
    }
      void set_tipo(char tipo)
    {
      this->tipo = tipo;
    }
    int get_tipo()
    {
      return tipo;
    }
};
network_configuration minha_rede;

int generate_new_ID()
{
  for(int i=2;i<256;i++)
  {
    if(!minha_rede.enderecos()[i])
    {
      return i;
    }
  }
  return 0;
}

bool handle_c_message(RF24NetworkHeader header) // recebe solicitação de id e retorna para o solicitante (id 1) o novo id dele. 
// retorna 1 caso o sensor receba o novo id e 0 caso não receba
{
  int id_solicitando_configuracao=0;
  network.read(header, &id_solicitando_configuracao, sizeof(id_solicitando_configuracao));
  Serial.println("Solicitação de ID recebida.");
  if(!(id_solicitando_configuracao>=1&&id_solicitando_configuracao<=255))
  {
    Serial.print("Solicitação de sensor inválida. ID Recebido:");
    Serial.println(id_solicitando_configuracao);
  }
  else
  {
    if(id_solicitando_configuracao==1)
    {
      Serial.println("Novo sensor solicitando entrada na rede");
    }
    else
    {
      Serial.print("ID antigo do sensor: ");
      Serial.println(id_solicitando_configuracao);
    }
  }
  int new_sensor_id = generate_new_ID();
  if(mesh.write(&new_sensor_id,'c',sizeof(new_sensor_id),1))
  {
    minha_rede.add_entity(new_sensor_id);
    Serial.print("Sensor novo computado na rede, ID: ");
    Serial.println(new_sensor_id);
    return 1;
  }
  else // se o write nao teve sucesso a central não deve computar o novo sensor na rede
  {
    Serial.print("Sem resposta do solicitante");
    return 0;
  }

}


bool escutar_rede()
{
  if (network.available()) 
  {
    RF24NetworkHeader header;
    network.peek(header); //ler o header da próxima mensagem da fila
    switch (header.type) 
    {
      // Display the incoming millis() values from the sensor nodes
      case 'c':
        handle_c_message(header);
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
  else
  {
    return 0;
  }
}

void setup() 
{
  Serial.begin(uC_serial);
  Serial.flush();
  Serial.println("\n########################################################################");
  // Set the nodeID to 0 for the master node
  mesh.setNodeID(0);
  Serial.println(mesh.getNodeID());
  // Set the PA Level to MIN and disable LNA for testing & power supply related issues
  radio.begin();
  radio.setPALevel(RF24_PA_MIN, 0);
  // Connect to the mesh
  if (!mesh.begin()) 
  {
    // if mesh.begin() returns false for a master node, then radio.begin() returned false.
    Serial.println(F("Hardware offline."));
  }
}
unsigned long t_max = 10000;
void loop() 
{
  // checar fluxo de dados vindos dos sensores
  unsigned long tempo = millis();  
  int mensagens_recebidas=0;
  while(millis()-tempo<t_max)
  {
    mesh.update(); // Manter a malha atualizada
    mesh.DHCP(); // Essa funcao só é adicionada na central para garantir que ela enderece os periféricos corretamente
    mensagens_recebidas += escutar_rede();
    delay(10);
  }  
  if (mensagens_recebidas == 0)
  {
    Serial.print("Nenhuma mensagem recebida nos últimos ");
    Serial.print(t_max/1000);
    Serial.println(" segundos.");
  }
}