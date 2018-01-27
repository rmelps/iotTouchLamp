// Main header file
// RMelpignano 1-17-2018
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/pgmspace.h>
#include "rgbledPinDefines.h"
#include "USART.h"
#include "SPI.h"
#include "ATCommands.h"


#define COLOR_SAVE_DELAY_COUNT		39063
#define COLOR_SAVE_ADDRESS			EEPROM_PAGE_SIZE * 0
#define SSID_SAVE_ADDRESS			EEPROM_PAGE_SIZE * 1
#define PSWD_SAVE_ADDRESS			EEPROM_PAGE_SIZE * 2

#define ARRAY_LENGTH(A)				sizeof(A)/sizeof(A[0])

#define R_BUTTON_DOWN				(BUTTON_PIN & (1 << RBUTTON)) == 0
#define G_BUTTON_DOWN				(BUTTON_PIN & (1 << GBUTTON)) == 0
#define B_BUTTON_DOWN				(BUTTON_PIN & (1 << BBUTTON)) == 0

// Connect to API parameters/execution
#define API_CONNECT_COMMAND_INDEX	11
#define API_RESPONSE_COMMAND_INDEX	4
#define API_WAITING_COMMAND_INDEX	3
#define API_CLOSE_COMMAND_INDEX		6

#define API_CONNECT_COMMAND			commands[API_CONNECT_COMMAND_INDEX]
#define API_RESPONSE_LINK_ID		commands[API_RESPONSE_COMMAND_INDEX].parameters[0]
#define API_CLOSE_LINK_ID			commands[API_CLOSE_COMMAND_INDEX].parameters[0]
#define SSID_CONFIG					API_CONNECT_COMMAND.parameters[0]
#define PSWD_CONFIG					API_CONNECT_COMMAND.parameters[1]
#define API_CONNECT_EXECUTE			API_CONNECT_COMMAND.execute(API_CONNECT_COMMAND.parameters, sizeof(API_CONNECT_COMMAND.parameters));

#define SM_BUFFER_SIZE				30
#define LAR_BUFFER_SIZE				60

void clearBuffer(volatile char *array, uint8_t len);

uint8_t compareString(volatile char *array, const char compStr[], uint8_t len);

void get_SSID_PSWD_fromPartialQueryString(volatile char *url, volatile char *assignSSID, volatile char *assignPSWD, uint8_t len);