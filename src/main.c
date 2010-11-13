#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define BEATS 4
#define BEAT_CYCLE_INTERVAL 80
#define BEAT_INTERVAL 10

ISR(TIMER0_OVF_vect) {
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
	DDRB = 0x07;
	TCCR0 = _BV(CS02);
	TIMSK |= _BV(TOIE0);
	sei();
	while(1) {
		PORTB &= ~_BV(0);
		while (PINB & (_BV(3) | _BV(4) | _BV(5))) {
			asm volatile("nop" ::);
		}
		PORTB ^= _BV(1);
		PORTB |= _BV(0);
		_delay_ms(50);
	}
}
