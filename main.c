#include <avr/io.h>
#include <stdint.h>
#include <stdlib.h>

void T1Delay1ms(void);
void delayMS(uint16_t ms);

int main(void)
{
	//Pin configuration
	DDRB |= (1 << PB1);
	
	// ADC: AVcc ref, ADC0 (MUX = 0000)
	ADMUX  = (1 << REFS0);

	// Enable, auto-trigger, start first conversion, prescaler 128
	ADCSRA = (1 << ADEN) | (1 << ADATE) | (1 << ADSC)
	| (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	
	ICR1 = 39999;
	OCR1A = 3000;
	
	// Timer1: mode 14 (fast PWM, TOP = ICR1), non-inverting OC1A, prescaler 8
	TCCR1A = (1 << COM1A1) | (1 << WGM11);
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);
	
	/*while (1) {
		OCR1A = 1000 + ((uint32_t)ADC * 4000) / 1023;
	}*/
}

//Prescaler 64 is the 16Mhz ATMEga328p op speed 16mhz/64 = 250 kHz, 4microsecs per tick
//1ms = 1000microsecs / 4 microsecs = 250 ticks for 1ms
//Delay is 1ms meaning it can be called exactly for the required ms amount
void T1Delay1ms(void)
{
	TCNT1  = 65286;                         // 65536 - 65286 = 250 ticks x 4 us = 1 ms
	TCCR1A = 0x00;                          //normal Mode
	TCCR1B = (1 << CS11) | (1 << CS10);     // start, prescaler 64
	while (!(TIFR1 & (1 << TOV1))) { }      // wait for overflow
	TCCR1B = 0x00;                          // stop
	TIFR1  = (1 << TOV1);                   // clear flag
}

void delayMS(uint16_t ms)
{
	while (ms--) T1Delay1ms();
}