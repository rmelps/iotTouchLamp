#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "USART.h"
#include "ATCommands.h"

const uint16_t sn = 1;

// General Separators
const char AT_separator[] PROGMEM = "\",\"";
const char AT_terminator[] PROGMEM = "\"\r\n";

// TCP request parameters
const char AT_getColor[] PROGMEM = "/getColor?s=";

const char AT_setColor[] PROGMEM = "/setColor?s=";
const char AT_setRed[] PROGMEM = "&r=";
const char AT_setGreen[] PROGMEM = "&g=";
const char AT_setBlue[] PROGMEM = "&b=";

const char AT_requestTypePost[] PROGMEM = "POST ";
const char AT_requestTypeGet[] PROGMEM = "GET ";
const char AT_HTTPver[] PROGMEM = " HTTP/1.0\r\n";
const char AT_TCPHostText[] PROGMEM = "Host: us-central1-iotlamp-rm.cloudfunctions.net\r\n";
const char AT_TCPContentLength[] PROGMEM = "Content-Length: 0\r\n";

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
const char AT_cipstart[] PROGMEM = "AT+CIPSTART=\"TCP\",\"https://us-central1-iotlamp-rm.cloudfunctions.net\",80\r\n";

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
	transmitFromPGMSpace(AT_cipstart, (sizeof(AT_cipstart)));
	AT_currentMode = AT_POLL_START;
}

void ATSendResp(char *parameters[], uint8_t len) {
	transmitFromPGMSpace(AT_cipsend, (sizeof(AT_cipsend)));
	if (parameters[1] == TCP_GET_ROUTE) {
		// The Numbers at the end acounts for the size of the serial number and the return/new line at the end
		uint8_t size = sizeof(AT_requestTypeGet) + sizeof(AT_getColor) + sizeof(AT_HTTPver)
				+ sizeof(AT_TCPHostText) + sizeof(AT_TCPContentLength) + 2;
		printByte(size);
	}
	else if (parameters[1] == TCP_POST_ROUTE) {
		// The Numbers at the end acounts for the size of the serial number, the return/new line at the end
		// and the 3 colors (3 chars each)
		uint8_t size = sizeof(AT_requestTypePost) + sizeof(AT_setColor) + sizeof(AT_HTTPver)
				+ sizeof(AT_TCPHostText) + sizeof(AT_TCPContentLength) + sizeof(AT_setRed) + sizeof(AT_setGreen)
				+ sizeof(AT_setBlue) + 9;
		printByte(size);
	}
	else {
		// In this case parameters[0] will be the connection number, parameters 1 will be the
		// length of the data to send
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
	}
	printString("\r\n");
	AT_currentMode = AT_SENDING;
}

void ATSendData(char *parameters[], uint8_t len) {

	if (parameters[1] == TCP_GET_ROUTE) {
		//TODO: Send HTTP request info for getColor
		transmitFromPGMSpace(AT_requestTypeGet, (sizeof(AT_requestTypeGet)));
		transmitFromPGMSpace(AT_getColor, (sizeof(AT_getColor)));
		printString("00");
		printByte(sn);
		transmitFromPGMSpace(AT_HTTPver, (sizeof(AT_HTTPver)));
		transmitFromPGMSpace(AT_TCPHostText, (sizeof(AT_TCPHostText)));
		transmitFromPGMSpace(AT_TCPContentLength, (sizeof(AT_TCPContentLength)));
		printString("\r\n");

	}
	else if (parameters[1] == TCP_POST_ROUTE) {
		//TODO: Send HTTP request info for setColor
		transmitFromPGMSpace(AT_requestTypePost, (sizeof(AT_requestTypePost)));
		transmitFromPGMSpace(AT_setColor, (sizeof(AT_setColor)));
		printString("00");
		printByte(sn);

		transmitFromPGMSpace(AT_setRed, (sizeof(AT_setRed)));
		printByte(*(parameters[0]));

		transmitFromPGMSpace(AT_setGreen, (sizeof(AT_setGreen)));
		printByte(*(parameters[0] + 1));

		transmitFromPGMSpace(AT_setBlue, (sizeof(AT_setBlue)));
		printByte(*(parameters[0] + 2));

		transmitFromPGMSpace(AT_HTTPver, (sizeof(AT_HTTPver)));
		transmitFromPGMSpace(AT_TCPHostText, (sizeof(AT_TCPHostText)));
		transmitFromPGMSpace(AT_TCPContentLength, (sizeof(AT_TCPContentLength)));
		printString("\r\n");
	}
	else {
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
	}	
	printString("\r\n");
}

void ATClose(char *parameters[], uint8_t len) {
	transmitFromPGMSpace(AT_cipclose, (sizeof(AT_cipclose)));
	printString(parameters[0]);
	printString("\r\n");
	AT_currentMode = AT_CLOSING;
}








