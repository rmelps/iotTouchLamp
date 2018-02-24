#include "I2C.h"
#include "USART.h"

void initI2C(void) {

	// SCL frequency = F_CPU/(16 + 2(TWBR)*(TWPS prescalar))
	// SCL frequency should be at least 16x lower than F_CPU
	// We will have I2C run at 176x slower (45 kHz)
	TWBR = 40;
	TWSR |= (1 << TWPS0);

	// In TWI Control Register, enable master ACK/NACK, enable TWI interrupts
	TWCR |= (1 << TWEA)|(1 << TWIE);
}

void i2cStartTransmission(void) {
	// Clear TWI Interrupt Flag, Enable TWI, and Send Start Condition
	printString("ST\0");
	TWCR |= (1 << TWEN) | (1 << TWINT) | (1 << TWSTA);
}

void i2cStopTransmission(void) {
	printString("SO\0");
	TWCR |= (1 << TWEN) | (1 << TWINT) | (1 << TWSTO);
}

void i2cSlaveTransmit(I2C_Trans *t) {
	// chip address is 7 bits, followed by R (1) or W (0) bit
	TWDR = (t->chipAddress << 1);
	TWDR += t->isReading;
	printString("SlTr\0");
	printByte(TWDR);
	//TWCR |= (1 << TWEN) | (1 << TWINT);
}

void i2cDataTransmit(I2C_Trans *t) {
	TWDR = t->data[t->iData];
	printString("DaTr\0");
	printByte(TWDR);
	//TWCR |= (1 << TWEN) | (1 << TWINT);
}

void i2cAddressTransmit(I2C_Trans *t) {
	TWDR = t->internalAddress;
	printString("AdTr\0");
	printByte(TWDR);
	//TWCR |= (1 << TWEN) | (1 << TWINT);
}

void i2cSendWrite(I2C_Trans *t) {
	TWDR = (t->chipAddress << 1);
	//DEBUG ONLY
	//TWDR = 0x36;
	printString("Write\0");
	printByte(TWDR);
	//TWCR |= (1 << TWEN) | (1 << TWINT);
}

uint8_t i2cRead(void) {
	printString("Read\0");
	printByte(TWDR);
  return (TWDR);
}