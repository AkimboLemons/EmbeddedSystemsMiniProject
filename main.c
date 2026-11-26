/*Define the clock frequency of 16MHz*/
#define F_CPU 16000000UL

/*Imports*/
#include <avr/io.h>
#include <stdint.h>

/*Define functions*/
void T0Delay1ms(void);
void delayMS(uint16_t ms);

/*Define LCD pins in usage*/
#define D4 eS_PORTD4   // Arduino D4
#define D5 eS_PORTD5   // Arduino D5
#define D6 eS_PORTD6   // Arduino D6
#define D7 eS_PORTD7   // Arduino D7
#define RS eS_PORTD2   // Arduino D2
#define EN eS_PORTD3   // Arduino D3

/*Duds: unused in 4-bit mode, parked on free pins (A1-A4)*/
#define D0 eS_PORTC1
#define D1 eS_PORTC2
#define D2 eS_PORTC3
#define D3 eS_PORTC4

/*Define LED placeholder for laser?*/
#define LED es_PORTB7

/*File import required after defining pins*/
#include "lcd.h"

/*Define placeholder status names, to be customized later on*/
typedef enum {
	STATUS_IDLE,
	STATUS_RUNNING,
	STATUS_ERROR,
	STATUS_DONE,
	STATUS_COUNT
} status_t;

/*Text for each status to be displayed on the LCD*/ 
const char *status_text[STATUS_COUNT][2] = {
	[STATUS_IDLE]    = {"Status:", "Idle"},
	[STATUS_RUNNING] = {"Status:", "Running"},
	[STATUS_ERROR]   = {"Status:", "ERROR"},
	[STATUS_DONE]    = {"Status:", "Done"},
};

/*LCD function definitions, done after defining the enum*/
void lcd_setup(void);
void show_status(status_t s);

/*LED states On/Off*/
typedef enum {
	off = 0,
	on  = 1
} led_state_t;

/*LED On/Off Pin13*/
void led(led_state_t state)
{
	DDRB |= (1 << PB5); //Set PB5(D13)            

	if (state == on)
	PORTB |=  (1 << PB5); // LED ON PB5/D13 High
	else
	PORTB &= ~(1 << PB5); //LED OFF PB5/D13 LOW
}

/*LCD Pins defined and initialized*/
void lcd_setup(void)
{
	DDRD |= 0xFC; // PD2-PD7 outputs (RS, EN, D4-D7)
	Lcd4_Init();
}

/*Display text to the LCD*/
void show_status(status_t s)
{
	Lcd4_Clear(); //Clear previous message
	delayMS(2); // clear takes 1.6 ms
	Lcd4_Set_Cursor(1, 0); //Set Cursor to the Start of the First Line
	Lcd4_Write_String(status_text[s][0]);  //Write Status:
	Lcd4_Set_Cursor(2, 0); //Set cursor to the Start of the Second Line
	Lcd4_Write_String(status_text[s][1]); //Write whatever current status
}

/*Read one ADC channel (0-7) we are using 0, 1 and return a value of 0-1023*/ 
uint16_t adc_read(uint8_t channel)
{
	ADMUX = (1 << REFS0) | (channel & 0x07);   // AVcc ref + select channel
	ADCSRA |= (1 << ADSC);                     // start one conversion
	while (ADCSRA & (1 << ADSC)) { }           // wait until it finishes
	return ADC;
}

int main(void)
{
	/*Pin configuration for servo output*/
	DDRB |= (1 << PB1) | (1 << PB2); //D9-D10 High

	/*ADC Enable, 128 Prescaler and auto enable not on*/
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	
	/*Timer1, Mode14(fast PWM, TOP = ICR1), prescaler 8*/
	ICR1 = 39999; // 40000 ticks x 0.5 us = 20 ms
	OCR1A = 3000; //Centre Servo 1 on D9
	OCR1B = 3000; //Centre Servo 2 on D10
	
	TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11);  // OC1A + OC1B on
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11); //Mode 14, prescaler 8
	
	/*LCD setup*/
	lcd_setup();
	show_status(STATUS_IDLE);
	
	while (1) {
		OCR1A = 1000 + ((uint32_t)adc_read(0) * 4000) / 1023;  // Pot A0 determines servo D9 position
		OCR1B = 1000 + ((uint32_t)adc_read(1) * 4000) / 1023;  // Pot A1 determines servo D10 position
		/*
		if mode = manual
			OCR1A = 1000 + ((uint32_t)adc_read(0) * 4000) / 1023;  // Pot A0 determines servo D9 position
			OCR1B = 1000 + ((uint32_t)adc_read(1) * 4000) / 1023;  // Pot A1 determines servo D10 position
			send status message on angles of turret
		else mode = auto
			while no object <50cm
				sway OCR1A between 1000 and 4000
				send status message finding target
			send status target found
			fire
			send status target eliminated
		*/
	}
}

/*Delay*/
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