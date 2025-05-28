# Controle_Planta_Industrial_Embarcados

### PROJETO

![img](https://github.com/NathielleA/Controle_Planta_Industrial_Embarcados/blob/main/imgs/ilstr_fabrica.jpg "Ilustração fábrica")

O projeto consiste no sistema embarcado composto por um módulo de chão de fábrica que realiza o controle das serras para o corte da madeira, sensoriamento dos parametros de operação como temperatura e nivel do tanque de oléo e a segurança dos operadores. Além do módulo de chão de fábrica o sistema possui um módulo supervisor que se comunica com o chão de fábrica, obtendo informações da operação e controlando a produção podendo ajustar a velocidade das serras e a parada da produção.

Este projeto vai ser desenvolvido utilizando como dispositivo microcontrolador o Atmega328p disponível no arduino nano. A progamação será feita em linguagem C com programação a nível de registradores para se obter a compreeensão da arquitetura de microcontroladores e desenvolvimento de aplicações microcontroladas.

Para simulações foi utilizado o simulador [wokwi](https://wokwi.com/) que disponibiliza o arduino Nano e um conjunto de sensores e outros recursos úteis ao projeto. Também foi utilizado para simulações o [Tinkercad](https://www.tinkercad.com/). Além das simulações o sistema foi implementado em bancada no laboratorio.

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

---

### ESPECIFICAÇÃO DO HARDAWARE

Foram utilizados dois arduinos nano para implementação do sistemas embaracdo. O arduino nano possui o microcontrolador Atmega328P um microcontrolador 8 bits com arquitetura AVR, na imagem abaixo tem-se a descrição dos pinos do arduino nano suas funções e os pinos do Atmega328P.


<img src="https://github.com/NathielleA/Controle_Planta_Industrial_Embarcados/blob/main/imgs/pinagem_nano.png" alt="Pinagem do arduino nano">

O ATmega328P possui varios recursos como GPIO programável, tratamento de interrupções, modulação PWM, temporizadores e contadores, conversor análogico digital entre outros que podem ser consultados no datasheet do [Atmega328P](https://github.com/NathielleA/Controle_Planta_Industrial_Embarcados/blob/main/recursos/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf).

A figura abaixo representa o hardware do módulo supervisor implementado no simulador wokwi, o supervisor possui como interfaces dois potenciomêtros para ajuste da velocidade das serras, botão de parada da produção. Uma interface com o monitor serial é utilizada para exibição de dados.


<img src="https://github.com/NathielleA/Controle_Planta_Industrial_Embarcados/blob/main/imgs/supervisor.png" alt="Pinagem do arduino nano">

A figura abaixo representa o hardware do módulo de chão de fábrica implementado no simulador wokwi, o chão de fábrica possui o sensores para observação da produção que são: sensor de temperatura, sensor de inclinação, sensor de presença, sensor de nível de oléo, os motroles para controle da serra de coret horizontal e vertical, motor do ajuste de inclinação e as interfaces para operadores: botão de parada da produção, leds de operação do sistema, buzzer para alarme e display para exibição de blocos cortados.

### ESPECIFICAÇÃO DO SOFTWARE

O software foi desenvolvido em linguagem C utilizando a biblioteca AVR para manipulação direta dos registradores do microcontrolador ATmega328P. Além disso, foi utilizada a biblioteca Wire.h para comunicação I2C.

1. Biblioteca AVR:
A biblioteca AVR é um conjunto de ferramentas e cabeçalhos fornecidos pela Atmel (agora parte da Microchip) para programar microcontroladores AVR, como o ATmega328P. Ela permite acesso direto aos registradores do microcontrolador, o que é essencial para configurações de baixo nível, como manipulação de portas digitais, temporizadores, interrupções e outros periféricos internos.

* Exemplo de uso: Configurar um pino como saída e alternar seu estado:

// filepath: /path/to/main.c
#include <avr/io.h>

int main(void) {
    DDRB |= (1 << PB0); // Configura o pino PB0 como saída
    while (1) {
        PORTB ^= (1 << PB0); // Alterna o estado do pino PB0
    }
}

2. Biblioteca Wire.h:
A biblioteca Wire.h é usada para comunicação I2C (Inter-Integrated Circuit), um protocolo que permite a comunicação entre dispositivos em um barramento compartilhado. No ATmega328P, ela abstrai a complexidade de configurar os registradores do hardware TWI (Two-Wire Interface), facilitando a comunicação com sensores, displays e outros dispositivos compatíveis com I2C.

* Exemplo de uso: Comunicação com um sensor I2C:

// filepath: /path/to/main.c
#include <Wire.h>

void setup() {
    Wire.begin(); // Inicializa o barramento I2C
    Wire.beginTransmission(0x68); // Endereço do dispositivo I2C
    Wire.write(0x00); // Envia um comando ou endereço de registro
    Wire.endTransmission();
}

void loop() {
    Wire.requestFrom(0x68, 2); // Solicita 2 bytes do dispositivo
    while (Wire.available()) {
        int data = Wire.read(); // Lê os dados recebidos
    }
}

O sistema é dividido em dois módulos principais: Supervisor e Chão de Fábrica, cada um com funcionalidades específicas, previamente descritas acima.


### ARQUITETURA DO SISTEMA

O sistema foi projetado com uma arquitetura modular, onde cada módulo possui responsabilidades bem definidas. A comunicação entre os módulos é feita exclusivamente via protocolo I2C, garantindo sincronização e troca de informações em tempo real.

#### Fluxo de Operação

* O supervisor ajusta as velocidades dos motores de corte enviando os valores lidos dos potenciômetros para o chão de fábrica.
* O chão de fábrica monitora continuamente os sensores e controla os motores de corte.
* Caso algum sensor detecte uma condição crítica (temperatura, inclinação ou presença), a produção é interrompida automaticamente.
* O supervisor solicita periodicamente os dados do chão de fábrica e exibe no monitor serial.
* O operador pode parar ou retomar a produção utilizando os botões de interrupção em ambos os módulos.

### DIAGRAMA DE BLOCO

O diagrama abaixo ilustra a interação entre os módulos Supervisor e Chão de Fábrica, bem como os sensores, atuadores e interfaces de comunicação:

<img src="https://github.com/NathielleA/Controle_Planta_Industrial_Embarcados/blob/main/imgs/dg_blc_1.png" alt="descrição de blocos de alto nivel do sistema">


(REVISAR ISSO TUDO A BAIXO E VER SE TEM REPETIÇÕES E AJUSTAR)

TESTES E SIMULAÇÕES
Para validar o funcionamento do sistema, foram realizados testes e simulações utilizando as plataformas Wokwi e Tinkercad. Essas ferramentas permitiram simular o comportamento dos sensores, motores e comunicação I2C, garantindo que o sistema atendesse aos requisitos especificados.

Além das simulações, o sistema foi implementado em bancada no laboratório, utilizando dois Arduinos Nano e os componentes descritos na especificação de hardware. Os testes em bancada confirmaram a funcionalidade do sistema em condições reais.

CONCLUSÃO
O projeto Controle Planta Industrial Embarcados demonstrou a viabilidade de implementar um sistema embarcado para controle de produção e segurança em um ambiente industrial. A utilização de programação a nível de registradores proporcionou maior controle sobre o hardware, enquanto a comunicação I2C garantiu a integração eficiente entre os módulos Supervisor e Chão de Fábrica.

O sistema atendeu a todos os requisitos especificados, sendo capaz de:

Monitorar e controlar a produção de forma segura.
Ajustar dinamicamente as velocidades dos motores.
Interromper a produção em condições críticas.
Exibir informações em tempo real no monitor serial e nos displays de 7 segmentos.
Este projeto pode ser expandido para incluir novos sensores, atuadores e funcionalidades, como integração com sistemas de supervisão remota ou armazenamento de dados em nuvem.