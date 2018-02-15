// R Melpignano 18 DEC 2017
// ------------------------
#include "RGBLEDwButtons.h"

// ----- GLOBALS -----
volatile uint8_t colorBalance[3];
volatile char receiveBuffer[LAR_BUFFER_SIZE];
volatile uint8_t iCommands, iReceiveBuffer, errorCount;
volatile uint8_t nextCommand = 1;
volatile char ssid[SM_BUFFER_SIZE], pswd[SM_BUFFER_SIZE];
volatile char linkID[] = "0";
volatile char dataR[3], dataG[3], dataB[3];

// ------ COMMANDS -------
typedef void(*commandFuncs)(char *parameters[], uint8_t len);

struct commandStruct {
	const commandFuncs execute;
	char *parameters[2];
};

// The AT commands to be executed, in order. iCommands keeps track of the current command being executed
// These commands will be executed at startup
struct commandStruct initCommands[] = {
	{
		&ATsetCurrentWifiMode,
		{"2"}
	},
	{
		&ATsetMultipleConnections,
		{""}
	},
	{
		&ATsetupServer,
		{"1"}
	},
	{
		&ATWaitForData,
		{""}
	},
	{
		&ATSendResp,
		{"",ROUTE_CLEARED}
	},
	{
		&ATSendData,
		{"", ROUTE_CLEARED}
	},
	{
		&ATClose,
		{""}
	},
	{
		&ATsetCurrentWifiMode,
		{"1"}
	},
	{
		&ATsetupServer,
		{"0"}
	},
	{
		&ATReset,
		{""}
	},
	{
		&ATsetCurrentWifiMode,
		{"1"}
	},
	{
		&ATconnectToAPI,
		{"s", "p"}
	}
};

struct commandStruct TCPReqCommands[] = {
	{
		&ATTCPStart,
		{""}
	},
	{
		&ATSendResp,
		{"",TCP_GET_ROUTE}
	},
	{
		&ATSendData,
		{"", TCP_GET_ROUTE}
	},
	{
		&ATWaitForData,
		{""}
	}
};

// ------ INTERRUPTS -------
ISR (PCINT1_vect) {
	// check to see which buttons are active
	// if active, increase the brightness value
	// we are comparing the input values on the button pin (which are pulled high
	// by the pullup resistor unless the button is active)

	AT_currentMode = AT_INTERRUPTED;

	if (R_BUTTON_DOWN) {
		colorBalance[0] += 15;
		OCR0B = colorBalance[0];
	}
	if (G_BUTTON_DOWN) {
		colorBalance[1] += 15;
		OCR2B = colorBalance[1];
	}
	if (B_BUTTON_DOWN) {
		colorBalance[2] += 15;
		OCR0A = colorBalance[2];
	}

	// Set Timer 1 count to 0
	TCNT1 = 0;
	// Clear flags on compare match A/B
	TIFR1 |= (1 << OCF1A) | (1 << OCF1B);
	// Enable interrupts on Timer 1 Comp A, to keep track of delay after user selects a color
	TIMSK1 |= (1 << OCIE1A);
	// Disable interrupts on Timer 1 Comp B, so that we are no longer initiating TCP polling
	TIMSK1 &= ~(1 << OCIE1B);
}

ISR (TIMER1_COMPA_vect) {
	// Called when the output color has changed and has remained constant for 
	// at least the amount of time in the COLOR_SAVE_DELAY_COUNT
	//printString("Writing... \r\n");

	// Write the current configuration to EEPROM
	//EEPROM_writePage(COLOR_SAVE_ADDRESS,sizeof(colorBalance), colorBalance);
	
	// Disable interrupts on Timer 1 Comp A/b
	TIMSK1 &= ~((1 << OCIE1A) | (1 << OCIE1B));

	// Reset the current AT mode so that we can begin making requests again
	// This time, we are going to make our POST request, to update database to new
	// configured color
	AT_currentMode = AT_READY;
	TCP_START_ROUTE = TCP_POST_ROUTE;
	// Pass the pointer to colorBalance as a parameter
	//TODO: When color is updated, the color changes briefly to a totally different color before
	
	TCP_RESPONSE_PARAM = colorBalance;
	TCP_DATA_PARAM = colorBalance;
	iCommands = 0;
	nextCommand = 1;
	iReceiveBuffer = 0;
	clearBuffer(receiveBuffer, ARRAY_LENGTH(receiveBuffer));


}

ISR (TIMER1_COMPB_vect) {
	// Disable interrupts on Timer 1 Comp B
	TIMSK1 &= ~(1 << OCIE1B);

	// Time for new TCP request, start from top of initCommands
	AT_currentMode = AT_READY;
	TCP_START_ROUTE = TCP_GET_ROUTE;
	iCommands = 0;
	nextCommand = 1;
	iReceiveBuffer = 0;
	clearBuffer(receiveBuffer, ARRAY_LENGTH(receiveBuffer));
}

