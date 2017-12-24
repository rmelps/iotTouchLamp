// R Melpignano 18 DEC 2017
// ------------------------

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include "rgbledPinDefines.h"
#include "USART.h"
#include "SPI.h"

#define COLOR_SAVE_DELAY_COUNT		39063

// ------ GLOBALS -------

volatile uint8_t rBrightness = 0;
volatile uint8_t gBrightness = 0;
volatile uint8_t bBrightness = 0;

// ------ INTERRUPTS -------
ISR (PCINT1_vect) {
	// check to see which buttons are active
	// if active, increase the brightness value
	// we are comparing the input values on the button pin (which are pulled high
	// by the pullup resistor unless the button is active)

	if ((BUTTON_PIN & (1 << RBUTTON)) == 0) {
		rBrightness += 15;
	}
	if ((BUTTON_PIN & (1 << GBUTTON)) == 0) {
		gBrightness += 15;
	}
	if ((BUTTON_PIN & (1 << BBUTTON)) == 0) {
		bBrightness += 15;
	}

	// Set Timer 1 count to 0
	TCNT1 = 0;
	// Clear flag on compare match A
	TIFR1 |= (1 << OCF1A);
	// Enable interrupts on Timer 1, to keep track of delay after user selects a color
	TIMSK1 |= (1 << OCIE1A);
}

ISR (TIMER1_COMPA_vect) {
	// Called when the output color has changed and has remained constant for 
	// at least the amount of time in the COLOR_SAVE_DELAY_COUNT
	printString("Time to Reset \r\n");
	// Disable interrupts on Timer 1
	TIMSK1 &= ~(1 << OCIE1A);
}

static inline void initTimers(void) {
	// Set the clock prescale divider to 1, so running at 8 Mhz
	clock_prescale_set(clock_div_1);

	// ------ TIMER 0 -------

	// Fast PWM mode, update OCRx at OCRA
	TCCR0A |= (1 << WGM00) | (1 << WGM01);

	// Set fast PWM mode clock prescaler to F_CPU/256
	TCCR0B |= (1 << CS02);

	// Clear OC0A on compare match, set OC0A at TOP
	TCCR0A |= (1 << COM0A1);

	// Clear OC0B on compare match, set OC0B at TOP
	TCCR0A |= (1 << COM0B1);

	// ------ TIMER 1 -------
	// CTC Mode, no pin outputs. CTC mode, OCR1A: TOP
	// Set prescaler to F_CPU/1024: 7812.5 Hz (assume 8MHz clock)
	// This will give us a maximum count value of 8.388 seconds
	TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10);

	// Set OCR1A so that an interrupt triggers at COLOR_SAVE_DEL_COUNT
	OCR1A = COLOR_SAVE_DELAY_COUNT;

	// Interrupts on Timer 1 will not be initially available. Only after the output
	// color is updated.
	//TIMSK1 |= (1 << OCIE1A)

	// ------ TIMER 2 -------
	// Fast PWM mode, update OCRx at OCRA
	TCCR2A |= (1 << WGM20) | (1 << WGM21);

	// Set fast PWM mode clock prescaler to F_CPU/256
	TCCR2B |= (1 << CS22) | (1 << CS21);

	// Clear OC2B on compare match, set OC2B at TOP
	TCCR2A |= (1 << COM2B1);
}

static inline void initButtonInterrupts(void) {
	// Enable interrupts on pin change interrupt vector 1
	PCICR |= (1 << PCIE1);
	// Enable interrupts specifically on the BUTTON pins
	PCMSK1 |= (1 << PCINT11) | (1 << PCINT10) | (1 << PCINT9);

}

int main(void) {
	// ------ INITS -------

	initTimers();
	initUSART();
	initButtonInterrupts();

	// ------ LED SETUP -------

	// Set LED data direction to output on connected LEDs
	LED_DDR |= (1 << RLED) | (1 << GLED) | (1 << BLED);

	// ------ BUTTON SETUP -------
	// Ensure that BUTTON_DDR is set for input
	BUTTON_DDR &= ~((1 << RBUTTON) | (1 << GBUTTON) | (1 << BBUTTON));
	// Enable pullup resistors on BUTTON_PIN
	BUTTON_PORT |= (1 << RBUTTON) | (1 << GBUTTON) | (1 << BBUTTON);


	// ------ EVENT LOOP -------

	while(1) {
		sei();
		OCR0A = rBrightness;
		OCR0B = gBrightness;
		OCR2B = bBrightness;
	}

	return 1;
}