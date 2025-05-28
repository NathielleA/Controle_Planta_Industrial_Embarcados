/*
    Programa bt_acende_led.
    Realiza a leitura de um push button e acende uma led quando pressionado
    LED no pino D7 arduino nano
    Botão no pino D8 arduino nano
*/

#define F_CPU  16000000UL // frequência de clock do MCU

#include <avr/io.h>
#include <util/delay.h>

//Definições de macros - empregadas para o trabalho com os bits

#define set_bit(Y,bit_x) (Y|=(1<<bit_x)) /*ativa o bit x da  variável Y 
  (coloca em 1)*/ 
#define clr_bit(Y,bit_x) (Y&=~(1<<bit_x))/*limpa o bit x da variável Y  
  (coloca em 0)*/ 
#define  tst_bit(Y,bit_x) (Y&(1<<bit_x))   /*testa o bit x da variável Y 
 (retorna 0 ou 1)*/

#define  cpl_bit(Y,bit_x) (Y^=(1<<bit_x)) /*troca o estado do bit x da  
variável Y (complementa)*/ 

#define LED PD7
#define BT PB0

int main(){
  // configura pino D7 como saida
  set_bit(DDRD,LED);
  // configura pino D8 como entrada
  clr_bit(DDRB,BT);
  // configura pull-up
  clr_bit(MCUCR,PUD);
  set_bit(PORTB,BT);
  bool bt_press = true;

  while(1){
    bt_press = tst_bit(PINB,BT);
    if(!bt_press){
      clr_bit(PORTD,LED);
      _delay_ms(1000);
    }
    else{
       set_bit(PORTD,LED);
    }
    _delay_ms(500);
    /*
      // ativa o led
      clr_bit(PORTD,LED);
      _delay_ms(500);
      set_bit(PORTD,LED);
      _delay_ms(500);
      */
  }

  return 0;
}
