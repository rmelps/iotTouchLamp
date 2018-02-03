// R Melpignano, JAN 4 2018
// Contains string AT commands to send over UART to ESP8266
// All AT constants ending in \r do not require inputs before sending.
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "USART.h"

// AT functions

// -- Mode setting --
#define AT_CONFIGURING			0
#define AT_WAITING				1
#define AT_SENDING				2
#define AT_CONNECTING			3
#define AT_CLOSING				4
#define AT_POLL_START			5
#define AT_INTERRUPTED			6
#define AT_READY				7

// --- Network Responses ---
#define PSWD_Q_STRING			'p'
#define SSID_Q_STRING			's'
#define LINK_ID_LOC				5

// --- Request Route Identifiers ---
#define HOME_ROUTE				'h'
#define NETWORK_CONFIG_ROUTE	'i'
#define FAVICON_ROUTE			'f'
#define ERROR_ROUTE				'e'
#define TCP_GET_ROUTE			'g'
#define TCP_POST_ROUTE			'p'
#define ROUTE_CLEARED			"x"


uint8_t AT_currentMode;

// --Generic--

// This commands takes strings saved in PGM space and transmits them sequentially over UART serial.
void transmitFromPGMSpace(const char *p, uint8_t len);

// --Commands to send--

// Sets the current wifi mode to either Station (1), SoftAP(2), or both (3)
void ATsetCurrentWifiMode(char *parameters[], uint8_t len);

// connect to AP with specific ssid and password.
void ATconnectToAPI(char *parameters[], uint8_t len);

// Set multiple connection mode
void ATsetMultipleConnections(char *parameters[], uint8_t len);

// Set Up Server, first parameter being the mode
void ATsetupServer(char *parameters[], uint8_t len);

// reset the ESP module
void ATReset(char *parameters[], uint8_t len);

// Start TCP Connection, first parameter being website, second being the port
void ATTCPStart(char *parameters[], uint8_t len);

// Send Data to network, first parameter being the number of bytes to send
void ATSendResp(char *parameters[], uint8_t len);

// Wait for incoming data, indicated by '+' symbol
void ATWaitForData(char *parameters[], uint8_t len);

// Send the actual data to the network, this follows the ATSendResp command
void ATSendData(char *parameters[], uint8_t len);

// Close transmission after sending response
void ATClose(char *parameters[], uint8_t len);





