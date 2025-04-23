#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

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

void adc_init() {
    ADMUX = (1 << REFS0); // Referência AVCC, canal ADC0
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1); // ADC enable, prescaler 64
}

uint16_t adc_read(uint8_t channel) {
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F); // Seleciona canal
    ADCSRA |= (1 << ADSC); // Inicia conversão
    while (ADCSRA & (1 << ADSC)); // Espera fim
    return ADC;
}

void print_temp(uint16_t value) {
  const float BETA = 3950; // Deve corresponder ao coeficiente beta do termistor

  float temp = 1 / (log(1 / (1023. / value - 1)) / BETA + 1.0 / 298.15) - 273.15;

  char buffer[10];
  dtostrf(temp, 5, 1, buffer); // Converte para decimal
  uart_print(buffer);
  uart_print("°C\n");
}

int main(void) {
    uart_init();
    adc_init();

    while (1) {
        uint16_t value = adc_read(0); // Leitura do ADC0
        print_temp(value);
        _delay_ms(1000);
    }
}
