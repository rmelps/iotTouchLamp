#include <avr/io.h>
#include <avr/pgmspace.h>
#include "USART.h"
#include "ATCommands.h"

// TCP request parameters
const char AT_hostAddress[] PROGMEM = "us-central1-radio-free-america.cloudfunctions.net";
const char AT_functionAddress[] PROGMEM = "/testFn";
const char AT_requestType[] PROGMEM = "POST";
const char AT_HTTPver[] PROGMEM = "HTTP/1.0\r\n";
const char AT_TCPHostText[] PROGMEM = "Host:";


// AT commands

// Set current Wifi mode
const char AT_cwmode_def[] PROGMEM = "AT+CWMODE_CUR=";

// List available AP's
const char AT_cwlap[] PROGMEM = "AT+CWLAP\r\n";

// Connect to desired AP, first argument being ssid, second being password
const char AT_cwjap_cur[] PROGMEM = "AT+CWJAP_CUR=";

// Send message byte count before sending actual request (first argument requests
// number of bytes to send, second argument is the message.)
const char AT_cipsend[] PROGMEM = "AT+CIPSEND=";

// Configure ESP8266 for multiple connections
const char AT_cipmux[] PROGMEM = "AT+CIPMUX=1\r\n";

// Setup ESP8266 server, '0' to disable, '1,80' to setup on port 80, to be
// accessed thru browser
const char AT_cipserver[] PROGMEM = "AT+CIPSERVER=";

//Reset the ESP8266 module
const char AT_rst[] PROGMEM = "AT+RST";

// Disable auto connection to Wifi (to avoid signal scrambling during power-up)
const char AT_autoconn[] PROGMEM = "AT+AUTOCONN=0\r\n";

// --Generic--
void transmitFromPGMSpace(const char *p, uint8_t len) {
	char character;
	uint8_t i;

	for (i = 0; i < len; i++){
		character = pgm_read_byte(p+i);
		transmitByte(character);
		if (character == '\n') {
			break;
		}
	}
}

// --Commands to send--

void setCurrentWifiMode(char *parameters[], uint8_t len) {
	transmitFromPGMSpace(&AT_cwmode_def[0], (sizeof(AT_cwmode_def) - 1));
	printString(parameters[0]);
	printString("\r\n");
}

void listAvailableAPs(char *parameters[], uint8_t len) {
	transmitFromPGMSpace(&AT_cwlap[0], (sizeof(AT_cwlap) - 1));
}

void connectToAPI(char *parameters[], uint8_t len) {
	transmitFromPGMSpace(&AT_cwjap_cur[0], (sizeof(AT_cwjap_cur) - 1));
	printString("\"");
	printString(parameters[0]);
	printString("\",\"");
	printString(parameters[1]);
	printString("\"\r\n");
}