#include <avr/io.h>

// ---- Shared TWSR Register values (Master RX/TX modes)

#define I2C_START_TRANSMITTED		0x08
#define I2C_START_REPEATED			0x10
#define I2C_ARBITRATION_LOST		0x38

// ---- Value of TWSR Register in Master RX mode

#define I2C_SLAR_SENT_ACK			0x40
#define I2C_SLAR_SENT_NACK			0x48
#define I2C_R_DATA_ACK				0x50
#define I2C_R_DATA_NACK				0x58

// ---- Value of TWSR Register in Master TX mode

#define I2C_SLAW_SENT_ACK			0x18
#define I2C_SLAW_SENT_NACK			0x20
#define I2C_W_DATA_ACK				0x28
#define I2C_W_DATA_NACK				0x30

// I2C Status Register TWSR masked on the 3 LSB, since these are prescalar/reserved bits
#define TWSR_READ					TWSR & 0xF8
#define TWINT_CLEAR					TWCR |= (1 << TWEN) | (1 << TWINT)

#define I2C_START 					TWCR |= (1 << TWEN) | (1 << TWINT) | (1 << TWSTA);
#define I2C_STOP					TWCR |= (1 << TWEN) | (1 << TWINT) | (1 << TWSTO);
#define I2C_ENABLE_ACK				TWCR |= (1 << TWEA);
#define I2C_DISABLE_ACK				TWCR &= ~(1 << TWEA);

// Specify Transmission Parameters
// The maximum amount of data we will send at once is 3 bytes

// ---- I2C Initialization
void initI2C(void);

// ---- I2C Transmission

void waitUntilTWIReady(void);

void i2cSend(uint8_t data);

uint8_t i2cRead();



