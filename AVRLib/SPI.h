// R Melpignano 26 DEC 2017
// ------------------------
// Intended for use with the 25LC256 EEPROM chip & ESP8266 Wifi module

#include <avr/io.h>
#include "rgbledPinDefines.h"

// ----- Slave Selects -----

#define EEPROM_SELECT 		SPI_PORT &= ~(1 << SPI_SS)
#define EEPROM_DESELECT 	SPI_PORT |= (1 << SPI_SS)

#define ESP8266_SELECT 		PORTC &= ~(1 << PC0)
#define ESP8266_DESELECT 	PORTC |= (1 << PC0)

// ----- Commands for 25LC256 External EEPROM chip -----

// READ COMMAND
#define EEPROM_READ 		0b00000011

// WRITE COMMAND
#define EEPROM_WRITE 		0b00000010

// WRITE ENABLE
#define EEPROM_WREN			0b00000110

// WRITE DISABLE
#define EEPROM_WRDI			0b00000100

// READ STATUS REGISTER
#define EEPROM_RDSR			0b00000101

// WRITE STATUS REGISTER
#define EEPROM_WRSR			0b00000001

// ----- EEPROM Status Register -----
// The following define the bit locations of status register values
// on the EEPROM chip. These values will be returned to the AVR in a byte
// when the EEPROM_RDSR command is sent

// WRITE IN PROGRESS BIT
// 1: Write currently in progress; 0: No Write in progress
#define EEPROM_WIP			0

// WRITE ENABLE LATCH BIT
// 1: Write permitted; 0: Write disabled
#define EEPROM_WEL			1

// BLOCK PROTECTION BITS
// Set by user through EEPROM_WRSR commands. Indicate which blocks are 
// currently write-protected
#define EEPROM_BP0			2
#define EEPROM_BP1			3


// ----- Functions -----

// --- GENERAL ---

// Initialize SPI for Master mode
void initSPI_Master(void);

// Send 1 byte from AVR to selected slave
void SPI_tradeByte(uint8_t byte);

// --- EEPROM SPECIFIC ---

// Read the currently stored value at given address
uint8_t EEPROM_readByte(uint16_t address);

// Read an array of bytes, starting at the given address and continuing for
// the requested number of bytes. Each byte that is read will be stored in an array at assignmentPointer
void EEPROM_readPage(uint16_t address, uint8_t numberOfBytes, volatile uint8_t *assignmentPointer);

// Write a given byte to a given address
void EEPROM_writeByte(uint8_t byte, uint16_t address);

// Write page of data, where each value is a 8 bit int and will be written
// in the order in which it is placed in the array
void EEPROM_writePage( uint16_t address, uint8_t numberOfBytes, volatile uint8_t *dataPointer);

// Read the value from the STATUS register
uint8_t EEPROM_readStatus(void);

// Send 16 bit address as two bytes
void EEPROM_sendAddress(uint16_t address);

// Enable Writing on the EEPROM. Used before any write commands.
void EEPROM_writeEnable(void);















