#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
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

// Webpages
const char AT_WP_home[] PROGMEM = "<h1>Welcome!</h1><h4>Please enter your network information to begin using your touch lamp!</h4><form action=\"/i\"><input type=\"text\" name=\"s\" placeholder=\"SSID\"><input type=\"text\" name=\"p\" placeholder=\"password\"><input type=\"submit\"></form>\r\n";
const char AT_WP_submit[] PROGMEM = "<h1>Thank you!</h1><p>Please follow the instructions provided with your touchlamp to finish setup<p>";
const char AT_WP_error[] PROGMEM = "<p>Error, please try again</p>";

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

// Close transmission
const char AT_cipclose[] PROGMEM = "AT+CIPCLOSE=";

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
	AT_currentMode = AT_CONNECTING;
}

void ATsetMultipleConnections(char *parameters[], uint8_t len) {
	transmitFromPGMSpace(AT_cipmux, (sizeof(AT_cipmux)));
}

void ATsetupServer(char *parameters[], uint8_t len) {
	transmitFromPGMSpace(AT_cipserver, (sizeof(AT_cipserver)));
	printString(parameters[0]);
	if (parameters[0] != "0") {
		printString(",");
		transmitFromPGMSpace(AT_port, (sizeof(AT_port)));
	}
	printString("\r\n");
}

void ATReset(char *parameters[], uint8_t len) {
	transmitFromPGMSpace(AT_rst, (sizeof(AT_rst)));
	_delay_ms(500);

}

void ATWaitForData(char *parameters[], uint8_t len) {
	AT_currentMode = AT_WAITING;
}

void ATTCPStart(char *parameters[], uint8_t len) {

}

void ATSendResp(char *parameters[], uint8_t len) {
	transmitFromPGMSpace(AT_cipsend, (sizeof(AT_cipsend)));
	printString(parameters[0]);
	printString(",");
	if (parameters[1] == HOME_ROUTE) {
		printByte(sizeof(AT_WP_home));
	}
	else if (parameters[1] == NETWORK_CONFIG_ROUTE) {
		printByte(sizeof(AT_WP_submit));
	}
	else {
		printByte(sizeof(AT_WP_error));
	}
	printString("\r\n");
	AT_currentMode = AT_SENDING;
}

void ATSendData(char *parameters[], uint8_t len) {
	if (parameters[1] == HOME_ROUTE) {
		transmitFromPGMSpace(AT_WP_home, (sizeof(AT_WP_home)));
	}
	else if (parameters[1] == NETWORK_CONFIG_ROUTE) {
		transmitFromPGMSpace(AT_WP_submit, (sizeof(AT_WP_submit)));
	}
	else if (parameters[1] == FAVICON_ROUTE) {
		// Do nothing
	}
	else {
		transmitFromPGMSpace(AT_WP_error, (sizeof(AT_WP_error)));
	}
	printString("\r\n");
}

void ATClose(char *parameters[], uint8_t len) {
	transmitFromPGMSpace(AT_cipclose, (sizeof(AT_cipclose)));
	printString(parameters[0]);
	printString("\r\n");
	AT_currentMode = AT_CLOSING;
}








