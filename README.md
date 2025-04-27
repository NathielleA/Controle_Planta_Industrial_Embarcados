# Controle_Planta_Industrial_Embarcados

### PROJETO

![img](https://github.com/NathielleA/Controle_Planta_Industrial_Embarcados/blob/main/imgs/ilstr_fabrica.jpg=100x200 "Ilustração fábrica")

O projeto consiste no sistema embarcado composto por um módulo de chão de fábrica que realiza o controle das serras para o corte da madeira, sensoriamento dos parametros de operação como temperatura e nivel do tanque de oléo e a segurança dos operadores. Além do módulo de chão de fábrica o sistema possui um módulo supervisor que se comunica com o chão de fábrica, obtendo informações da operação e controlando a produção podendo ajustar a velocidade das serras e a parada da produção.

Este projeto vai ser desenvolvido utilizando como dispositivo microcontrolador o Atmega328p disponivél no arduino nano. A progamação será feita em linguagem C com programação a nível de registradores para se obter a compreeensão da arquitetura de microcontroladores e desenvolvimento de aplicações microcontroladas.

Para simulações foi utilizado o simulador [wokwi](https://wokwi.com/) que disponibiliza o arduino Nano e um conjunto de sensores e outros recursos úteis ao projeto. Além das simulações o sistema foi implementado em bancada no laboratorio.

---

### REQUISITOS

Abaixo seguem os requisitos solicitados para o sistemas embarcado com a divisão entre os requisistos do módulo supervisor e módulo chão de fábrica.

###### MÓDULO SUPERVISOR

* Deverá ter um interruptor ligado ao pino de interrupção externa, que caso acionado, deverá encaminhar mensagem “Parada solicitada”
  para o Arduino 2 parar a produção, paralelamente escrever no Monitor Serial a mesma mensagem. Em seguida, o Arduino 2 deverá
  mandar mensagem de confirmação da parada da produção, e o Arduino 1 escrever no Monitor Serial a mensagem (“Parada realizada
  com sucesso!”);
* Implementar dois potenciômetros, onde cada um irá controlar a velocidade dos dois motores situados no chá de Fábrica;
* O Arduino 1 deverá comunicar com o Arduino 2 utilizando um dos protocolos já existente no Arduino, como por exemplo, UART, SPI,
  I2C etc.
* A cada 3 segundos, o Arduino 1 deverá escrever no Monitor Serial as seguintes informações encaminhadas pelo Arduino 2, que devem
  estar atualizadas:
  * Status do Sensor de Temperatura;
  * Status do Sensor de Inclinação;
  * Status do Sensor de Presença;
  * Status do Nível do Tanque de Óleo;
  * Status da Produção
  * Velocidade dos motores;
  * Quantidade de Blocos de madeiras cortados;

###### MÓDULO CHÃO DE FÁBRICA

* Deverá ter um interruptor ligado ao pino de interrupção externa, que caso acionado, deverá parar a produção, paralelamente escrever no
  Monitor Serial a mesma mensagem “Parada realizada com sucesso!”. Em seguida, o Arduino 2 deverá mandar mensagem para o Arduino
  1 da parada da produção, e o Arduino 1 escrever no Monitor Serial a mensagem (“Parada realizada com sucesso!”);
* Sensor de Temperatura, com faixa de Operação entre 10ºC e 40ºC, que caso seja acionado, deverá ligar o LED vermelho, acionar o
  Buzzer, parar a produção e encaminhar mensagem de Status do Sensor de Temperatura e Parada da Produção para o Arduino 1.
  Paralelamente, deverá escrever no monitor serial a mensagem “Temperatura Crítica!”;
* Deverá implementar sensor de inclinação, para verificar a orientação da madeira, caso esteja fora da inclinação correta, deverá acionar um
  servo motor, até que a posição esteja correta. Paralelamente deverá parar a produção e encaminhar a mensagem de aviso para o Arduino
  1 “Madeira fora do Eixo”, que deverá ser escrita no Monitor Serial;
* Deverá implementar dois motores CC/Servo, um para o corte vertical e outro o corte horizontal, ambos com as velocidades controladas
  pelos potenciômetros do Arduino 1.
* Deverá ter dois displays de 7 segmentos e/ou LCD mostrando a contagem dos blocos cortados, um mostrando a casa das dezenas e
  outra da unidade. Os blocos deverão ser cortados com os seguintes tamanhos: 10cm x 25cm. Considerar que a cada 100 rotações, o
  motor CC/servo consegue cortar 5cm de madeira;
* Os motores só estarão ligados caso o Sensor de Presença não detecte a presença humana em torno da esteira;
* Enquanto a produção estiver ocorrendo de forma prevista, o LED verde deverá ficar acesso, quando houver parada, deverá ser desligado;
* O sensor de nível deverá verificar periodicamente o nível do tanque de óleo utilizado para a finalização do bloco, alertando caso o tanque
  esteja no nível do limite superior ou em nível crítico baixo.
