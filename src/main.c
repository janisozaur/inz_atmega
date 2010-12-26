#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "uart.h"

#define BEATS 4
#define BEAT_CYCLE_INTERVAL 80
#define BEAT_INTERVAL 10

#define PIN_UP PB3
#define PIN_LEFT PB4
#define PIN_RIGHT PB5

volatile uint8_t ovfCounter;
uint8_t prevPin;
FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

ISR(TIMER0_OVF_vect) {
	++ovfCounter;
}

ISR(TIMER1_OVF_vect) {
	static uint8_t counter = 0;
	static uint8_t beats = BEATS;
	++counter;
	if (beats == BEATS) {
		if (counter == BEAT_CYCLE_INTERVAL) {
			PORTB ^= _BV(2);
			--beats;
			counter = 0;
		}
	} else {
		if (counter == BEAT_INTERVAL) {
			PORTB ^= _BV(2);
			--beats;
			if (beats == 0) {
				beats = BEATS;
			}
			counter = 0;
		}
	}
}

int main() {
	uart_init();
	_delay_ms(3000);
	fprintf(&uart_str, "hello\n");
	DDRB = 0x07;
	// 16-bit counter, up-counting to 0xFFFF and overflowing, without prescaler
	TCCR1B = _BV(CS10);
	TIMSK |= _BV(TOIE0) | _BV(TOIE1);
	sei();
	while (1) {
		ovfCounter = 0;
		TCNT0 = 0;
		prevPin = 0xFF;
		uint8_t tcntU = 0xFF, coeffU = 0xFF;
		uint8_t tcntL = 0xFF, coeffL = 0xFF;
		uint8_t tcntR = 0xFF, coeffR = 0xFF;
		// reset prescaler
		SFIOR |= _BV(PSR10);
		// prescaler: clk/8
		TCCR0 = _BV(CS01);
		PORTB &= ~_BV(0);
		//_delay_ms(5);
		uint8_t steady = 0;
		do {
			uint8_t currentPin = PINB;
			if (((prevPin & _BV(PIN_LEFT)) != 0) && ((currentPin & _BV(PIN_LEFT)) == 0)) {
				tcntL = TCNT0;
				coeffL = ovfCounter;
			}
			if (((prevPin & _BV(PIN_RIGHT)) != 0) && ((currentPin & _BV(PIN_RIGHT)) == 0)) {
				tcntR = TCNT0;
				coeffR = ovfCounter;
			}
			if (((prevPin & _BV(PIN_UP)) != 0) && ((currentPin & _BV(PIN_UP)) == 0)) {
				tcntU = TCNT0;
				coeffU = ovfCounter;
			}
			if ((prevPin & (_BV(PIN_UP) | _BV(PIN_LEFT) | _BV(PIN_RIGHT))) == 0) {
				++steady;
			}
			prevPin = currentPin;
		// gether this many samples prior to considering them a stable result
		} while (steady < 3);
		// stop timer/counter0
		TCCR0 = 0x00;
		PORTB ^= _BV(1);
		PORTB |= _BV(0);
		uart_putchar(0, NULL);
		//uart_putchar(1, NULL);
		//uart_putchar(2, NULL);
		uart_putchar(tcntU, NULL);
		uart_putchar(coeffU, NULL);
		uart_putchar(tcntL, NULL);
		uart_putchar(coeffL, NULL);
		uart_putchar(tcntR, NULL);
		uart_putchar(coeffR, NULL);
		uart_putchar(0xFF, NULL);
		_delay_ms(20);
	}
}
