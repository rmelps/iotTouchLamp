#include "I2C.h"

void initI2C(void) {

	// SCL frequency = F_CPU/(16 + 2(TWBR)*(TWPS prescalar))
	// SCL frequency should be at least 16x lower than F_CPU
	// We will have I2C run at 176x slower (45 kHz)
	TWBR = 40;
	TWSR |= (1 << TWPS0);

	// In TWI Control Register, enable master ACK/NACK
	// I am disabling interrupts for now
	TWCR |= (1 << TWEA);
}

void waitUntilTWIReady(void) {
	while (!(TWCR & (1 << TWINT))) {
		// wait for TWI flag to be set
	}
}

void i2cSend(uint8_t data) {
	waitUntilTWIReady();
	TWDR = data;
	TWCR = (1 << TWEN) | (1 << TWINT);
}

uint8_t i2cRead() {
	waitUntilTWIReady();
	return (TWDR);
}