// R Melpignano, JAN 4 2018
// Contains string AT commands to send over UART to ESP8266
// All AT constants ending in \r do not require inputs before sending.
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "USART.h"

// AT functions

// --Generic--

// This commands takes strings saved in PGM space and transmits them sequentially over UART serial.
void transmitFromPGMSpace(const char *p, uint8_t len);

// --Commands to send--

// Sets the current wifi mode to either Station (1), SoftAP(2), or both (3)
void ATsetCurrentWifiMode(char *parameters[], uint8_t len);

// connect to AP with specific ssid and password.
void ATconnectToAPI(char *parameters[], uint8_t len);

// Set multiple connection mode
void ATsetMultipleConnections(void);

// Set Server on Port 80
void ATsetupServer(void);

// reset the ESP module
void ATReset(void);




