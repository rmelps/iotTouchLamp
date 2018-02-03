// R Melpignano 18 DEC 2017
// ------------------------
// Pin defines for RGBLED Project with pushbutton inputs

// General pins controlling balance of RGB of LEDs

#define RLED 						PD5
#define GLED 						PD3
#define BLED						PD6

#define LED_PORT					PORTD
#define LED_PIN                 	PIND
#define LED_DDR                 	DDRD

#define RBUTTON						PC2
#define GBUTTON						PC1
#define BBUTTON						PC3

#define BUTTON_PORT					PORTC
#define BUTTON_PIN					PINC
#define BUTTON_DDR					DDRC

#define STATUS_LED					PB1
#define ERROR_LED					PC0

// SPI port defines

#define SPI_SS                     	PB2
#define SPI_MOSI                    PB3
#define SPI_MISO                    PB4
#define SPI_SCK                     PB5

#define SPI_PORT               		PORTB
#define SPI_PIN                		PINB
#define SPI_DDR                		DDRB

