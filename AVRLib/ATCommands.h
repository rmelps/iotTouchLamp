// R Melpignano, JAN 4 2018
// Contains string AT commands to send over UART to ESP8266
// All AT constants ending in \r do not require inputs before sending.
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "USART.h"

// TCP request parameters
const char AT_hostAddress[] PROGMEM = "us-central1-radio-free-america.cloudfunctions.net";
const char AT_functionAddress[] PROGMEM = "/testFn";
const char AT_requestType[] PROGMEM = "POST";
const char AT_HTTPver[] PROGMEM = "HTTP/1.0\r";
const char AT_TCPHostText[] PROGMEM = "Host:";


// AT commands

// Set default Wifi mode
const char AT_cwmode_def[] PROGMEM = "AT+CWMODE_DEF=1\r";

// List available AP's
const char AT_cwlap[] PROGMEM = "AT+CWLAP\r";

// Connect to desired AP
const char AT_cwjap_cur[] PROGMEM = "AT+CWJAP_CUR=";

// Send message byte count before sending actual request
const char AT_cipsend[] PROGMEM = "AT+CIPSEND=";


// AT functions

// --Generic--
void transmitFromPGMSpace(const char *p, uint8_t len) {
	char character;
	uint8_t i;

	for (i = 0; i < len; i++){
		character = pgm_read_byte(p+i);
		transmitByte(character);
		if (character == "\r") {
			break;
		}
	}
}

// --Commands to send--

void setDefaultWifiMode(void) {
	transmitFromPGMSpace(&AT_cwmode_def[0], (sizeof(AT_cwmode_def) - 1));
}

void listAvailableAPs(void) {
	transmitFromPGMSpace(&AT_cwlap[0], (sizeof(AT_cwlap) - 1));
}

void connectToAPI(const char ssid[], const char pswd[]) {
	transmitFromPGMSpace(&AT_cwjap_cur[0], (sizeof(AT_cwjap_cur) - 1));
	printString("\"");
	printString(ssid);
	printString("\",\"");
	printString(pswd);
	printString("\"\r");
}


