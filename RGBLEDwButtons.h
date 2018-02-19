// Main header file
// RMelpignano 1-17-2018
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "rgbledPinDefines.h"
#include "USART.h"
#include "SPI.h"
#include "ATCommands.h"
#include "I2C.h"
#include "AT42.h"


#define COLOR_SAVE_DELAY_COUNT			39063
#define TCP_POLL_DELAY_COUNT			23438
#define STATUS_LED_TOGGLE_DELAY_COUNT	10000
#define COLOR_SAVE_ADDRESS				EEPROM_PAGE_SIZE * 0
#define SSID_SAVE_ADDRESS				EEPROM_PAGE_SIZE * 1
#define PSWD_SAVE_ADDRESS				EEPROM_PAGE_SIZE * 2

#define CHAR_TO_UINT_OFFSET				48

#define ARRAY_LENGTH(A)				sizeof(A)/sizeof(A[0])

#define R_BUTTON_DOWN				(BUTTON_PIN & (1 << RBUTTON)) == 0
#define G_BUTTON_DOWN				(BUTTON_PIN & (1 << GBUTTON)) == 0
#define B_BUTTON_DOWN				(BUTTON_PIN & (1 << BBUTTON)) == 0

#define CHANGE_DETECTED				(CHANGE_PIN & (1 << CHANGE_P)) == 0

// Connect to API parameters/execution
// Found in the "commands" and "TCPReqCommands" structs...ensure that these are up to date at compile time
// ---------------------
#define API_CONNECT_COMMAND_INDEX	11
#define API_RESPONSE_COMMAND_INDEX	4
#define API_DATA_COMMAND_INDEX		5
#define API_WAITING_COMMAND_INDEX	3
#define API_CLOSE_COMMAND_INDEX		6

#define TCP_START_COMMAND_INDEX		0
#define TCP_RESPONSE_COMMAND_INDEX	1
#define TCP_DATA_COMMAND_INDEX		2
// ---------------------
#define API_CONNECT_COMMAND			initCommands[API_CONNECT_COMMAND_INDEX]
#define API_RESPONSE_LINK_ID		initCommands[API_RESPONSE_COMMAND_INDEX].parameters[0]
#define API_RESPONSE_ROUTE			initCommands[API_RESPONSE_COMMAND_INDEX].parameters[1]
#define API_DATA_ROUTE				initCommands[API_DATA_COMMAND_INDEX].parameters[1]
#define API_CLOSE_LINK_ID			initCommands[API_CLOSE_COMMAND_INDEX].parameters[0]
#define SSID_CONFIG					API_CONNECT_COMMAND.parameters[0]
#define PSWD_CONFIG					API_CONNECT_COMMAND.parameters[1]
#define API_CONNECT_EXECUTE			API_CONNECT_COMMAND.execute(API_CONNECT_COMMAND.parameters, sizeof(API_CONNECT_COMMAND.parameters));


#define TCP_START_ROUTE				TCPReqCommands[TCP_START_COMMAND_INDEX].parameters[0]
#define TCP_RESPONSE_PARAM			TCPReqCommands[TCP_RESPONSE_COMMAND_INDEX].parameters[0]
#define TCP_RESPONSE_ROUTE			TCPReqCommands[TCP_RESPONSE_COMMAND_INDEX].parameters[1]
#define TCP_DATA_PARAM				TCPReqCommands[TCP_DATA_COMMAND_INDEX].parameters[0]
#define TCP_DATA_ROUTE				TCPReqCommands[TCP_DATA_COMMAND_INDEX].parameters[1]


#define SM_BUFFER_SIZE				30
#define LAR_BUFFER_SIZE				60

// ESP8266 should take no longer than 0.2 ms to initialize and ready for command inputs
#define START_DELAY_TIME_MS			200

void clearBuffer(volatile char *array, uint8_t len);

uint8_t compareString(volatile char *array, const char compStr[], uint8_t len);

void get_SSID_PSWD_fromPartialQueryString(volatile char *url, volatile char *assignSSID, volatile char *assignPSWD, uint8_t len);

uint8_t strToUInt8(volatile char *letter, uint8_t len);

void resetCurrentOp(void);

void configureCurrentOp(uint8_t chipA, uint8_t internalA, uint8_t isReading, uint8_t *data);
