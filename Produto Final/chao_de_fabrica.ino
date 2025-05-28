/*
| Programa Controle chão de fábrica
*/

#include <Wire.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define F_CPU  16000000UL // Define frequência da CPU para rotinas de atraso
#define TRUE 1
#define FALSE 0
#define RPS 33
// Define os dispositivos conetados aos pinos/ports

// saidas
#define LED_OP   PB0
#define Buzzer   PD3
#define Mt_vert  PD6
#define Mt_hor   PD5
#define Mt_Inc   PD3
#define DS       PC1 
#define SHCP     PC3  
#define STCP     PC2  
#define Triger   PB4
#define Dig_1    PB3
#define Dig_2    PB1 

// entradas
#define Sensor_PIR  PD7
#define Sensor_temp PC0
#define Sensor_Inc  PD4
#define Echo        PB2
#define Bt_parada   PD2

// Comunicação i2c
#define SDA PC4
#define SCL PC5
#define SLAVE_ADDRESS 0x08


#define NOTE_C  261
#define NOTE_D  294
#define NOTE_E  329
#define NOTE_F  349
#define NOTE_G  392
#define NOTE_A  440
#define NOTE_B  493
#define NOTE_CH 523  // Dó acima

// Tabela dos dígitos 0-9 para display de 7 segmentos ânodo comum
const uint8_t digitos[10] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F  // 9
};

//Definições de macros - empregadas para o trabalho com os bits 
#define set_bit(Y,bit_x) (Y|=(1<<bit_x)) /*ativa o bit x da  
variável Y (coloca em 1)*/ 
#define clr_bit(Y,bit_x) (Y&=~(1<<bit_x)) /*limpa o bit x da variável Y  
(coloca em 0)*/  
#define  tst_bit(Y,bit_x) (Y&(1<<bit_x))   /*testa o bit x da variável Y 
 (retorna 0 ou 1)*/ 
#define  cpl_bit(Y,bit_x) (Y^=(1<<bit_x)) /*troca o estado do bit x da  
variável Y (complementa)*/
#define pulse(port, bit)      do { set_bit(port, bit); _delay_us(1); clr_bit(port, bit); } while (0)

//==========================================
// cabeçalho de funções
void config_gpio();
void config_interrupcoes();
uint8_t prox_estado(uint8_t estado_atual);
uint16_t leitura_ultrasonico();
void leitura_sensores();
void print_dados();
void atualizar_producao();
void adc_init();
void shiftOut(uint8_t valor);
void config_pwm_timer0();
void set_pwmA(uint8_t duty_cycle);
void set_pwmB(uint8_t duty_cycle);
void timer1_temporizador_init();
void buzzer_init_timer2(uint16_t frequencia_hz);
void buzzer_off();
void play_note(uint16_t freq, uint16_t tempo_ms);
void alarme();
uint16_t adc_read(uint8_t channel);
void print_number(uint16_t value);
void uart_print(char* str);
void uart_transmit(char data);
void uart_init();

void onRequestHandler();
void wire_init_slave();
void onReceiveHandler(int numBytes);
void sendMessage();
void data_to_send();
void tratar_comando_pwm(const char* comando);

ISR(INT0_vect);
ISR(PCINT2_vect);
ISR(TIMER1_COMPA_vect);
//==========================================

//==========================================
// Variaveis globais
static volatile uint8_t status_producao=1;    // 0 = producao parada 1 = producao ativa
// desvio_inclinacao || presenca || temperatura_critca
static volatile uint8_t desvio_inclinacao=0;  // 0 = em ângulo    1 = fora do ângulo    
static volatile uint8_t presenca=0;           // 0 = sem presença 1= presença detectada
static volatile uint8_t temperatura_critca=0;  // 0 = norma 1= critica
static volatile uint8_t reativar_producao=0;  // 0 = limpa 1= troca de estado
static volatile uint8_t contador=0;
static volatile uint8_t estado_atual;   // variavel para controle de estados de operação do sistema
static volatile uint8_t comando_supervisor = FALSE;
static volatile uint8_t duty_cycle_A,duty_cycle_B; 
uint16_t distancia;
float temperatura;
float nivel_tanque;
uint8_t blocos;
uint8_t timer_zerado=FALSE; //0 = em espera 1 = fim da temporização
uint8_t producao_blocos =0;
uint8_t velocidade_MH,velocidade_MV; // variaveis para velocidade dos motores
uint8_t contador_RH=0,contador_RV=0;
uint8_t Dz_blocos=0,Un_blocos=0; // variavéis para controle da quantidade de blocos

