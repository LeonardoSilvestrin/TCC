
Na pasta NRF24L01:

%------------------------------------------------------------------------------------------------------------------------
    meu_rx_nodemcu -- código para o receptor esp8266, não está funcionando
    meu_tx_nodemcu -- código para o transmissor arduino UNO, não está funcionando

        Problema: a comunicação via NRF24L01 fica errada quando eu tento concatentar string com inteiro, não consegui arrumar o erro.
        Tentei criar uma struct para corrigir mas o erro persistiu, ele tem dificuldades para separar o pacote do lado do receptor.

%------------------------------------------------------------------------------------------------------------------------
%------------------------------------------------------------------------------------------------------------------------
    rx_pingpong -- código para o receptor esp8266
    tx_pingpong -- código para o transmissor arduino UNO

        Nova implementação: dessa vez, vou tentar fazer uma comunicação ping pong, onde o transmissor envia uma string, espera resposta para em seguida mandar o inteiro separadamente.

%------------------------------------------------------------------------------------------------------------------------