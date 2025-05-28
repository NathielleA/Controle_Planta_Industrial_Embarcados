/*
  PROGRAMA SUPERVISOR
*/

#include <avr/io.h>
#include <util/delay.h>
#include <Wire.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL
#define TRUE 1
#define FALSE 0

// Define os dispositivos conetados aos pino do nano

#define potenciometro_A PC0
#define potenciometro_B PC1
#define LED_OP PB0
#define bt_parada PD2

// Comunicação i2c
#define SDA PC4
#define SCL PC5
#define SLAVE_ADDRESS 0x08

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

// Cabeçalho de funções
void config_gpio();
void config_interrupcoes();
void leitura_pot();
void timer0_temporizador_init();
void timer1_temporizador_init();
void print_dados();
void adc_init();
uint16_t adc_read(uint8_t channel);
void print_number(uint16_t value);
void uart_print(char* str);
void uart_transmit(char data);
void uart_init();

void wire_solicitar_dados(uint8_t* buffer, uint8_t quantidade);
void wire_enviar_comando_parar();
void wire_enviar_comando_retomada();
void wire_enviar_string(const char* mensagem);

ISR(INT0_vect);
ISR(INT1_vect);
ISR(TIMER0_COMPA_vect);
ISR(TIMER1_COMPA_vect);

// configuração de variaveis globais
static volatile uint8_t status_producao=1;    // 0 = producao parada 1 = producao ativa
static volatile uint8_t desvio_inclinacao=0;  // 0 = em ângulo    1 = fora do ângulo    
static volatile uint8_t presenca=0;           // 0 = sem presença 1= presença detectada
static volatile uint8_t velocidade_MotorV,velocidade_motorH;
static volatile uint8_t enviar_comando=FALSE;
volatile float temperatura;
volatile float nivel_tanque;
volatile uint8_t blocos;
static volatile uint8_t contador=0,contador_ms=0;
static volatile uint8_t buffer[20];  //buffer para mensagens i2c

String mensagemI2C = "";

void setup() {
  // put your setup code here, to run once:
    set_bit(DDRB, PB5);
    config_gpio();
    uart_init();
    Wire.begin();
    config_interrupcoes();
    // Inicializa o Timer1
    timer1_temporizador_init();
    adc_init();
    // Inicializa timer 0;
    timer0_temporizador_init();
}

void loop() {
  // put your main code here, to run repeatedly:
  principal();
}

void sendMessage() {
  // String msg = "Mensagem do chão: ";
  // msg += concat_msg;
  // concat_msg++;
  for (size_t i = 0; i < mensagemI2C.length(); i++) {
    Wire.write(mensagemI2C[i]);
  }
}

void principal(){
    const char* identificadores[] = {
        "Produção: ", "Blocos: ", "Presença: ", "Inclinação: ", "Temperatura: ", "Nível: "
    };
    uint8_t idx = 0;

    while (1) {
        // verfica se passou o tempo para leitura dos potenciometros
        if(contador_ms==10){
          // limpa_contador
          contador_ms=0;
          // realiza a leitura analogia
          leitura_pot();
          //envia dados para o chao de fabrica

        }

        // verifica se passou o tempo requsição dos dados do chão de fábrica
        if(contador == 3){
          // limpa contador
          contador=0;
          //solicita dados do chao de fabrica
          wire_solicitar_dados(buffer, 25);

          // exibe os dados recebidos
          char str[25];
          memcpy(str, buffer, 25);
          str[15] = '\0'; // Adiciona o terminador nulo
          char *token = strtok(str, ",");
          
          uart_print("====================================\n");
          for (int i = 0; i < 6; i++)  {

            // Atualiza o status da produção
            if (idx == 0) {
              if(strcmp(token,"1") == 0){
                status_producao = TRUE;
                clr_bit(PORTB,LED_OP);
              }
              else{
                set_bit(PORTB,LED_OP);
                status_producao = FALSE;
              }
            }

            uart_print(identificadores[idx]);
            uart_print(token);  // ou formatar com identificador
            uart_print("\n");
            token = strtok(NULL, ",");
            idx++;
          }
          // limpa idx (int i = 0; i < 7; i++)
          idx =0;
          
          // verifica solicitação de parada ou retomada
          if(enviar_comando){
            enviar_comando =  FALSE;
            if(status_producao){
              wire_enviar_comando_parar();
            }
            else{
              wire_enviar_comando_retomada();
            }
          }
        }
    }
}

uint8_t map_adc_to_percent(uint16_t adc_value) {
    return (adc_value * 100) / 1023;
}

void config_gpio(){
   //configura direcao dos pinos
  DDRD = 0b11110000;
  DDRB = 0b00000001;
  DDRC = 0x00;

  // configura pull-up na entrada
  PORTD = 0b11111100;
}

void leitura_pot(){
          uint16_t valor_potA = adc_read(0); // PC0
          _delay_us(10);
          uint16_t valor_potB = adc_read(1); // PC1

          uint8_t potA_percent = map_adc_to_percent(valor_potA);
          uint8_t potB_percent = map_adc_to_percent(valor_potB);

          //potA_percent = mapear_faixa(valor_potA, 150, 170, 0, 40);
          //potB_percent = mapear_faixa(valor_potB, 150, 170, 0, 40);

          //uart_print("Pot A (%): ");
          //print_number(valor_potA);

          //uart_print("Pot B (%): ");
          //print_number(valor_potB);

          char msg[16];
          snprintf(msg, sizeof(msg), "%s_%d", "pwmA", potA_percent); // Monta string: pwmA_123
          wire_enviar_string(msg);
          snprintf(msg, sizeof(msg), "%s_%d", "pwmB", potA_percent); // Monta string: pwmA_123
          wire_enviar_string(msg);
}


