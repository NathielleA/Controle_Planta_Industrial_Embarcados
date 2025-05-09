#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

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

const uint8_t digitos[10] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
  0b01101101, // 5
  0b01111101, // 6
  0b00000111, // 7
  0b01111111, // 8
  0b01101111  // 9
};

// Exibe dois dígitos no display de 7 segmentos conectado aos PORTB e PORTD
void mostrar_display(uint8_t temp) {
    uint8_t dezena = temp / 10;
    uint8_t unidade = temp % 10;

    // Exemplo: PORTB controla a dezena, PORTD controla a unidade
    PORTB = digitos[dezena];
    PORTD = digitos[unidade];
}

int main(void) {
    adc_init();

    // Configure PORTB e PORTD como saída (conectados aos displays)
    DDRB = 0xFF;
    DDRD = 0xFF;

    while (1) {
        uint16_t value = adc_read(0);

        // Se estiver usando um termistor NTC (como no seu código anterior)
        const float BETA = 3950;
        float temp = 1 / (log(1 / (1023.0 / value - 1)) / BETA + 1.0 / 298.15) - 273.15;

        // Mostra somente a parte inteira da temperatura no display
        if (temp >= 0 && temp < 100) {
            mostrar_display((uint8_t)temp);
        }
        _delay_ms(500);
    }
}
