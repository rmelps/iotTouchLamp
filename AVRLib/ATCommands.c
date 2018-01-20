#include <avr/io.h>
#include <avr/pgmspace.h>
#include "USART.h"
#include "ATCommands.h"

// General Separators
const char AT_separator[] PROGMEM = "\",\"";
const char AT_terminator[] PROGMEM = "\"\r\n";

// TCP request parameters
const char AT_hostAddress[] PROGMEM = "us-central1-radio-free-america.cloudfunctions.net";
const char AT_functionAddress[] PROGMEM = "/testFn";
const char AT_requestType[] PROGMEM = "POST";
const char AT_HTTPver[] PROGMEM = "HTTP/1.0\r\n";
const char AT_TCPHostText[] PROGMEM = "Host:";
const char AT_port[] PROGMEM = "80";

// AT commands

// Set current Wifi mode
const char AT_cwmode_cur[] PROGMEM = "AT+CWMODE_CUR=";

// Connect to desired AP, first argument being ssid, second being password
const char AT_cwjap_cur[] PROGMEM = "AT+CWJAP_CUR=\"";

// Send message byte count before sending actual request (first argument requests
// number of bytes to send, second argument is the message.)
const char AT_cipsend[] PROGMEM = "AT+CIPSEND=";

// Configure ESP8266 for multiple connections
const char AT_cipmux[] PROGMEM = "AT+CIPMUX=1\r\n";

// Setup ESP8266 server, '0' to disable, '1,80' to setup on port 80, to be
// accessed thru browser
const char AT_cipserver[] PROGMEM = "AT+CIPSERVER=";

//Reset the ESP8266 module
const char AT_rst[] PROGMEM = "AT+RST\r\n";

// Start TCP connection with server
const char AT_cipstart[] PROGMEM = "AT+CIPSTART=\"TCP\",\"";

// --Generic--
void transmitFromPGMSpace(const char *p, uint8_t len) {
	char character;
	uint8_t i;

	for (i = 0; i < len; i++){
		character = pgm_read_byte(p+i);
		if (character == 0x00) {
			break;
		}
		transmitByte(character);
		if (character == '\n') {
			break;
		}
	}
}

// --Commands to send--

void ATsetCurrentWifiMode(char *parameters[], uint8_t len) {
	transmitFromPGMSpace(AT_cwmode_cur, (sizeof(AT_cwmode_cur)));
	printString(parameters[0]);
	printString("\r\n");
	AT_currentMode = AT_CONFIGURING;
}

void ATconnectToAPI(char *parameters[], uint8_t len) {
	transmitFromPGMSpace(AT_cwjap_cur, (sizeof(AT_cwjap_cur)));
	printString(parameters[0]);
	transmitFromPGMSpace(AT_separator, (sizeof(AT_separator)));
	printString(parameters[1]);
	transmitFromPGMSpace(AT_terminator, (sizeof(AT_terminator)));
}

void ATsetMultipleConnections(char *parameters[], uint8_t len) {
	transmitFromPGMSpace(AT_cipmux, (sizeof(AT_cipmux)));
}

void ATsetupServer(char *parameters[], uint8_t len) {
	transmitFromPGMSpace(AT_cipserver, (sizeof(AT_cipserver)));
	printString(parameters[0]);
	printString(",");
	transmitFromPGMSpace(AT_port, (sizeof(AT_port)));
	printString("\r\n");
	AT_currentMode = AT_WAITING;
}

void ATReset(char *parameters[], uint8_t len) {
	transmitFromPGMSpace(AT_rst, (sizeof(AT_rst)));
}

void ATWaitForData(char *parameters[], uint8_t len) {
	// Do nothing, waiting will be done in loop
}

void ATTCPStart(char *parameters[], uint8_t len) {

}

void ATSendData(char *parameters[], uint8_t len) {

}








