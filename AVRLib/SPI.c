// R Melpignano 26 DEC 2017
// ------------------------

#include <avr/io.h>
#include "SPI.h"

void initSPI_Master(void) {

	// Set SS, MOSI, and SCK to output. MISO remains input.
	SPI_DDR |= (1 << SPI_SS) | (1 << SPI_MOSI) | (1 << SPI_SCK);

	// Set SS on SPI high, so that EEPROM is not selected.
	// Set pullup on MISO.
	SPI_PORT |= (1 << SPI_SS) | (1 << SPI_MISO);

	// Enable SPI hardware, set mode to master, enable SPI interrupts, set
	// clock rate to F_CPU / 128 = 62.5 kHz (assuming F_CPU is 8 MHz)
	SPCR |= (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);
}

void SPI_tradeByte(uint8_t byte) {
	// Load the byte to transfer into the SPDR register.
	SPDR = byte;
	loop_until_bit_is_set(SPSR, SPIF);
}

uint8_t EEPROM_readByte(uint16_t address) {
	EEPROM_SELECT;
	SPI_tradeByte(EEPROM_READ);
	EEPROM_sendAddress(address);
	SPI_tradeByte(0);
	EEPROM_DESELECT;
	return (SPDR);
}

uint8_t EEPROM_readPage(uint16_t address, uint8_t numberOfBytes, volatile uint8_t *assignmentPointer) {
	uint8_t i;
	EEPROM_SELECT;
	SPI_tradeByte(EEPROM_READ);
	EEPROM_sendAddress(address);

	for (i = 0; i < numberOfBytes; i++) {
		SPI_tradeByte(0);

		if (SPDR == 0xFF) {
			break;
		}

		*(assignmentPointer + i) = SPDR;
	}
	EEPROM_DESELECT;
	return i;
}

void EEPROM_writeByte(uint8_t byte, uint16_t address) {
	// Enable Write actions
	EEPROM_writeEnable();
	// Send write command
	EEPROM_SELECT;
	SPI_tradeByte(EEPROM_WRITE);
	// Send address
	EEPROM_sendAddress(address);
	// Send data
	SPI_tradeByte(byte);
	EEPROM_DESELECT;
	// Wait until the write completes
	while (EEPROM_readStatus() & (1 << EEPROM_WIP)) {;

	}
}

void EEPROM_writePage( uint16_t address, uint8_t numberOfBytes, volatile uint8_t *dataPointer) {
	uint8_t i;

	// Enable Write actions
	EEPROM_writeEnable();
	// Send write command
	EEPROM_SELECT;
	SPI_tradeByte(EEPROM_WRITE);
	// Send address
	EEPROM_sendAddress(address);
	// Send data
	
	for (i = 0; i < numberOfBytes; i++) {
		SPI_tradeByte(*(dataPointer + i));
	}
	
	EEPROM_DESELECT;
	// Wait until the write completes
	while (EEPROM_readStatus() & (1 << EEPROM_WIP)) {;

	}
}

uint8_t EEPROM_readStatus(void) {
	EEPROM_SELECT;
	SPI_tradeByte(EEPROM_RDSR);
	SPI_tradeByte(0);
	EEPROM_DESELECT;
	return (SPDR);
}

void EEPROM_sendAddress(uint16_t address) {
	// First send the MSB of the address, then send LSB. Casting as 8 bit int
	// and shifting will give MSB, and simply casting will give LSB.
	SPI_tradeByte((uint8_t) (address >> 8));
  	SPI_tradeByte((uint8_t) address); 
}

void EEPROM_writeEnable(void) {
	// Select the EEPROM
	EEPROM_SELECT;
	SPI_tradeByte(EEPROM_WREN);
	// Slave must be deselected after receiving command to enable writes
  	EEPROM_DESELECT;
}




