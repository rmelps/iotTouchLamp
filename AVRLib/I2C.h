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

// Specify Transmission Parameters
// The maximum amount of data we will send at once is 3 bytes
typedef struct I2C_Trans{
	volatile uint8_t chipAddress;
	volatile uint8_t internalAddress;
	volatile uint8_t isReading;
	volatile uint8_t data[3];
	volatile uint8_t iData;
} I2C_Trans;

// ---- I2C Initialization
void initI2C(void);

// ---- I2C Transmission

void i2cStartTransmission(void);
void i2cStopTransmission(void);

void i2cSlaveTransmit(I2C_Trans *t);

void i2cDataTransmit(I2C_Trans *t);

void i2cAddressTransmit(I2C_Trans *t);

void i2cSendWrite(uint8_t *chipAddress);