uint8_t mapear_faixa(float valor, float original_min, float original_max, float novo_min, float novo_max) {
    // Mapeamento linear
    return (valor - original_min) * (novo_max - novo_min) / (original_max - original_min) + novo_min;
}

void config_interrupcoes(){
  // configura interrupções externas por borda de descida
  EICRA = 0b00001010;
  //habilita as duas interrupções externas
  EIMSK = (1<<INT1) | (1<<INT0);
  // habilita interrupções
  sei();

}

//========================================================
//    Funções de tratamento das interrupções
/*
| Função de tratamento da interrupção externa 0
| Ao botão ser pressionado solicita parada ou retoma a produção
*/
ISR(INT0_vect){
  //uart_print("teste\n");
    // dispara flag para parada ou retomada da producao
  enviar_comando = TRUE;
  if(status_producao){
    //solicita parada da produção
    uart_print("Parada solicitada\n");
    set_bit(PORTB,LED_OP);
    status_producao= FALSE;
  }
  else{
    // envia comando para retomar producao
    uart_print("Retomada solicitada\n");
    clr_bit(PORTB,LED_OP);
    status_producao=TRUE;
  }

}
/*
| Função de tratamento da interrupção externa 1
| Ao receber um sinal lógico, solicita a ao dispositivo escravo o envio
| de informações
*/
ISR(INT1_vect){
  
}

ISR(TIMER0_COMPA_vect){
  //uart_print("passou 10 milisegundos\n");
  // incrementa o contador e reinicia a temporização
  contador_ms++;
}


ISR(TIMER1_COMPA_vect) {
    // Código executado a cada 1 segundo
    // Por exemplo: inverter um pino
    PORTB ^= (1 << PB5); // Alterna LED no pino 13 (PB5)
    //uart_print("passou 1 segundo\n");
    contador++;
}
//========================================================

// Funções de configuração e utilização da usart

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

//========================================================

/*FUNÇÕES DA COMUNICAÇO I2C*/

/*
| Função wire_solicitar_dados
| Solicita dados do chão de fábrica
*/
void wire_solicitar_dados(uint8_t* buffer, uint8_t quantidade) {
    Wire.requestFrom( (uint8_t)SLAVE_ADDRESS, quantidade);

    uint8_t i = 0;
    while (Wire.available() && i < quantidade) {
        buffer[i++] = Wire.read();
    }
}


/*
| Função wire_enviar_comando_parar
| Envia uma mensagem via i2c ao chão de fabrica para parar a produção
*/
void wire_enviar_comando_parar(){
    const char* comando = "parar_producao";
    Wire.beginTransmission(SLAVE_ADDRESS);
    Wire.write(comando); // Envia string como array de bytes
    Wire.endTransmission();
    uart_print("Parada da produção\n");
}

void wire_enviar_comando_retomada(){
    const char* comando = "voltar_producao";
    Wire.beginTransmission(SLAVE_ADDRESS);
    Wire.write(comando); // Envia string como array de bytes
    Wire.endTransmission();
    uart_print("Produção retomada\n");
}
/*
| Função wire_enviar_string
| Envia a string recebida como parâmetro ao chão de fábrica
*/
void wire_enviar_string(const char* mensagem) {
    Wire.beginTransmission(SLAVE_ADDRESS);
    Wire.write(mensagem);  // Envia a string (como array de bytes)
    Wire.endTransmission();
}


//========================================================
/* Funções de uso e configuração do ADC*/

/*
| Função adc_init
| Configura os registradores do ADC para modo de conversão one-shoot e
| Tensão de referência interna
*/
void adc_init(){
    ADMUX = (1 << REFS0); // Referência AVCC, canal ADC0
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1); // ADC enable, prescaler 64
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

//========================================================

//=========================================================
/* Funções de configuração dos temporizadores */

/*
| Função timer0_temporizador_init
| COnfigura o timer 0 para contar 10ms e gerar in
*/
void timer0_temporizador_init(){
  // Modo CTC (WGM01 = 1, WGM00 = 0)
    set_bit(TCCR0A, WGM01);
    clr_bit(TCCR0A, WGM00);
    
    // Prescaler = 1024 (CS02 = 1, CS01 = 0, CS00 = 1)
    set_bit(TCCR0B, CS02);
    clr_bit(TCCR0B, CS01);
    set_bit(TCCR0B, CS00);

    OCR0A = 156;
    // Habilita a interrupção por comparação
    set_bit(TIMSK0, OCIE0A);

}
void timer1_temporizador_init(){
   // Modo CTC (WGM12 = 1, WGM13:0 = 0100)
    set_bit(TCCR1B, WGM12);
    clr_bit(TCCR1A, WGM10);
    clr_bit(TCCR1A, WGM11);
    clr_bit(TCCR1B, WGM13);

    // Prescaler = 1024 (CS12 = 1, CS11 = 0, CS10 = 1)
    set_bit(TCCR1B, CS12);
    clr_bit(TCCR1B, CS11);
    set_bit(TCCR1B, CS10);

    // Valor de comparação para 1 segundo
    OCR1A = 15624;

    // Habilita interrupção por comparação com OCR1A
    set_bit(TIMSK1, OCIE1A);
}