char buffer[32];
String mensagemI2C;

//==========================================

/*
void setup(){

}

void loop(){
  principal();
}
*/
int main(){
  //uint8_t velocidade_MH,velocidade_MV; // variaveis para velocidade dos motores
  // inicializa variaveis

  estado_atual=0;
  status_producao=1; // começa com a produção parada
  velocidade_MH=1;
  velocidade_MV=1;

  config_gpio();  // faz a configuração dos pinos do GPIO
  config_interrupcoes();  // configura interrupções
  adc_init();      // Configura o conversor análogico digital
  uart_init();    // configura uart e a inicializa
  config_pwm_timer0();
  timer1_temporizador_init(); // configura e ativa o timer1
  wire_init_slave();    // configura o chão de fabrica como escravo i2c

  // teste de velocidade dos motores
  set_pwmA(velocidade_MH);
  set_pwmB(velocidade_MV);
  // laço de execução
  while(1){
    

      if(status_producao){
        set_pwmA(velocidade_MH);
        set_pwmB(velocidade_MV);
      }
      

      //print_number(estado_atual);
      switch(estado_atual){
        // No caso 0 aguarda a flag da temporização para mudança de estado
        case 0:
          if(timer_zerado && status_producao){
            // caso o timer seja zerado e não nenhuma flag de ocorrência esteja ativada
            // vai para o estado de leitura 
            estado_atual=1;
      
          }
          else if(desvio_inclinacao){
            // se a flag do sensor de inclinação estiver ativada
            // vai para o estado de correção da inclinação
            estado_atual= 3;
            buzzer_init_timer2(1000);
          }
          else if(presenca || (!status_producao)){
            // caso o sensor de presença, botão ou supervisor tenha parado a produção
            // vai para o estao de parada3
            set_bit(PORTB,LED_OP);
            estado_atual=2;
          }
          break;
        case 1:
          // No estado 1 faz a leitura dos sensores
          leitura_sensores();
          // reativa o timer
          timer1_temporizador_init();
          // verifica se a temperatura passou da faixa de segurança
          if(temperatura >=40 && temperatura <10){
            temperatura_critca = TRUE;
            status_producao = FALSE;
            buzzer_init_timer2(1000);
            //para a produção
            // vai para o estado de temperatura critica
            estado_atual=4;
          }
          // se tudo estiver ok vai para o estado de espera do timer
          estado_atual=0;
          break;
        case 2:
          // No estado 2 aguarda que as flags sejam limpas
          if( (!presenca) && reativar_producao){
            status_producao=TRUE;
            uart_print("Condicoes limpas\n");
            clr_bit(PORTB,LED_OP);
            //ativa motores
            estado_atual=0;
          }
          // verifica se o timer parou
          if(timer_zerado){
            // reativa timer
            timer1_temporizador_init();
          }
          // limpa flag temporaria para reativação da produção
          reativar_producao= FALSE;
          break;
        case 3:
            // no caso 3 aguarda a correção da angulação
            if(!desvio_inclinacao){
              //desativa alarme e desativa motor de ajuste
              buzzer_off();
              // se não houver presença libera ativa os motores e volta para espera
              if(!presenca){
                //ativa motores
                status_producao=TRUE;
                //ativa_led
                clr_bit(PORTB,LED_OP);
                estado_atual=0;
              }
              else{
                // caso alguma flag esteja setada vai para o estado 2
                estado_atual=2;
              }
              
            }
            // verifica se o timer parou
            if(timer_zerado){
              // reativa timer
              timer1_temporizador_init();
            }
          break;
        case 4:
          // No caso 4 aguarda a temperatura se estabilizar
          if(temperatura_critca >= 40){
              // Toca o alarme
          }
          else{
            // se não houver presença de funcioarios nas maquinas
            if(!presenca){
              // reativa os motores
              clr_bit(PORTB,LED_OP);
              status_producao = TRUE;
              estado_atual=0;
            }
            else{
              estado_atual=2;
            }
          }
          // verifica se o timer parou
          if(timer_zerado){
            // reativa timer
            timer1_temporizador_init();
          }
          break;
        default:
          break;
      }

      
      //====================================================
      // atualiza display
      clr_bit(PORTB,Dig_1);
      shiftOut(digitos[Un_blocos]);
      //_delay_ms(5);
      set_bit(PORTB,Dig_1);
      clr_bit(PORTB,Dig_2);
      shiftOut(digitos[Dz_blocos]);
      //_delay_ms(5);
      set_bit(PORTB,Dig_2);
      
      //====================================================
      
      // envia dados na uart após 1 segundos 
      if(contador == 5){
        if(status_producao)
            atualizar_producao();
        print_dados();
        data_to_send();
        contador=0;
      }

      // verifica solicitações de mensagens
      if(comando_supervisor){
        comando_supervisor=FALSE;

        if (strcmp(buffer, "parar_producao") == 0) {
           uart_print("Parada pelo supervisor\n");
           status_producao=0;
           set_pwmA(0);
           set_pwmB(0);
           set_bit(PORTB,LED_OP);
        }
        else if(strcmp(buffer, "voltar_producao") == 0) {
          if(!presenca && !desvio_inclinacao && !(estado_atual ==4)){
            uart_print("retomada pelo supervisor\n");
            reativar_producao=TRUE;
            set_pwmA(velocidade_MH);
            set_pwmB(velocidade_MV);
            set_bit(PORTB,LED_OP);
          }
           
        }
        else{
            tratar_comando_pwm(buffer); 
        }
      }
      
  }
  return 0;
}

