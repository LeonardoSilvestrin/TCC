  #include <SPI.h>
  #include <RF24.h>
   
  RF24 radio(D3, D4); // CE, CSN
  const byte address[6] = "00001"; // Endereço de envio
   
  void setup() {
    Serial.begin(9600);
    radio.begin();
    radio.setPALevel(RF24_PA_MIN);
    radio.setDataRate(RF24_250KBPS);
    radio.setChannel(120);
    radio.openReadingPipe(0, address);
    radio.startListening(); // Configura o módulo como receptor
  }
   
  void loop() {
    if (radio.available()) { // Verifica se há mensagem recebida
      int message; // Mensagem recebida
      radio.read(&message, sizeof(message)); // Lê a mensagem
      Serial.print("Mensagem recebida: ");
      Serial.println(message);
   
      int ack = message; // Envia uma mensagem de resposta igual à mensagem recebida
      radio.stopListening(); // Configura o módulo como transmissor
      radio.write(&ack, sizeof(ack)); // Envia a resposta
      radio.startListening(); // Configura o módulo como receptor novamente
    }
  }
