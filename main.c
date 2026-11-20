//Define clock speed
#define F_CPU 16000000UL

//Imports
#include <avr/io.h>
#include <stdint.h>

//Define functions
void T0Delay1ms(void);
void delayMS(uint16_t ms);

//Define LCD pins including the duds
#define D4 eS_PORTD4   // Arduino D4
#define D5 eS_PORTD5   // Arduino D5
#define D6 eS_PORTD6   // Arduino D6
#define D7 eS_PORTD7   // Arduino D7
#define RS eS_PORTD2   // Arduino D2
#define EN eS_PORTD3   // Arduino D3

//Duds: unused in 4-bit mode, parked on free pins (A1-A4)
#define D0 eS_PORTC1
#define D1 eS_PORTC2
#define D2 eS_PORTC3
#define D3 eS_PORTC4

//Define LED placeholder for laser?
#define LED es_PORTB7

//File import required after defining pins
#include "lcd.h"

//Define placeholder status names
typedef enum {
	STATUS_IDLE,
	STATUS_RUNNING,
	STATUS_ERROR,
	STATUS_DONE,
	STATUS_COUNT      // always last: number of statuses
} status_t;

// Text for each status, in the same order as the enum
const char *status_text[STATUS_COUNT][2] = {
	[STATUS_IDLE]    = {"Status:", "Idle"},
	[STATUS_RUNNING] = {"Status:", "Running"},
	[STATUS_ERROR]   = {"Status:", "ERROR"},
	[STATUS_DONE]    = {"Status:", "Done"},
};

//LCD function defintions after defining typedef etc
void lcd_setup(void);
void show_status(status_t s);

//LED states
typedef enum {
	off = 0,
	on  = 1
} led_state_t;

//Turn the LED on pin 13 (PB5) on or off
void led(led_state_t state)
{
	DDRB |= (1 << PB5);            // make sure PB5 is an output

	if (state == on)
	PORTB |=  (1 << PB5);      // LED on
	else
	PORTB &= ~(1 << PB5);      //LED off
}

void lcd_setup(void)
{
	//Pin configuration for lcd signals
	DDRD |= 0xFC;   // PD2-PD7 outputs (RS, EN, D4-D7)
	Lcd4_Init();
}

void show_status(status_t s)
{
	Lcd4_Clear();
	delayMS(2);                      // clear takes 1.6 ms
	Lcd4_Set_Cursor(1, 0);
	Lcd4_Write_String(status_text[s][0]);
	Lcd4_Set_Cursor(2, 0);
	Lcd4_Write_String(status_text[s][1]);
}

int main(void)
{
	//Pin configuration for servo signal
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
	
	//LCD setup
	lcd_setup();
	show_status(STATUS_IDLE);
	
	while (1) {
		OCR1A = 1000 + ((uint32_t)ADC * 4000) / 1023;   // pot -> servo
		// status logic: only call show_status() when the status changes
	}
}

void T0Delay1ms(void)
{
	TCNT0  = 6;                          // 256 - 6 = 250 ticks x 4 us = 1 ms
	TCCR0A = 0x00;                       // normal mode
	TCCR0B = (1 << CS01) | (1 << CS00);  // prescaler 64
	while (!(TIFR0 & (1 << TOV0))) { }
	TCCR0B = 0x00;
	TIFR0  = (1 << TOV0);
}

void delayMS(uint16_t ms)
{
	while (ms--) T0Delay1ms();
}