/*
| Função config_gpio
| Configura os pinos da GPIO para uso
*/
void config_gpio(){
  // configura a direção dos pinos das ports
  DDRD = 0b01101011;
  DDRB = 0b00111011;
  DDRC = 0b11111110;

  clr_bit(MCUCR,4);
  // configurando pull-up nas entradas
  PORTD = 0x94;
  PORTB = 0xC4;
  PORTC = 0x31;

}
/*
| Função config_interrupcoes
| Configura as interrupções do gpio
*/

void config_interrupcoes(){
    // === INTERRUPÇÃO EXTERNA INT0 (D2/PD2) ===
    // EICRA – External Interrupt Control Register A
    // ISC01 = 1, ISC00 = 0 → Interrupção na borda de descida
    set_bit(EICRA, ISC01);
    clr_bit(EICRA, ISC00);

    // EIMSK – External Interrupt Mask Register
    // Habilita INT0
    set_bit(EIMSK, INT0);

    // === INTERRUPÇÕES POR MUDANÇA DE NÍVEL ===
    // PCICR – Pin Change Interrupt Control Register
    // Habilita interrupções no grupo PCINT2 (pinos PD0 a PD7)
    set_bit(PCICR, PCIE2);

    // PCMSK2 – Pin Change Mask Register 2
    // Habilita PCINT20 (PD4) e PCINT23 (PD7)
    set_bit(PCMSK2, PCINT20);
    set_bit(PCMSK2, PCINT23);

    // === HABILITA INTERRUPÇÕES GLOBAIS ===
    sei();
}