ISR (USART_RX_vect) {
	char received = UDR0;

	if (AT_currentMode == AT_INTERRUPTED) {
		// Do nothing, at mode was interrupted so we do not need to store any incoming transmissions
		//printString("interrupted");
	}
	// New line character received, check buffer for response
	else if (received == '\n') {

		// Received an OK command, indicating that the command executed successfully, and we can move onto
		// the next command
		if (compareString(receiveBuffer, "OK\r", 3)){

			switch (AT_currentMode) {
				case AT_WAITING:
				case AT_SENDING:
					// do nothing if sending or waiting, waiting for '<' char
					break;
				case AT_CLOSING:
					// check to see if we have an ssid, if we do, continue.
					// If not, need to keep waiting
					if (ssid[0]) {
						nextCommand++;
					} else {
						// return to waiting command
						iCommands = API_WAITING_COMMAND_INDEX;
						nextCommand = API_WAITING_COMMAND_INDEX + 1;
					}
					break;
				case AT_POLL_START:
					TCP_RESPONSE_ROUTE = TCP_START_ROUTE;
					TCP_DATA_ROUTE = TCP_START_ROUTE;
				default:
					nextCommand++;
					break;
			}

		} else {
			switch (AT_currentMode) {
				case AT_CONFIGURING:
					// Received an ERROR command, indicating that the command executed with an ERROR. We will try to reexecute
					// the command, in case the error was due to unexpected communication errors
					if (compareString(receiveBuffer, "ERROR\r", 6)){
						errorCount++;
						iCommands--;
					}
					break;
				case AT_CONNECTING:
					// Received a FAIL command, indicating the the API connection failed after attempting to connect. 
					// We will need to reconfigure the network.
					if (compareString(receiveBuffer, "FAIL\r", 5)) {
						iCommands = 0;
						nextCommand = 1;
						clearBuffer(ssid, ARRAY_LENGTH(ssid));
						clearBuffer(pswd, ARRAY_LENGTH(pswd));
					}
					break;
				case AT_SENDING:
					// Received a SEND OK command, can close transmission (next command)
					if (compareString(receiveBuffer, "SEND OK\r", 8)){
						nextCommand++;
						// clear the SEND parameters
						API_DATA_ROUTE = ROUTE_CLEARED;
						API_RESPONSE_ROUTE = ROUTE_CLEARED;
					}
					break;
				case AT_WAITING:
					// Received the end of the first line of the network connection string,
					// which ends with a single return statement. If this is the FAVICON route, then
					// we ignore it. Otherwise, we continue onto the next command.
					if (compareString(receiveBuffer, "ERROR\r", 6)) {
						nextCommand++;
					}

					else if (compareString(receiveBuffer, "\r", 1)) {
						if ((API_RESPONSE_ROUTE != FAVICON_ROUTE) && (API_RESPONSE_ROUTE != ROUTE_CLEARED)) {
							nextCommand++;
						}

					}
					else if (compareString(receiveBuffer, "r ", 2)) {
						for (uint8_t i = 0; i < ARRAY_LENGTH(dataR); i++) {
							*(dataR + i) = *(receiveBuffer + 2 + i);
						}
					}
					else if (compareString(receiveBuffer, "g ", 2)) {
						for (uint8_t i = 0; i < ARRAY_LENGTH(dataG); i++) {
							*(dataG + i) = *(receiveBuffer + 2 + i);
						}
					}
					else if (compareString(receiveBuffer, "b ", 2)) {
						for (uint8_t i = 0; i < ARRAY_LENGTH(dataB); i++) {
							*(dataB + i) = *(receiveBuffer + 2 + i);
						}
						
					}
					else if (compareString(receiveBuffer, "CLOSED", 6)) {
						//printString(receiveBuffer);
						TCNT1 = 0;
						// Clear flags on compare match A/B
						TIFR1 |= (1 << OCF1A) | (1 << OCF1B);
						// Enable interrupts on Compare match B, to determine when we should make our next request.
						TIMSK1 |= (1 << OCIE1B);

						//Assign colors
						if (TCP_START_ROUTE == TCP_GET_ROUTE) {
							*colorBalance = strToUInt8(dataR, 3);
							*(colorBalance + 1) = strToUInt8(dataG, 3);
							*(colorBalance + 2) = strToUInt8(dataB, 3);
						}
						
					}
					break;
				default:
					break;
			}
		}

		iReceiveBuffer = 0;
		clearBuffer(receiveBuffer, ARRAY_LENGTH(receiveBuffer));
	} 
	// If the ESP8266 is acting as a server, received a space character, and either the SSID or Password is not configured,
	// we will check the buffer for network data
	else if ((AT_currentMode == AT_WAITING) && (received == ' ') && !((ssid[0]) || (pswd[0]))) {

		if (receiveBuffer[0] == '+') {
			//TODO: linkID[0] is following whatever receiveBuffer is becoming, so is constantly shifting.
			// Needs to copy the current value
			linkID[0] = receiveBuffer[LINK_ID_LOC];
			API_RESPONSE_LINK_ID = linkID;
			API_CLOSE_LINK_ID = linkID;
		}
		else if (receiveBuffer[0] == '/') {
			//Need to determine the route, and respond appropriately.
			// Rudimentary implementation, but we will only check the character
			// immediately proceeding the '/' character, to quickly determine route
			// Will set parameter for the current command, which is sending the response
			// and the parameter for the upcoming command, which is sending the actual data
			if (receiveBuffer[1] == 0) {
				// present home screen
				API_RESPONSE_ROUTE = HOME_ROUTE;
				API_DATA_ROUTE = HOME_ROUTE;
			}
			else if (receiveBuffer[1] == NETWORK_CONFIG_ROUTE) {
				get_SSID_PSWD_fromPartialQueryString(receiveBuffer, ssid, pswd, ARRAY_LENGTH(receiveBuffer));
				// present "thank you" screen
				API_RESPONSE_ROUTE = NETWORK_CONFIG_ROUTE;
				API_DATA_ROUTE = NETWORK_CONFIG_ROUTE;
			}
			else if (receiveBuffer[1] == FAVICON_ROUTE) {
				// Do not want to send a response, just forget about this
				// will set the parameters anyway so that when we do receive the full network connection
				// data set, we will know not to continue on to send the response/data.
				API_RESPONSE_ROUTE = FAVICON_ROUTE;
				API_DATA_ROUTE = FAVICON_ROUTE;
			}
			else {
				// Handle Error
				API_RESPONSE_ROUTE = ERROR_ROUTE;
				API_DATA_ROUTE = ERROR_ROUTE;
			}
		}

		iReceiveBuffer = 0;
		clearBuffer(receiveBuffer, ARRAY_LENGTH(receiveBuffer));
	}
	else if ((AT_currentMode == AT_SENDING) && (received == '>')) {
		nextCommand++;
	}
	else {
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
	//OCR1A = COLOR_SAVE_DELAY_COUNT;
	OCR1A = STATUS_LED_TOGGLE_DELAY_COUNT;

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
	cli();
	// Enable interrupts on pin change interrupt vector 1
	PCICR |= (1 << PCIE1);
	// Enable interrupts specifically on the BUTTON pins
	PCMSK1 |= (1 << PCINT11) | (1 << PCINT10) | (1 << PCINT9);
	// Clear any flags on PCINT1
	PCIFR |= (1 << PCIF1);
	sei();

}

int main(void) {
	// ------ INITS -------

	initTimers();
	initUSART();
	initSPI_Master();

	// Set OC1A for output, for yellow status LED
	DDRB |= (1 << STATUS_LED);

	// ------ BUTTON SETUP -------
	// Ensure that BUTTON_DDR is set for input
	BUTTON_DDR &= ~((1 << RBUTTON) | (1 << GBUTTON) | (1 << BBUTTON));
	// Enable pullup resistors on BUTTON_PIN
	BUTTON_PORT |= (1 << RBUTTON) | (1 << GBUTTON) | (1 << BBUTTON);

	// ----- EEPROM CHECK -----
	//EEPROM_readPage(COLOR_SAVE_ADDRESS,sizeof(colorBalance), colorBalance);

	// ----- AT COMMANDS -----

	// Enable Global interrupts
	sei();

	// ------ NETWORK SETUP ------

	// 1. Retrieve saved SSID from EEPROM, if one exists
	SSID_CONFIG = ssid;
	PSWD_CONFIG = pswd;

	/*
	//////// TESTING PURPOSES ONLY
		char testSSID[] = "TEST";
		EEPROM_writePage(SSID_SAVE_ADDRESS, sizeof(testSSID), testSSID);
	/////// END TESTING
	*/
	
	EEPROM_readPage(SSID_SAVE_ADDRESS, sizeof(ssid), ssid);
	// If an ssid was found, grab the password from EEPROM and attempt to connect
	if (ssid[0]) {
		// Grab password from EEPROM
		EEPROM_readPage(PSWD_SAVE_ADDRESS, sizeof(pswd), pswd);

		// Attempt to connect to WiFi immediately by moving the command index forward
		iCommands = API_CONNECT_COMMAND_INDEX;
		nextCommand = API_CONNECT_COMMAND_INDEX + 1;

	}
	// Wait for ESP8266 initialization to complete
	_delay_ms(START_DELAY_TIME_MS);

	// execute COMMANDS serially to setup WiFi connection
	while (iCommands < ARRAY_LENGTH(initCommands)) {
		if (iCommands == API_CONNECT_COMMAND_INDEX) {
			// Begin toggling the STATUS LED to indicate that the lamp is attempting to
			// connect to the network.
			TCCR1A |= (1 << COM1A0);
		}
		initCommands[iCommands].execute(initCommands[iCommands].parameters, sizeof(initCommands[iCommands].parameters));
		iCommands++;
		while (iCommands == nextCommand) {
			// Wait until nextCommand is ready to be executed
		}
	}
	// After initialization commands finish, reset iCommands and nextCommand for use with TCP commands
	// initial TCP mode will be the getColor request
	iCommands = 0;
	nextCommand = 1;
	AT_currentMode = AT_READY;
	TCP_START_ROUTE = TCP_GET_ROUTE;

	// When commands have been exhausted, we should save new SSID/password to EEPROM
	// TODO: Only do this when passwords have changed
	EEPROM_writePage(SSID_SAVE_ADDRESS, sizeof(ssid), ssid);
	EEPROM_writePage(PSWD_SAVE_ADDRESS, sizeof(pswd), pswd);

	// Disable Status LED Toggling
	TCCR1A &= ~(1 << COM1A0);

	// Update Timer 1 Compare Value A for color saving delay
	// Update Timer 1 Compare Value B for TCP polling delay
	OCR1A = COLOR_SAVE_DELAY_COUNT;
	OCR1B = TCP_POLL_DELAY_COUNT;


	// ----- INTERRUPT INIT ------
	initButtonInterrupts();

	// ------ LED SETUP -------

	// Set LED data direction to output on connected LEDs
	LED_DDR |= (1 << RLED) | (1 << GLED) | (1 << BLED);

	// ------ EVENT LOOP -------
	while(1) {
		// If there is a command queued and the sequence was not interrupted by an color change interrupt,
		// execute the next command
		if ((iCommands < nextCommand) && (AT_currentMode != AT_INTERRUPTED)) {
			// if there is a command queued to be performed, perform it.
			// The main loop will handle TCP requests.
			if (iCommands < ARRAY_LENGTH(TCPReqCommands)) {
				TCPReqCommands[iCommands].execute(TCPReqCommands[iCommands].parameters, sizeof(TCPReqCommands[iCommands].parameters));
				iCommands++;
			}			
		}
		OCR0B = colorBalance[0];
		OCR2B = colorBalance[1];	
		OCR0A = colorBalance[2];
	}

	// This line is never reached
	return 0;
}

void clearBuffer(volatile char *array, uint8_t len) {
	for (uint8_t i = 0; i < len; i++) {
		*(array + i) = 0;
	}
}

uint8_t compareString(volatile char *array, const char compStr[], uint8_t len) {
	//TODO: Having trouble getting the correct array length from this macro
	for (uint8_t i = 0; i < len; i++){
		if (*(array + i) != compStr[i]) {
			return 0;
		}
	}
	return 1;
}

void get_SSID_PSWD_fromPartialQueryString(volatile char *url, volatile char *assignSSID, volatile char *assignPSWD, uint8_t len) {
	uint8_t i;
	char *p;

	while (i < len) {
		if ((*(url + i) == '?') || (*(url + i) == '&')) {

			if (*(url + i + 1) == SSID_Q_STRING) {
				p = assignSSID;
			}
			else if (*(url + i + 1) == PSWD_Q_STRING) {
				p = assignPSWD;
			}
			else {
				i++;
				continue;
			}
			// skip to first char in ssid string
			i += 3;
			const uint8_t start = i;
			// loop through this query string, until the end of the string is reached
			// or another query is seen.
			while ((*(url + i) != '&') && (*(url + i) != 0)) {
				*(p + i - start) = *(url + i);
				i++;
			}
		}
		else {
			i++;
		}
	}
}

uint8_t strToUInt8(volatile char *letter, uint8_t len) {
	uint8_t result;

	for (uint8_t i = 0; i < len; i++) {
		uint8_t multiplier = 1;
		for (uint8_t x = i+1; x < len; x++) {
			multiplier *= 10;
		}
		//TODO: Figure out why the result is correct when printByte(multiplier) exists, but
		// is wrong when it is omitted... (off by 2)
		printByte(multiplier);
		result +=((*(letter + i) - CHAR_TO_UINT_OFFSET)) * multiplier;
	}
	
	return result;
}





