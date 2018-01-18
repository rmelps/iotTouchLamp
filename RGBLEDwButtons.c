// R Melpignano 18 DEC 2017
// ------------------------
#include "RGBLEDwButtons.h"

// ------ COMMANDS -------
typedef void(*commandFuncs)(char *parameters[], uint8_t len);


struct commandStruct {
	const commandFuncs execute;
	char *parameters[2];
};

// The AT commands to be executed, in order. iCommands keeps track of the current command being executed
// These commands will be executed at startup
struct commandStruct commands[] = {
	{
		&ATsetCurrentWifiMode,
		"1"
	},
	{
		&ATconnectToAPI,
		{"ss", "password"}
	},
	{
		&ATconnectToAPI,
		{"ssid", "pass"}
	}
};

// ----- GLOBALS -----
volatile uint8_t colorBalance[3];
volatile char receiveBuffer[30];
volatile uint8_t iCommands, iReceiveBuffer;
volatile uint8_t nextCommand = 1;


// ------ INTERRUPTS -------
ISR (PCINT1_vect) {
	// check to see which buttons are active
	// if active, increase the brightness value
	// we are comparing the input values on the button pin (which are pulled high
	// by the pullup resistor unless the button is active)

	if (R_BUTTON_DOWN) {
		colorBalance[0] += 15;
		OCR0A = colorBalance[0];
	}
	if (G_BUTTON_DOWN) {
		colorBalance[1] += 15;
		OCR0B = colorBalance[1];
	}
	if (B_BUTTON_DOWN) {
		colorBalance[2] += 15;
		OCR2B = colorBalance[2];
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
	printString("Writing... \r\n");

	// Write the current configuration to EEPROM
	EEPROM_writePage(COLOR_SAVE_ADDRESS,sizeof(colorBalance), colorBalance);
	printString("Written... \r\n");

	// Disable interrupts on Timer 1
	TIMSK1 &= ~(1 << OCIE1A);
}

ISR (USART_RX_vect) {
	char received = UDR0;
	if (received == '\n') {
		if (compareString(receiveBuffer, "OK\r")){
			nextCommand++;
		}
		else if (compareString(receiveBuffer, "ERROR\r")){
			iCommands--;
		} 
		else if (receiveBuffer[0] == '+') {
			printString("network");
		}
		else {
			printVolatileString(receiveBuffer);
		}
		iReceiveBuffer = 0;
		clearBuffer(receiveBuffer, ARRAY_LENGTH(receiveBuffer));
	} else {
		receiveBuffer[iReceiveBuffer] = received;
		iReceiveBuffer++;
	}
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
	// COLOR_SAVE_DEL_COUNT is derived from [t / (1/prescaler)] where t is the desired
	// delay time, in seconds. 
	OCR1A = COLOR_SAVE_DELAY_COUNT;

	// Interrupts on Timer 1 will not be initially available. Only after the output
	// color is updated.
	// where TIMSK1 |= (1 << OCIE1A) will enable interrupt.

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
	// ------ VARS -------

	char ssid[30], pswd[30];

	// ------ INITS -------

	initTimers();
	initUSART();
	initSPI_Master();

	// ----- EEPROM CHECK -----
	EEPROM_readPage(COLOR_SAVE_ADDRESS,sizeof(colorBalance), colorBalance);

	// ----- AT COMMANDS -----
	/*
	char *ssidPass[] = {
		"ssid",
		"password"
	};
	*/

	// Enable Global interrupts
	sei();

	// ------ NETWORK SETUP ------
	/*
	char test[] = "testPSWD";
	EEPROM_writePage(PSWD_SAVE_ADDRESS, sizeof(test), test);
	*/

	// 1. Retrieve saved SSID from EEPROM, if one exists

	EEPROM_readPage(SSID_SAVE_ADDRESS, sizeof(ssid), ssid);

	// If an ssid was found, grab the password from EEPROM and attempt to connect
	//TODO: FOR TESTING ONLY!!! Alter the operator in the IF statement to skip this loop
	// SHOULD BE RETURNED to '!=' for production
	if (ssid[0] != 0) {
		SSID_CONFIG = ssid;

		// Grab password from EEPROM
		EEPROM_readPage(PSWD_SAVE_ADDRESS, sizeof(pswd), pswd);
		PSWD_CONFIG = pswd;

		// Attempt to connect to WiFi
		API_CONNECT_EXECUTE;

	} else {
		// execute COMMANDS serially to setup WiFi connection
		while (iCommands < ARRAY_LENGTH(commands)) {
			// TODO: Execute commands here
			commands[iCommands].execute(commands[iCommands].parameters, sizeof(commands[iCommands].parameters));
			iCommands++;
			while (iCommands == nextCommand) {
				// Wait until nextCommand is ready to be executed
			}
		}
	}

	/*
	for (uint16_t i = 0; i < ARRAY_LENGTH(commands); i++) {
		commands[iCommands].execute(commands[iCommands].parameters, sizeof(commands[iCommands].parameters));
		iCommands++;
	}
	
	commands[2].parameters[0] = "updated";
	commands[2].execute(commands[2].parameters, sizeof(commands[2].parameters));
	*/

	// ------ LED SETUP -------

	// Set LED data direction to output on connected LEDs
	LED_DDR |= (1 << RLED) | (1 << GLED) | (1 << BLED);

	// ------ BUTTON SETUP -------
	// Ensure that BUTTON_DDR is set for input
	BUTTON_DDR &= ~((1 << RBUTTON) | (1 << GBUTTON) | (1 << BBUTTON));
	// Enable pullup resistors on BUTTON_PIN
	BUTTON_PORT |= (1 << RBUTTON) | (1 << GBUTTON) | (1 << BBUTTON);

	// ----- INTERRUPT INIT ------
	initButtonInterrupts();

	// ------ EVENT LOOP -------

	// Initialize lamp color
	OCR0A = colorBalance[0];
	OCR0B = colorBalance[1];
	OCR2B = colorBalance[2];

	printString("Entering main loop");
	while(1) {
	// Can do anything here that needs to be looped
	// CPU is currently freed up while waiting for interrupts
	}

	// This line is never reached
	return 0;
}

void clearBuffer(volatile char *array, uint8_t len) {
	for (uint8_t i = 0; i < len; i++) {
		*(array + i) = 0;
	}
}

uint8_t compareString(volatile char *array, const char compStr[]) {
	for (uint8_t i = 0; i < ARRAY_LENGTH(compStr); i++){
		if (*(array + i) != compStr[i]) {
			return 0;
		}
	}
}