/*
| Atualizar producao
|
*/
void atualizar_producao(){
  uint8_t ROT_MotorH,ROT_MotorV;
  ROT_MotorH = (RPS*(velocidade_MH))/255;
  ROT_MotorV = (RPS*(velocidade_MV))/255;

  contador_RH += ROT_MotorH;
  contador_RV += ROT_MotorV;

  // verifica se as rotaçẽos bateram o nivel
  if(contador_RH >=10 && contador_RV >=25){
      contador_RH -=25;
      contador_RV -=10;
      producao_blocos++;
  }

  Dz_blocos= producao_blocos/10;
  Un_blocos= producao_blocos%10;

}

/*
|   Função leitura_ultrasonico
|   Realiza a leitura de distância com o sensor ultrassonico
|   Pino Trigger
|   Pino Echo
*/
uint16_t leitura_ultrasonico(){
 //==============================================================
    uint16_t contador = 0;

    // Garante nível baixo antes do pulso
    clr_bit(PORTB,Triger);
    _delay_us(2);
    // Pulso de 10us para disparar
    set_bit(PORTB,Triger);
    _delay_us(10);
    clr_bit(PORTB,Triger);

    // Espera início do sinal ECHO (borda de subida)
    //while (!(PINB & (1 << ECHO)));
    while(!tst_bit(PINB,Echo));

    // Artiva o contador 1 para contar até o retorno da onda
    TCNT1 = 0;
    TCCR1B = (1 << CS11); // Prescaler 8

    // se passar do empo máximo de contagem
    while (PINB & (1 << Echo)) {
        if (TCNT1 > 60000) break; // timeout (~32ms)
    }
    TCCR1B = 0;
    contador = TCNT1;
    // Calcula distância: tempo/2 / 58 = distância em cm (em ar)
    return ((contador)/2) / 58;

 //=================================================================
}




/*
| Função adc_init
| Configura os registradores do ADC para modo de conversão one-shoot e
| Tensão de referência interna
*/
void adc_init(){
    ADMUX = (1 << REFS0); // Referência AVCC, canal ADC0
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1); // ADC enable, prescaler 64
}


void shiftOut(uint8_t valor) {
    for (int8_t i = 7; i >= 0; i--) {
        // Envia bit mais significativo primeiro (MSB first)
        if (valor & (1 << i))
            set_bit(PORTC, DS);
        else
            clr_bit(PORTC, DS);

        pulse(PORTC, SHCP); // Pulso de clock
    }
    pulse(PORTC, STCP);     // Pulso de latch
}

/*
| Função timer1_init
| Configura e ativa o timer 1 para contar 200ms
*/

void timer1_temporizador_init() {
    /*
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS12) | (1 << CS10);
    TCCR1B &= ~(1 << CS11);
    */
    // Configura o Timer1 no modo CTC (WGM12 = 1)
    // Prescaler = 1024 (CS12 = 1, CS11 = 0, CS10 = 1)
    TCCR1B = 0b00001101;

    // Valor de comparação para 200 ms
    OCR1A = 3124;

    // Habilita interrupção por comparação com OCR1A
    //TIMSK1 |= (1 << OCIE1A);
    set_bit(TIMSK1,OCIE1A);
    timer_zerado=FALSE;
}

/*
| Função adc_read
| Realiza a leitura e conversão do valor análogico nas portas análogicas
| Channel é o canal de entrada do ADC_MUX de A0 até A7
| Retorna o valor digital da medida análogica
*/
uint16_t adc_read(uint8_t channel) {
   ADMUX = (ADMUX & 0xF0) | (channel & 0x0F); // Seleciona canal
   ADCSRA |= (1 << ADSC); // Inicia conversão
   while (ADCSRA & (1 << ADSC)); // Espera fim
   return ADC;
}

//====================================================================

/*
| Função buzzer_init_timer2
| Configura o timer 2 para gerar a onda quadrada com a frequênica especificada
*/

