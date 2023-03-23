  #include <SPI.h>
  #include <RF24.h>

 #define CE 9
 #define CSN 10
  RF24 radio(CE, CSN); // CE, CSN
  const byte address[6] = "00001"; // Endereço de envio
   
  void setup() {
    Serial.begin(9600);
    radio.begin();
    radio.setPALevel(RF24_PA_MIN);
    radio.setDataRate(RF24_250KBPS);
    radio.setChannel(120);
    radio.openWritingPipe(address);
  }
   
  void loop() {
    int message = 0; // Mensagem a ser enviada
    while (true) {
      radio.stopListening(); // Configura o módulo como transmissor
      radio.write(&message, sizeof(message)); // Envia a mensagem
      Serial.print("Mensagem enviada: ");
      Serial.println(message);
   
      unsigned long start_time = millis();
      bool ack_received = false;
      while (millis() - start_time < 5000) { // Espera por uma resposta por 5 segundos
        if (radio.available()) { // Verifica se há resposta
          int ack; // Mensagem de resposta
          radio.read(&ack, sizeof(ack)); // Lê a resposta
          Serial.print("Ack recebido: ");
          Serial.println(ack);
          ack_received = true;
          break;
        }
      }
      if (!ack_received) { // Se não receber resposta, tenta enviar novamente
        Serial.println("Ack não recebido");
        message++;
      } else { // Se receber resposta, incrementa a mensagem e envia novamente
        message++;
      }
      delay(1000); // Aguarda um segundo antes de enviar a próxima mensagem
    }
  }