void buzzer_init_timer2(uint16_t frequencia_hz) {
    set_bit(DDRD, Buzzer); // OC2B como saída

    TCCR2A = 0;
    TCCR2B = 0;

    // Modo CTC (Clear Timer on Compare Match)
    clr_bit(TCCR2A, WGM20);
    set_bit(TCCR2A, WGM21);
    clr_bit(TCCR2B, WGM22);

    // Toggle na saída OC2B
    set_bit(TCCR2A, COM2B0);
    clr_bit(TCCR2A, COM2B1);

    // Prescaler = 64
    clr_bit(TCCR2B, CS22);
    set_bit(TCCR2B, CS21);
    set_bit(TCCR2B, CS20);

    // Calcula OCR2A
    uint16_t ocr_val = (16000000UL / (2UL * 64UL * frequencia_hz)) - 1;
    if (ocr_val > 255) ocr_val = 255;
    OCR2A = (uint8_t)ocr_val;
}

/*
| Função buzzer_off
| Desativa o buzzer desconectando oc2b
*/
void buzzer_off() {
    TCCR2A &= ~((1 << COM2B0) | (1 << COM2B1));
}

/*
| Função play_note
| Toca uma nota musical pelo tempo em milisegundos
*/
void play_note(uint16_t freq, uint16_t tempo_ms) {
    buzzer_init_timer2(freq);
    //_delay_ms(tempo_ms);
    while(tempo_ms--){
        _delay_ms(1);
    }
    buzzer_off();
    _delay_ms(50); // pausa curta entre notas
}


void alarme(){
    play_note(NOTE_C, 200);
    play_note(NOTE_D, 200);
    play_note(NOTE_C, 100);
    play_note(NOTE_F, 100);
    play_note(NOTE_C, 200);
    play_note(NOTE_D, 200);
    play_note(1000, 500);
}
//====================================================================
/*FUNÇÃO UART*/
void uart_init() {
    // Baud rate 9600
    uint16_t ubrr = F_CPU / 16 / 9600 - 1;
    UBRR0H = (ubrr >> 8);
    UBRR0L = ubrr;
    UCSR0B = (1 << TXEN0); // habilita transmissor
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8 bits
}

void uart_transmit(char data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

void uart_print(char* str) {
    while (*str) {
        uart_transmit(*str++);
    }
}

void print_number(uint16_t value) {
    char buffer[5];
    itoa(value, buffer, 10); // Converte para decimal
    uart_print(buffer);
    uart_print("\n");
}

void print_dados(){
  // exibe dados no monitor serial
  uart_print("============================\n");
  uart_print("Estado atual\n");
  print_number(estado_atual);
  uart_print("Blocos:");
  print_number(producao_blocos);
  uart_print("Presenca:");
  print_number(presenca);
  uart_print("Inclinacao:");
  print_number(desvio_inclinacao);
  // uart_print("reativar:");
  // print_number(reativar_producao);
  uart_print("Temperatura:");
  print_number(temperatura);
  uart_print("Nivel de oleo:");
  print_number(nivel_tanque);

  uart_print("Velocidade motor H:");
  print_number(velocidade_MH);
  uart_print("Velocidade motor V");
  print_number(velocidade_MV);
  uart_print("============================\n");
}
//====================================================================

/*FUNÇÃO DA COMUNICAÇÃO I2C*/

/*
|
|
*/
void onRequestHandler(){
    sendMessage();
}

void onReceiveHandler(int numBytes){
     if (numBytes > sizeof(buffer) - 1) numBytes = sizeof(buffer) - 1;

    // recebe bytes
    for (int i = 0; i < numBytes; i++) {
        buffer[i] = Wire.read();
    }
    buffer[numBytes] = '\0'; // Finaliza como string

    comando_supervisor = TRUE;
}
/*
|
|
*/
void wire_init_slave(){
  // Inicia como escravo com endereço
  Wire.begin(SLAVE_ADDRESS);
  // Configura função de resposta       
  Wire.onRequest(onRequestHandler);
  // Configura interrupção de recepção
  Wire.onReceive(onReceiveHandler);
}

// Comunicação I2C
void sendMessage() {
  // String msg = "Mensagem do chão: ";
  // msg += concat_msg;
  // concat_msg++;
  for (size_t i = 0; i < mensagemI2C.length(); i++) {
   
    Wire.write(mensagemI2C[i]);
  }
}

void data_to_send() {
  char buffer[5];
  mensagemI2C = "";

  itoa(status_producao, buffer, 10); // Converte estado para string
  //mensagemI2C += ",";
  mensagemI2C = buffer;
  mensagemI2C += ",";

  itoa(producao_blocos, buffer, 10); // Converte estado para string
  mensagemI2C += buffer;
  mensagemI2C += ",";

  itoa(presenca, buffer, 10);
  mensagemI2C += buffer;
  mensagemI2C += ",";

  itoa(desvio_inclinacao, buffer, 10);
  mensagemI2C += buffer;
  mensagemI2C += ",";

  itoa(temperatura, buffer, 10);
  mensagemI2C += buffer;
  mensagemI2C += ",";

  itoa(nivel_tanque, buffer, 10);
  mensagemI2C += buffer;
  mensagemI2C += ",";
}


void tratar_comando_pwm(const char* comando) {
    // Divide a string no separador '_'
    char comando_copia[32];
    strncpy(comando_copia, comando, sizeof(comando_copia));
    
    char* tipo = strtok(comando_copia, "_");
    char* valor_str = strtok(NULL, "_");

    if (tipo && valor_str) {
        uint8_t valor = atoi(valor_str); // Converte string para uint8_t

        if (strcmp(tipo, "pwmA") == 0) {
            //uart_print("PWM A setado para: ");
            //print_number(valor);
            // Aqui você pode aplicar o valor em OCR0A ou OCR1A
            velocidade_MH = valor;
        }
        else if (strcmp(tipo, "pwmB") == 0) {
            //uart_print("PWM B setado para: ");
            //print_number(valor);
            // Aqui você pode aplicar o valor em OCR0B ou OCR1B
            velocidade_MV = valor;
        }
        else {
            uart_print("Comando inválido\n");
        }
    }
}
//====================================================================


/*
| Função leitura_sensores
| Realiza a leitura do sensores e atualiza os valores das variaveis
*/
void leitura_sensores(){ 
    clr_bit(PORTC,PC0);
    float tensao,leitura; 
    leitura = (float) adc_read(0);
    tensao = (leitura*5.0)/1023.0;
    temperatura = tensao/ 0.010;
    //uart_print("Temperatura:");
    //print_number(temperatura);
    distancia = leitura_ultrasonico();
    nivel_tanque = (float) distancia;
    //uart_print("Distancia:");
    //print_number(distancia);

}

//========================================================================================================
//  Função de configuração PWM

/*
| Função config_pwm_timer0
| COnfigura o timer0 para modo pwm rapdio
*/
void config_pwm_timer0(){
  // Configura os pinos OC0A (PD6) e OC0B (PD5) como saída
    set_bit(DDRD, PD6);  // D6 - OC0A
    set_bit(DDRD, PD5);  // D5 - OC0B

    // Configura Timer0 em Fast PWM com TOP=255 (modo 3)
    set_bit(TCCR0A, WGM00);
    set_bit(TCCR0A, WGM01);
    clr_bit(TCCR0B, WGM02);

    // PWM não invertido nas duas saídas
    set_bit(TCCR0A, COM0A1); clr_bit(TCCR0A, COM0A0);  // OC0A habilitado, modo não invertido
    set_bit(TCCR0A, COM0B1); clr_bit(TCCR0A, COM0B0);  // OC0B habilitado, modo não invertido

    // Prescaler = 64 → Freq = 16 MHz / (64 × 256) ≈ 976 Hz
    clr_bit(TCCR0B, CS02);
    set_bit(TCCR0B, CS01);
    set_bit(TCCR0B, CS00);

    // Duty cycle inicial
    OCR0A = 0; // 0% no canal A
    OCR0B = 0; // 0% no canal B
}

// Define o duty cycle de 0 a 100% para OC0A (D6)
void set_pwmA(uint8_t duty_cycle) {
    if (duty_cycle > 100) duty_cycle = 100;
    OCR0A = (duty_cycle * 255) / 100;
}

// Define o duty cycle de 0 a 100% para OC0B (D5)
void set_pwmB(uint8_t duty_cycle) {
    if (duty_cycle > 100) duty_cycle = 100;
    OCR0B = (duty_cycle * 255) / 100;
}
//=========================================================================================================
// Função de tratamento da interrupções

/*
| Tratamento da interrupção externa 0
| no pino D2 do nano está ligado  botão de parada da produção
*/
ISR(INT0_vect){
  
  // Se a fabrica estiver em produção
  if(status_producao){
    // com a chamada da para atualiza o estado logico da variavel parada
    uart_print("Botao parada pressionado\n");
    status_producao = 0;
    cpl_bit(PORTB,LED_OP);
    // desativa os motores e ativa flag para rotina principal
    set_pwmA(0);
    set_pwmB(0);
  }
  // se estiver parada e não houver nenhuma flag ativa
  else if( !(desvio_inclinacao || presenca || temperatura_critca) ){
        uart_print("Producao retomada\n");
        //status_producao=1;
        reativar_producao =1;
        cpl_bit(PORTB,LED_OP);
  }
  else if(desvio_inclinacao || presenca || temperatura_critca){
    // caso as flags tenham sido limpas
    reativar_producao =1;
    uart_print("Flag atualizada\n");
  }

}

/*
| Tratamento da interrupção por mudança de pino 3 PORTD
| 
*/
ISR(PCINT2_vect){
  /*verifica qual pino ativou a interrupção */

  // PD4-D4 tem como entrada o sensor de inclinação
  if(tst_bit(PIND,Sensor_Inc) && !desvio_inclinacao){
    // informa o desvio da inclinação
    uart_print("A tora esta inclinada La ele\n");
    set_bit(PORTB,LED_OP);
    //print_number(tst_bit(PIND,Sensor_Inc));
    desvio_inclinacao = TRUE;
    // Desativa motores
    // desativa os motores
    set_pwmA(0);
    set_pwmB(0);
    status_producao=FALSE;
    // Com a flag ativada a rotina principal muda para o ajuste da inclinação
  }
  if(!tst_bit(PIND,Sensor_Inc) && desvio_inclinacao){
    uart_print("inclinacao Corrigida\n");
    desvio_inclinacao= FALSE;
  }
  // PD7-D7 tem como entrada o sensor de presença
  else if(tst_bit(PIND,Sensor_PIR) && !presenca){
    // informa a presença de um trabalhador perto das serras
    presenca = TRUE;
    status_producao=FALSE;
    uart_print("Tem alguém ai\n");
    print_number(presenca);
   // desativa os motores
    set_pwmA(0);// Comunicação I2C
    set_pwmB(0);
    // Com a flag ativada a rotina principal muda para a parada da produção
  }
  else if(!tst_bit(PIND,Sensor_PIR) && presenca){
    // informa a presença de um trabalhador perto das serras
    presenca = FALSE;
    reativar_producao= TRUE;
    uart_print("Nao tem alguem ai\n");
    print_number(presenca);
    // Com a flag ativada a rotina principal muda para a parada da produção
  }
}

/*
| Tratamento da interrupção por comparação no timer1
| Interrupção é gerada a cada 200ms
*/
ISR(TIMER1_COMPA_vect){
  // atualiza a flag de timer zerado
  timer_zerado =TRUE;
  // desativa e limpa o timer
  TCNT1 = 0;
  contador++;
  clr_bit( TCCR1B,CS12);
  clr_bit( TCCR1B,CS11);
  clr_bit( TCCR1B,CS10);
}
